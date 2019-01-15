#pragma once

#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <iostream>

#define timerStart \
	cudaEventCreate(&start); \
	cudaEventCreate(&stop); \
	cudaEventRecord( start, 0 ); \

#define timerEnd \
	cudaEventRecord( stop, 0 ); \
	cudaEventSynchronize( stop ); \
	cudaEventElapsedTime( &time, start, stop ); \

typedef struct 
{
	double m;      /*8 bytes*/
	float3 p;      /*12 bytes*/
	float dummy[3];/*12 bytes*/

} CUDA_STRUCT;
#define  MAX_DEVICE_ARRAY_SIZE 75000 

//////////////////////////////////////////////////////////////////////////  -------------------------------------------  CUDA UTILS

inline int cu_ConvertSMVer2Cores(int major, int minor)
{
	// Defines for GPU Architecture types (using the SM version to determine the # of cores per SM
	typedef struct {
		int SM; // 0xMm (hexidecimal notation), M = SM Major version, and m = SM minor version
		int Cores;
	} sSMtoCores;

	sSMtoCores nGpuArchCoresPerSM[] = 
	{ { 0x10,  8 }, // Tesla Generation (SM 1.0) G80 class
	  { 0x11,  8 }, // Tesla Generation (SM 1.1) G8x class
	  { 0x12,  8 }, // Tesla Generation (SM 1.2) G9x class
	  { 0x13,  8 }, // Tesla Generation (SM 1.3) GT200 class
	  { 0x20, 32 }, // Fermi Generation (SM 2.0) GF100 class
	  { 0x21, 48 }, // Fermi Generation (SM 2.1) GF10x class
	  {   -1, -1 }
	};

	int index = 0;
	while (nGpuArchCoresPerSM[index].SM != -1) {
		if (nGpuArchCoresPerSM[index].SM == ((major << 4) + minor) ) {
			return nGpuArchCoresPerSM[index].Cores;
		}	
		index++;
	}
	printf("MapSMtoCores undefined SM %d.%d is undefined (please update to the latest SDK)!\n", major, minor);
	return -1;
}
int cu_gpuGetMaxGflopsDeviceId()
{
	int current_device     = 0, sm_per_multiproc  = 0;
	int max_compute_perf   = 0, max_perf_device   = 0;
	int device_count       = 0, best_SM_arch      = 0;
	cudaDeviceProp deviceProp;
	cudaGetDeviceCount( &device_count );

	// Find the best major SM Architecture GPU device
	while (current_device < device_count)
	{
		cudaGetDeviceProperties( &deviceProp, current_device );
		if (deviceProp.major > 0 && deviceProp.major < 9999)
		{
			best_SM_arch = MAX(best_SM_arch, deviceProp.major);
		}
		current_device++;
	}

	// Find the best CUDA capable GPU device
	current_device = 0;
	while( current_device < device_count )
	{
		cudaGetDeviceProperties( &deviceProp, current_device );
		if (deviceProp.major == 9999 && deviceProp.minor == 9999)
		{
			sm_per_multiproc = 1;
		}
		else
		{
			sm_per_multiproc = cu_ConvertSMVer2Cores(deviceProp.major, deviceProp.minor);
		}

		int compute_perf  = deviceProp.multiProcessorCount * sm_per_multiproc * deviceProp.clockRate;

		if( compute_perf  > max_compute_perf )
		{
			// If we find GPU with SM major > 2, search only these
			if ( best_SM_arch > 2 )
			{
				// If our device==dest_SM_arch, choose this, or else pass
				if (deviceProp.major == best_SM_arch)
				{
					max_compute_perf  = compute_perf;
					max_perf_device   = current_device;
				}
			}
			else
			{
				max_compute_perf  = compute_perf;
				max_perf_device   = current_device;
			}
		}
		++current_device;
	}
	return max_perf_device;
}
static void checkCUDAError(const char *msg)
{
	cudaError_t err = cudaGetLastError();
	if( cudaSuccess != err) {
		fprintf(stderr, "Cuda error: %s: %s.\n", msg, cudaGetErrorString( err) ); 
		//exit(EXIT_FAILURE); 
	}
} 


//////////////////////////////////////////////////////////////////////////  -------------------------------------------  DEVICE VARIABLES;

CUDA_STRUCT *d_dataArray ;
CUDA_STRUCT *d_dataArray_buf ;
int d_nElems = 0; ;

//////////////////////////////////////////////////////////////////////////  -------------------------------------------  DEVICE FUNCTIONS

__device__ float Clamp(float value, float min, float max) 
{
	return value < min ? min : value > max ? max : value;
}

__device__ float Map(float value, float inputMin, float inputMax, float outputMin, float outputMax) 
{
	return ((value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin);
}

__device__ float dot(float3 a, float3 b) 
{
	return ( (a.x * b.x) + (a.y * b.y) + (a.z * b.z) );
}
__device__ float3 normalise(float3 a) 
{
	float inv_mag = rsqrt(dot(a,a));
	return make_float3( (a.x * inv_mag) , (a.y * inv_mag) , (a.z * inv_mag) );
}





//////////////////////////////////////////////////////////////////////////  ------------------------------------------- KERNELS

__global__ void kernel_updateData(CUDA_STRUCT *data,int np )
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;

	if( idx < np )
	{
		//d_dataArray[idx].p.x+= 0.1 ;
		//d_dataArray[idx].p.y+= 0.1 ;
		data[idx].p.z -= 0.1 ;
	}
}


__global__ void kernel_moveToNearest( CUDA_STRUCT *verts , CUDA_STRUCT *verts_buf , int nv , float disp )
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;


	if( idx < nv )
	{
		float3 p1,p2 , nearestP ;
		int nearestId ;
		float minD2 ;
		minD2 = pow(10.0f,10.0f) ;

		p1 = verts[idx].p ;

		//if( !verts[idx].fixed )
		{
			for( int j =0 ; j < nv ; j++ )
			{
				if( idx == j )continue ;

				p2 = verts[j].p ;


				float3 v = make_float3( p2.x - p1.x ,p2.y - p1.y ,p2.z - p1.z ) ;
				float d2 = ( v.x*v.x + v.y*v.y + v.z*v.z) ;/*+ 1e-16*/; // calc distance squared to emitter - make sure it's not 0

				//if( d2 > disp * 10.0 )continue;

				if( d2 < minD2 && (p2.z < p1.z) && int(verts_buf[j].dummy[0]) != (idx) )
				{
					minD2 = d2 ; 
					nearestP = p2 ;
					nearestId = j ;
				}

			}

			float3 v = make_float3( nearestP.x - p1.x ,nearestP.y - p1.y ,nearestP.z - p1.z ) ;
			float rsq = rsqrt(dot(v,v));   
			v.x *= rsq ;
			v.y *= rsq;
			v.z *= rsq ;// normalize v
			verts_buf[idx].p =  make_float3( p1.x + disp * v.x ,p1.y + disp * v.y ,p1.z + disp * v.z ) ;
			verts_buf[idx].dummy[0] = float( nearestId ); ;
		}

		//rand[idx] = verts[idx].p.z ;
		//printf(" pts : %1.2f ,%1.2f ,%1.2f  \n" , verts_buf[idx].p.x ,verts_buf[idx].p.y ,verts_buf[idx].p.z ) ;
		//rand[idx] = verts[idx].v_acc ;

	}
}
__global__ void kernel_fillVbo(CUDA_STRUCT *data,int np,float4 *devicePtr_vbo )
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;

	if( idx < np )
	{
		devicePtr_vbo[idx] = make_float4(data[idx].p.x,data[idx].p.y,data[idx].p.z,1.0); ;
	}

}






//////////////////////////////////////////////////////////////////////////  -------------------------------------------  EXTERNS

//DATA-TRANSFER
extern "C" void initialiseDeviceMemory()
{
	if (d_dataArray != NULL)
		cudaFree(d_dataArray);

	cudaMalloc( /*(void **)*/ &d_dataArray , MAX_DEVICE_ARRAY_SIZE * sizeof(CUDA_STRUCT) );
	checkCUDAError(" cudaMalloc - device data storage ");
	

	if (d_dataArray_buf != NULL)
		cudaFree(d_dataArray_buf);

	cudaMalloc( /*(void **)*/ &d_dataArray_buf , MAX_DEVICE_ARRAY_SIZE * sizeof(CUDA_STRUCT) );
	checkCUDAError(" cudaMalloc - device data storage ");

	printf(" memory allocated on device : %1.2f MBytes \n" , 2.0 * (float(MAX_DEVICE_ARRAY_SIZE * sizeof(CUDA_STRUCT))/ pow(10.0f,6.0f)) ) ;
}
extern "C" void copyData_HostToDevice( CUDA_STRUCT *h_dataArray , int h_nElems )
{
	d_nElems = h_nElems ;
	cudaMemcpy(d_dataArray,h_dataArray,d_nElems*sizeof(CUDA_STRUCT),cudaMemcpyHostToDevice);
	checkCUDAError("cudaMemcpy particles H-D");
	printf(" pts : %i \n" , d_nElems ) ;
}

extern "C" CUDA_STRUCT * copyData_DeviceToHost( CUDA_STRUCT *h_dataArray , int &n )
{
	h_dataArray = (CUDA_STRUCT *) malloc( d_nElems * sizeof(CUDA_STRUCT));
	cudaMemcpy(h_dataArray,d_dataArray,d_nElems*sizeof(CUDA_STRUCT),cudaMemcpyDeviceToHost);
	checkCUDAError("cudaMemcpy particles H-D");
	n = d_nElems ;
	return h_dataArray ;
}
// UPDATE DATA

extern "C" void updateDeviceData()
{
	

	cudaEvent_t start, stop; 
	float time , cumTime ; time = cumTime = 0;; 

	timerStart ;

	kernel_updateData<<< int(sqrt(float(d_nElems))) , int(sqrt(float(d_nElems)))+2 >>>(d_dataArray,d_nElems);
	checkCUDAError("kernel_updateData");
	
	timerEnd;
	cumTime += time ;
	//printf(" GPU : kernel_updateData : time in ms : %1.4f , cumulative Time : %1.4f \n" , time , cumTime );

}

extern "C" float runStreams()
{
	cudaEvent_t start, stop; 
	float time , cumTime ; time = cumTime = 0;; 
	
	timerStart ;

		kernel_moveToNearest<<< int( sqrt(float(d_nElems))) , int( sqrt(float(d_nElems)))+2 >>>( d_dataArray , d_dataArray_buf ,d_nElems , 0.01 ); 
		checkCUDAError(" kernel_moveToNearest ");

		cudaMemcpy(d_dataArray,d_dataArray_buf,d_nElems*sizeof(CUDA_STRUCT),cudaMemcpyDeviceToDevice);
		checkCUDAError(" cudaMemcpy verts H-D ");

	timerEnd;
	cumTime += time ;
	//printf(" GPU : kernel_updateData : time in ms : %1.4f , cumulative Time : %1.4f \n" , time , cumTime );
	return cumTime;
}
//DISPLAY
extern "C" void fillVbo( float4 *dev_pos ) 
{
	kernel_fillVbo<<< int(sqrt(float(d_nElems))) , int(sqrt(float(d_nElems)))+2 >>>(d_dataArray,d_nElems,dev_pos);
	checkCUDAError("kernel_fillVbo");
}

//EXIT

extern "C" void CUDA_cleanup()
{
	cudaFree(d_dataArray);
	checkCUDAError("cudaFree : device data ");
}
