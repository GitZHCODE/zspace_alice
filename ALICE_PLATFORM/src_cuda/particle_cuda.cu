//#define _VERBOSE ;

#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif

#define transform \
dev_pos[idx] =  make_float4(p[idx].p.x,p[idx].p.y,p[idx].p.z,1.0);\
dev_pos[idx].w =  d;\
if(rotate)dev_pos[idx] = ArbitraryRotate2(p[idx].p,angle , t , make_float3(t.x,t.y,t.z + 1)) ;\
dev_pos[idx].w =  d;\
if(translate)\
{\
dev_pos[idx].x -= t.x -128.0 * i ;\
dev_pos[idx].y -= t.y ;\
dev_pos[idx].z = 0.0f ;\
dev_pos[idx].w =  d;\
}\


//dev_pos[idx].z =  10;\
//dev_pos[idx].w =  p[idx].p.z;\

//dev_pos[idx].z = 10.0f ;\
//#include "host_config.h"
//#include "builtin_types.h"
#include "channel_descriptor.h"
#include "cuda_runtime_api.h"
#include "driver_functions.h"
//#include "host_defines.h"
//#include "vector_functions.h"
//#include "vector_types.h"
#include <cuda_runtime.h>




#include <stdio.h>
#include <iostream>

#ifndef _PARTICLE_CUDA
#define _PARTICLE_CUDA

typedef struct {
	double m;   /* 8 Mass                          */
	float3 p;      /*  12 Position                      */
	float3 v;      /* 12 Velocity                      */
	float3 f;      /* 12 Force                         */
	int fixed;  /* 4 Fixed point or free to move   */
	//
	double dummy1[2]; //16
	//double dummy2; //8 

} PARTICLE;

typedef struct {
	float3 dpdt; // 12
	float3 dvdt; //12 

	double dummy1; // 8
} PARTICLEDERIVATIVES;

typedef struct {
	double gravitational; //8
	double viscousdrag; //8 
} PARTICLEPHYS;

typedef struct {
	int from; //4 
	int to; //4 
	double springconstant; //8
	float3 springForce ; // 12
	double dampingconstant; //8 
	double restlength; //8

	float dummy[4] ; //20 

} PARTICLESPRING;

typedef struct
{
	float x,y,z ;
}point;

typedef struct
{
	point min ;
	point max ;
}bbox;




#endif

// ---------------

int nparticles = 0;
PARTICLE *particles;
PARTICLE *particles_buf;
int nsprings = 0;
PARTICLESPRING *springs;
PARTICLEPHYS physical;
PARTICLEDERIVATIVES *deriv;

int *ParticleCentricSpringIds ; 
int4 *ParticleCentricSpringIds_baseAddress ; 
int baseAddress = 0 ;

int *fixed0 ;
int *fixed1 ;
int *fixed2 ;
int *fixed3 ;
int n_f0,n_f1,n_f2,n_f3 ;

float3 lamp0,lamp1,lamp2,lamp3 ;

int numBlocks = 0 ;

//bool cudaInited = false ;
// ------------ 

float3 *d_vbo_pos;

bool dMat_init = false ;
bool dPos_init = false ;
bool d_vbo_init = false ;


int d_iterations = 0 ;;

#define timerStart \
cudaEventCreate(&start); \
cudaEventCreate(&stop); \
cudaEventRecord( start, 0 ); \

#define timerEnd \
cudaEventRecord( stop, 0 ); \
cudaEventSynchronize( stop ); \
cudaEventElapsedTime( &time, start, stop ); \

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





//////////////////////////////////////////////////////////////////////////


__device__ float Clamp(float value, float min, float max) 
{
	return value < min ? min : value > max ? max : value;
}

__device__ float Map(float value, float inputMin, float inputMax, float outputMin, float outputMax) 
{
	return ((value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin);
}

__device__ float func(float x, float y)
{
	return ( cos(x) -sin(y)  ) ; //y - (1/(.5-pow(x,2))) ;//sin(x) - sin(pow(x,2));//tan(x)-tan(pow(x,2));//cos(x)-sin(y) ;// sin(x-y);//4*x - pow(x,2);
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
__device__ float4 ArbitraryRotate2(float3 p,double theta,point p1,float3 p2)
{
	float4 q = make_float4(0.0,0.0,0.0,1.0);
	double costheta,sintheta;
	float3 r;

	r.x = p2.x - p1.x;
	r.y = p2.y - p1.y;
	r.z = p2.z - p1.z;
	p.x -= p1.x;
	p.y -= p1.y;
	p.z -= p1.z;
	r = normalise(r);

	costheta = cos(theta);
	sintheta = sin(theta);

	q.x += (costheta + (1 - costheta) * r.x * r.x) * p.x;
	q.x += ((1 - costheta) * r.x * r.y - r.z * sintheta) * p.y;
	q.x += ((1 - costheta) * r.x * r.z + r.y * sintheta) * p.z;

	q.y += ((1 - costheta) * r.x * r.y + r.z * sintheta) * p.x;
	q.y += (costheta + (1 - costheta) * r.y * r.y) * p.y;
	q.y += ((1 - costheta) * r.y * r.z - r.x * sintheta) * p.z;

	q.z += ((1 - costheta) * r.x * r.z - r.y * sintheta) * p.x;
	q.z += ((1 - costheta) * r.y * r.z + r.x * sintheta) * p.y;
	q.z += (costheta + (1 - costheta) * r.z * r.z) * p.z;

	q.x += p1.x;
	q.y += p1.y;
	q.z += p1.z;
	return(q);
}



__device__ bool isInside( float3 pt , bbox bx )
{
	return ( pt.x > bx.min.x && pt.x < bx.max.x &&
			pt.y > bx.min.y && pt.y < bx.max.y  ) ;
}

__device__ float sqDist( float3 pt , bbox bx )
{
	float2 mid = make_float2((bx.max.x+bx.min.x)*0.5 , (bx.max.y+bx.min.y)*0.5 );
	float2 t = make_float2(pt.x-mid.x ,pt.y-mid.y );

	float d1 = fabs(t.x*t.x + t.y*t.y) ;
	
	return d1;
}

__global__ void kernel_moveToNearest( PARTICLE *verts , PARTICLE *verts_buf , int nv , float disp )
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;


	if( idx < nv )
	{
		float3 p1,p2 , nearestP ;
		int nearestId ;
		float minD2 ;
		minD2 = pow(10.0f,10.0f) ;

		p1 = verts[idx].p ;

		if( !verts[idx].fixed )
		{
			for( int j =0 ; j < nv ; j++ )
			{
				if( idx == j )continue ;

				p2 = verts[j].p ;


				float3 v = make_float3( p2.x - p1.x ,p2.y - p1.y ,p2.z - p1.z ) ;
				float d2 = ( v.x*v.x + v.y*v.y + v.z*v.z) ;/*+ 1e-16*/; // calc distance squared to emitter - make sure it's not 0

				//if( d2 > disp * 10.0 )continue;

				if( d2 < minD2 && (p2.z < p1.z) && int(verts_buf[j].dummy1[0]) != (idx) )
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
			verts_buf[idx].dummy1[0] = float( nearestId ); ;
		}

		//rand[idx] = verts[idx].p.z ;
		//printf(" pts : %1.2f ,%1.2f ,%1.2f  \n" , verts_buf[idx].p.x ,verts_buf[idx].p.y ,verts_buf[idx].p.z ) ;
		//rand[idx] = verts[idx].v_acc ;

	}
}

__global__ void kernel_clearNeighborTags( PARTICLE *verts , int nv )
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;


	if( idx < nv )
	{
		verts[idx].dummy1[0] = 0 ;
	}
}

__global__ void kernel_calculateForces_springs(PARTICLE *p , int np,
									PARTICLEPHYS phys, PARTICLESPRING *s,int ns )
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;


	if( idx < ns ) 
	{
		/*s[idx].springForce.x = 0;
		s[idx].springForce.y = 0;
		s[idx].springForce.z = 0;*/
		int p1,p2;
		float3 down = make_float3(0.0,0.0,-1.0);
		float3 zero =  make_float3(0.0,0.0,0.0);
		float3 f;
		double len,dx,dy,dz;
		PARTICLESPRING sp = s[idx];
		
		float SC = sp.springconstant ;
		float RL = sp.restlength ;
		float DC = sp.dampingconstant ;

		p1 = sp.from;
		p2 = sp.to;

		

		PARTICLE pr1 =  p[p1] ;
		PARTICLE  pr2 =  p[p2] ;
		dx =  pr1.p.x -  pr2.p.x;
		dy =  pr1.p.y -  pr2.p.y;
		dz =  pr1.p.z -  pr2.p.z;
		len = /*rsqrt(dx*dx + dy*dy + dz*dz) * (dx*dx + dy*dy + dz*dz)  ;*/ sqrt(dx*dx + dy*dy + dz*dz);
		f.x  = SC  * (len - RL);
		f.x += DC * ( pr1.v.x -  pr2.v.x) * dx / len;
		f.x *= - dx / len;
		f.y  = SC  * (len - RL);
		f.y += DC * ( pr1.v.y -  pr2.v.y) * dy / len;
		f.y *= - dy / len;
		f.z  = SC  * (len - RL);
		f.z += DC * ( pr1.v.z -  pr2.v.z) * dz / len;
		f.z *= - dz / len;
		
		s[idx].springForce.x = f.x ;
		s[idx].springForce.y = f.y ;
		s[idx].springForce.z = f.z ;
		
		//if (!pr1.fixed) 
		//{
		//	p[p1].f.x += f.x;
		//	p[p1].f.y += f.y;
		//	p[p1].f.z += f.z;
		//	//p[p1] = pr1;
		//}
		//
		//if (!pr2.fixed)
		//{
		//	p[p2].f.x -= f.x;
		//	p[p2].f.y -= f.y;
		//	p[p2].f.z -= f.z;

		//	//p[p2] = pr2;
		//}


	}
}



__global__ void kernel_gatherPerParticleForceAndUpdate(PARTICLE *p,int np ,PARTICLESPRING *s ,PARTICLEPHYS phys ,int *PCSpringIds , int4 *PCSpringIds_baseAddress , double dt )
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;
	
	/*for( int idx =0 ; idx < np ; idx++ )*/
	if( idx < np )
	{
		if( !p[idx].fixed )
		{
			int4 baseID = PCSpringIds_baseAddress[idx];
			int baseAddress = baseID.x ;
			int fromCnt = baseID.y ;
			int toCnt = baseID.z ;
			/*printf(" per particle spring ids for vert ID %i \n : fromCnt %i , toCnt %i \n" , idx ,fromCnt , toCnt  ) ;*/
			float3 v = p[idx].v ;
			float m = p[idx].m ;
			/*float3*/ /*f = p[idx].f ;*/
			
			float3 down = make_float3(0.0,0.0,-1.0);
			float3 f = make_float3(0,0,0) ;
			float3 SF = make_float3(0,0,0) ;
			//p[idx].f = cud::make_float3(0,0,0) ;
			int sId;
			//printf("   spring ids (FROM)  \n : %i ,\n" , PCSpringIds[baseAddress]  ) ;
				for( int i = baseAddress ; i < baseAddress + fromCnt ; i++ )
				{
					//printf(" per particle spring ids for vert ID %i \n : %i \n" , idx ,PCSpringIds[i]  ) ;
					sId = PCSpringIds[i] ;
					SF = s[sId].springForce ;
					//if( !p[idx].fixed )
					//{
						f.x -= SF.x ;
						f.y -= SF.y ;
						f.z -= SF.z ;
					//}
					//	printf(" pts : %1.2f ,%1.2f ,%1.2f , %1.2f \n" , s[sId].springForce.x  ,s[sId].springForce.y ,s[sId].springForce.z  ) ;
				}
		
				for( int i = baseAddress + fromCnt ; i < baseAddress + fromCnt + toCnt ; i++ )
				{
					/*printf(" per particle spring ids for vert ID %i \n : %i \n" , idx ,PCSpringIds[i]  ) ;*/
					sId = PCSpringIds[i] ;
					SF = s[sId].springForce ;
					//{
						f.x += SF.x ;
						f.y += SF.y ;
						f.z += SF.z ;
					//}
					//printf(" pts : %1.2f ,%1.2f ,%1.2f , %1.2f \n" , s[sId].springForce.x  ,s[sId].springForce.y ,s[sId].springForce.z  ) ;
				}

			/*p[idx].f.x += f.x ;
			p[idx].f.y += f.y ;
			p[idx].f.z += f.z ;*/

				/* Gravitation */			f.x += phys.gravitational * m * down.x;			f.y += phys.gravitational * m * down.y;			f.z += phys.gravitational *m * down.z;				/* Viscous drag */			f.x -= phys.viscousdrag * /*p[idx].*/v.x;			f.y -= phys.viscousdrag * /*p[idx].*/v.y;			f.z -= phys.viscousdrag */* p[idx].*/v.z;

			float3 dpdt = make_float3(0,0,0);
			float3 dvdt = make_float3(0,0,0);

			float oneOverMass = 1.0 / float(m);
			dpdt.x = v.x;
			dpdt.y = v.y;
			dpdt.z = v.z;
			dvdt.x = f.x * oneOverMass;
			dvdt.y = f.y * oneOverMass;
			dvdt.z = f.z * oneOverMass;

			//deriv[idx].dpdt = make_float3(dpdt.x,dpdt.y,dpdt.z );
			//deriv[idx].dvdt = make_float3(dvdt.x,dvdt.y,dvdt.z );

			//float3 pos = make_float3(0,0,0);
			//float3 vel = make_float3(0,0,0);
			/*float3*/ //dpdt = deriv[idx].dpdt ;
			/*float3*/// dvdt = deriv[idx].dvdt ;
	/*		pos.x += dpdt.x * dt;
			pos.y += dpdt.y * dt;
			pos.z += dpdt.z * dt;
			vel.x += dvdt.x * dt;
			vel.y += dvdt.y * dt;
			vel.z += dvdt.z * dt;*/

			//p[idx].f = f ;

			p[idx].p.x += dpdt.x * dt;
			p[idx].p.y += dpdt.y * dt;
			p[idx].p.z += dpdt.z * dt;

			p[idx].v.x += dvdt.x * dt;
			p[idx].v.y += dvdt.y * dt;
			p[idx].v.z += dvdt.z * dt;
		}


	}

}


__global__ void kernel_updateFixedPts( PARTICLE *p , int *fixed , int n_fixed , float ht   )
{
	int i = 0;
	for( i =0 ; i < n_fixed ; i++ )
	{
		int id = fixed[i] ;
		/*if( p[id].fixed )*/p[id].p.z = ht ;
	}
}

__global__ void kernel_calculateForces_springs_serial(PARTICLE *p,int np,
											   PARTICLEPHYS phys, PARTICLESPRING *s,int ns)
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;

	int i,p1,p2;
	float3 down = make_float3(0.0,0.0,-1.0);
	float3 zero =  make_float3(0.0,0.0,0.0);
	float3 f;
	double len,dx,dy,dz;

	for( /*int*/ idx =0 ; idx < ns ; idx++ ) 
	{
		p1 = s[idx].from;
		p2 = s[idx].to;
		dx = p[p1].p.x - p[p2].p.x;
		dy = p[p1].p.y - p[p2].p.y;
		dz = p[p1].p.z - p[p2].p.z;
		len = sqrt(dx*dx + dy*dy + dz*dz);
		f.x  = s[idx].springconstant  * (len - s[idx].restlength);
		f.x += s[idx].dampingconstant * (p[p1].v.x - p[p2].v.x) * dx / len;
		f.x *= - dx / len;
		f.y  = s[idx].springconstant  * (len - s[idx].restlength);
		f.y += s[idx].dampingconstant * (p[p1].v.y - p[p2].v.y) * dy / len;
		f.y *= - dy / len;
		f.z  = s[idx].springconstant  * (len - s[idx].restlength);
		f.z += s[idx].dampingconstant * (p[p1].v.z - p[p2].v.z) * dz / len;
		f.z *= - dz / len;

		s[idx].springForce = make_float3(f.x,f.y,f.z);

		//if (!p[p1].fixed) 
		//{
		//	p[p1].f.x += f.x;
		//	p[p1].f.y += f.y;
		//	p[p1].f.z += f.z;
		//}
		//if (!p[p2].fixed)
		//{
		//	p[p2].f.x -= f.x;
		//	p[p2].f.y -= f.y;
		//	p[p2].f.z -= f.z;
		//}
	}
}


__global__ void kernel_calculateForces_global(PARTICLE *p,int np,
											  PARTICLEPHYS phys, PARTICLESPRING *s,int ns)
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;

	
	//double len,dx,dy,dz;
	

	if( idx < np )
	{
		int i ;//,p1,p2;
		float3 down = make_float3(0.0,0.0,-1.0);
		//float3 zero =  make_float3(0.0,0.0,0.0);
		float3 f =  make_float3(0.0,0.0,0.0);

		p[idx].f = make_float3(0.0,0.0,0.0);
		if ( !p[idx].fixed )
		{
			float m = p[idx].m ;
			float3 v = p[idx].v ;
			/* Gravitation */			/*p[idx].*/f.x += phys.gravitational * /*p[idx].*/m * down.x;			/*p[idx].*/f.y += phys.gravitational * /*p[idx].*/m * down.y;			/*p[idx].*/f.z += phys.gravitational * /*p[idx].*/m * down.z;			/* Viscous drag */			/*p[idx].*/f.x -= phys.viscousdrag * /*p[idx].*/v.x;			/*p[idx].*/f.y -= phys.viscousdrag * /*p[idx].*/v.y;			/*p[idx].*/f.z -= phys.viscousdrag */* p[idx].*/v.z;			p[idx].f = make_float3(f.x,f.y,f.z);			//printf(" GLOBAL : gravity %1.2f \n" , phys.gravitational);			//printf(" GLOBAL : viscousdrag %1.2f \n" , phys.viscousdrag);			//printf(" FORCES_GLOBAL : %1.2f ,%1.2f ,%1.2f , %1.2f \n" , p[idx].f.x ,p[idx].f.y ,p[idx].f.z, 2.0 ) ;
		}
	}
}



__global__ void kernel_calculateDerivatives(
	PARTICLE *p,int np,
	PARTICLEDERIVATIVES *deriv)
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;

	if( idx < np )
	{
		float3 dpdt = make_float3(0,0,0);
		float3 dvdt = make_float3(0,0,0);
		float3 v = p[idx].v ;
		float3 f = p[idx].f ;
		float m = p[idx].m ;

		dpdt.x = v.x;
		dpdt.y = v.y;
		dpdt.z = v.z;
		dvdt.x = f.x / m;
		dvdt.y = f.y / m;
		dvdt.z = f.z / m;

		deriv[idx].dpdt = make_float3(dpdt.x,dpdt.y,dpdt.z );
		deriv[idx].dvdt = make_float3(dvdt.x,dvdt.y,dvdt.z );
	}
}


__global__ void kernel_updatePositionAndVelocity(
					   PARTICLE *p,int np,
					   PARTICLEDERIVATIVES *deriv,double dt)
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;

	if( idx < np )
	{
		float3 pos = make_float3(0,0,0);
		float3 vel = make_float3(0,0,0);
		float3 dpdt = deriv[idx].dpdt ;
		float3 dvdt = deriv[idx].dvdt ;
		pos.x += dpdt.x * dt;
		pos.y += dpdt.y * dt;
		pos.z += dpdt.z * dt;
		vel.x += dvdt.x * dt;
		vel.y += dvdt.y * dt;
		vel.z += dvdt.z * dt;

		
		p[idx].p.x += pos.x ;
		p[idx].p.y += pos.y ;
		p[idx].p.z += pos.z ;

		p[idx].v.x += vel.x ;
		p[idx].v.y += vel.y ;
		p[idx].v.z += vel.z ;
		//printf(" pts : %1.2f ,%1.2f ,%1.2f , %1.2f \n" , p[idx].p.x ,p[idx].p.y ,p[idx].p.z, 2.0 ) ;
	}
}

__global__ void kernel_initialiseParticles(PARTICLE *p,int np)
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;

	if( idx < np )
	{
		p[idx].m = 1;
		p[idx].fixed = false;
		p[idx].v.x = 0;
		p[idx].v.y = 0;
		p[idx].v.z = 0;
	}
}

__global__ void kernel_initiliaseSprings(PARTICLE *p,int np,
										 PARTICLEPHYS phys, PARTICLESPRING *s,int ns)
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;

	if( idx < ns )
	{
		s[idx].springconstant = 1;
		s[idx].dampingconstant = 40;
		s[idx].restlength = 0.001 ;
	}

}


__global__ void kernel_fillVbo(PARTICLE *p,int np,float4 *dev_pos )
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;

	if( idx < np )
	{
		dev_pos[idx] = make_float4(p[idx].p.x,p[idx].p.y,p[idx].p.z,p[idx].p.z); ;
	}

}

__global__ void kernel_fillVbo(PARTICLE *p,int np,float4 *dev_pos ,float4 *dev_col , bbox b0,bbox b1, bbox b2 ,bbox b3 , 
							   float3 lamp0, float3 lamp1, float3 lamp2, float3 lamp3,float angle , bool translate , bool rotate , bool scale )
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;

	if( idx < np /*&& !p[idx].fixed*/ )
	{
		int i = 0;
		float d = 0;
		if( isInside(p[idx].p, b0 ))
		{
			point t = b0.min ; 
			i = 0 ;
			d = sqDist(p[idx].p,b0);
			transform ;
			dev_col[idx] =  make_float4(lamp0.x,lamp0.y,lamp0.z,1.0);
		}
		else if(  isInside(p[idx].p, b1 ) )
		{
			point t = b1.min ;
			i = 1 ;
			d = sqDist(p[idx].p,b1);
			transform;
			dev_col[idx] =  make_float4(lamp1.x,lamp1.y,lamp1.z,1.0);
		}
		else if(  isInside(p[idx].p, b2 ) )
		{
			point t = b2.min ;
			i = 2 ;
			d = sqDist(p[idx].p,b2);
			transform ;
			dev_col[idx] =  make_float4(lamp2.x,lamp2.y,lamp2.z,1.0);
		}
		else if(  isInside(p[idx].p, b3 ) )
		{
			point t = b3.min ;
			i = 3 ;
			d = sqDist(p[idx].p,b3);
			transform ;
			dev_col[idx] =  make_float4(lamp3.x,lamp3.y,lamp3.z,1.0);
			
		}
		else
		{

			dev_pos[idx] = make_float4(0,0,0,1);
			dev_col[idx] = make_float4(1,1,1,1);
			
		}

		if(scale)
		{
			dev_pos[idx].x *= 10 ;
			dev_pos[idx].y *= 10 ;
			dev_pos[idx].z *= 10 ;
		}
		
		
			
		
	}

}



/*
   Perform one step of the solver
*/
//#define _VERBOSE_


extern "C" void UpdateParticles(
								PARTICLE *p,int np,
								PARTICLEPHYS phys,
								PARTICLESPRING *s,int ns,
								double dt , /*PARTICLE *h_particles ,*/ float &gpuTime
								)
{
	cudaEvent_t start, stop; float time , cumTime ; time = cumTime = 0;; 

	dim3 dimBlock( int(sqrt(float(nparticles))) , 1 , 1 ); 
	int thr = float(nparticles) /( dimBlock.x * dimBlock.y * dimBlock.z )  + 1 ;//9;
	dim3 dimGrid(  thr * 1 , 1 ); 


	int threads = dimBlock.x * dimBlock.y * dimBlock.y * dimGrid.x * dimGrid.y * dimGrid.z ;
	//	printf("                 total number of threads : %i \ntotal number of particles : %i \n" , threads , nparticles );
	//	printf("                 ratio - threads to particles : %i , \n" , threads / nparticles );// there should be more threads than agents, else all the agents wont get updated.
	//	printf("                 threads per block : %i , \n" , dimGrid.x );
	//	printf("                 # blocks : %i , \n" , dimBlock.x );

	//timerStart ;

		kernel_calculateForces_springs<<<  int(sqrt(float(nsprings))), int(sqrt(float(nsprings)))+2 >>>(p,np,phys,s,ns);
			checkCUDAError(" kernel_calculateForces_springs ") ;

	//timerEnd;
	//cumTime += time ;
	//printf(" GPU : kernel_calculateForces_springs : time in Milliseconds : %1.4f , cumulative Time : %1.4f \n" , time , cumTime );

	//timerStart ;

		kernel_gatherPerParticleForceAndUpdate<<<dimGrid,dimBlock>>>( p, np,s,phys, ParticleCentricSpringIds , ParticleCentricSpringIds_baseAddress , dt) ;
			checkCUDAError(" kernel_gatherPerParticleForceAndUpdate ") ;

	//cumTime += time ;
	//printf(" GPU : kernel_gatherPerParticleForceAndUpdate : time in Milliseconds : %1.4f , cumulative Time : %1.4f \n" , time , cumTime );


}





//extern "C" void UpdateParticles(
//								PARTICLE *p,int np,
//								PARTICLEPHYS phys,
//								PARTICLESPRING *s,int ns,
//								double dt , /*PARTICLE *h_particles ,*/ float &gpuTime
//								)
//{
//	cudaEvent_t start, stop; float time , cumTime ; time = cumTime = 0;; 
//	
//	dim3 dimBlock( int(sqrt(float(nparticles))) , 1 , 1 ); 
//	int thr = float(nparticles) /( dimBlock.x * dimBlock.y * dimBlock.z )  + 1 ;//9;
//	dim3 dimGrid(  thr * 1 , 1 ); 
//
//
//	int threads = dimBlock.x * dimBlock.y * dimBlock.y * dimGrid.x * dimGrid.y * dimGrid.z ;
////	printf("                 total number of threads : %i \ntotal number of particles : %i \n" , threads , nparticles );
////	printf("                 ratio - threads to particles : %i , \n" , threads / nparticles );// there should be more threads than agents, else all the agents wont get updated.
////	printf("                 threads per block : %i , \n" , dimGrid.x );
////	printf("                 # blocks : %i , \n" , dimBlock.x );
//
//	
//	/*PARTICLEDERIVATIVES *deriv;*/
//
//	/*deriv = (PARTICLEDERIVATIVES *)*/
//	//cudaMalloc((void**)&deriv ,np * sizeof(PARTICLEDERIVATIVES));
//	//checkCUDAError(" malloc deriv ") ;
//
//    /* Euler */
//	timerStart;
//		kernel_calculateForces_global<<<dimGrid,dimBlock>>>(p,np,phys,s,ns);
//			checkCUDAError(" kernel_calculateForces_global ") ;
//	timerEnd;
//	cumTime+=time;
//	printf(" GPU : kernel_calculateForces_global : time in Milliseconds : %1.4f \n" , time );
//
//	timerStart;
//		//kernel_calculateForces_springs_serial<<<1,1>>>(p,np,phys,s,ns);
//		kernel_calculateForces_springs<<<  int(sqrt(float(nsprings))), int(sqrt(float(nsprings)))+2 >>>(p,np,phys,s,ns);
//			checkCUDAError(" kernel_calculateForces_springs ") ;
//	timerEnd;
//	cumTime+=time;
//	printf(" GPU : kernel_calculateForces_springs : time in Milliseconds : %1.4f \n" , time );
//
//	timerStart;
//		kernel_gatherPerParticleForceAndUpdate<<<dimGrid,dimBlock>>>( p, np,s,/*deriv,*/ ParticleCentricSpringIds , ParticleCentricSpringIds_baseAddress , dt) ;
//			checkCUDAError(" kernel_gatherPerParticleForceAndUpdate ") ;
//	timerEnd;
//	cumTime+=time;
//	printf(" GPU : kernel_gatherPerParticleForceAndUpdate : time in Milliseconds : %1.4f \n" , time );
//
//	timerStart;
//		kernel_calculateDerivatives<<<dimGrid,dimBlock>>>(p,np,deriv);
//			checkCUDAError(" kernel_calculateDerivatives ") ;
//	timerEnd;
//	cumTime+=time;
//	printf(" GPU : kernel_calculateDerivatives : time in Milliseconds : %1.4f \n" , time );
//
//	timerStart;
//		kernel_updatePositionAndVelocity<<<dimGrid,dimBlock>>>( p,np,deriv,dt );
//			checkCUDAError(" kernel_updatePositionAndVelocity ") ;
//	timerEnd;
//	cumTime+=time;
//	printf( " GPU : kernel_updatePositionAndVelocity : time in Milliseconds : %1.4f \n" , time );
//
//	gpuTime = cumTime ;
//
//	//if (h_particles != NULL)
//	//	free(h_particles);
//	//h_particles = ( PARTICLE *)malloc(nparticles*sizeof(PARTICLE));
//	//printf(" GPU : UPDATE_SYSTEM - without memcpy D-H : time in Milliseconds : %1.4f \n" , cumTime );
//
//	//timerStart;
//	//cudaMemcpy(h_particles,particles,nparticles * sizeof(PARTICLE),cudaMemcpyDeviceToHost);
//	//	checkCUDAError( " cudaMemcpy D_H ");
//	//timerEnd;
//	//cumTime+=time;
//	//printf(" GPU : CUM_TIME : time in Milliseconds : %1.4f \n" , cumTime );
//	gpuTime = cumTime ;
//
//	cudaEventDestroy( start ); 
//	cudaEventDestroy( stop );
//
//	//cudaFree(deriv);
//
//	//kernel_calculateForces_springs<<<  int(sqrt(float(nsprings))), int(sqrt(float(nsprings)))+2 >>>(p,np,phys,s,ns);
//	//checkCUDAError(" kernel_calculateForces_springs ") ;
//
//	//kernel_gatherPerParticleForceAndUpdate<<<dimGrid,dimBlock>>>( p, np,s,/*deriv,*/ ParticleCentricSpringIds , ParticleCentricSpringIds_baseAddress , dt) ;
//	//checkCUDAError(" kernel_gatherPerParticleForceAndUpdate ") ;
//
//}
//
//



extern "C" int SetupParticles(int np,int ns)
{
   int i;

   nparticles = np;
   nsprings = ns;


   

   if (particles != NULL)
      cudaFree(particles);
	cudaMalloc( /*(void **)*/ &particles , nparticles * sizeof(PARTICLE) );
								checkCUDAError(" cudaMalloc - particles ");
   if (springs != NULL)
      cudaFree(springs);
	cudaMalloc( /*(void **)*/ &springs ,nsprings * sizeof(PARTICLESPRING));
									checkCUDAError(" cudaMalloc - springs ");

   physical.gravitational = 0;
   physical.viscousdrag = 0.1;

   return 1; 
}



extern "C" void InitialiseSystem( PARTICLE *h_p , int h_np , PARTICLESPRING *h_s , int h_ns , int *h_ParticleCentricSpringIds , int4 *h_ParticleCentricSpringIds_baseAddress , int h_baseAddress /* cnt*/) 
{
   int i;

	numBlocks = int(h_np / 1024) ;

	//  cudaError_t result ;
	//  int device ;
	//  size_t uCurAvailMemoryInBytes;
	//  size_t uTotalMemoryInBytes;
	//  int nNoOfGPUs;
	//  CUcontext context;

	//  {
	//   int nID =  cu_gpuGetMaxGflopsDeviceId();
	//   cudaGetDevice( &nID ); // Get handle for device
	//   cuCtxCreate( &context, 0, device ); // Create context
	//   result = cudaMemGetInfo( &uCurAvailMemoryInBytes, &uTotalMemoryInBytes );

	//   if( result == cudaSuccess )
	//   {
	//	   printf( "Device: %d\nTotal Memory: %d MB, Free Memory: %d MB\n",
	//		   nID,
	//		   uTotalMemoryInBytes / ( 1024 * 1024 ),
	//		   uCurAvailMemoryInBytes / ( 1024 * 1024 ));
	//   }
	//   cuDetach( context ); // Destroy context

	//  }

	//  float memNeeded =  float(h_np) * sizeof(PARTICLE) + float(h_ns) * sizeof(PARTICLESPRING) ;
	//  printf( "Device: Memory need  %1.2f MB\n",  memNeeded / ( 1024 * 1024 ));

	//if( memNeeded >= uCurAvailMemoryInBytes ) 
	//{
	//	 printf( " required memory not available, reconfigure arguments ");
	//	// return 0 ;
	//}

	printf( " -------------------------- # NP %i , # NS %i :  \n" , h_np , h_ns );
	SetupParticles(h_np,h_ns);
  
	cudaMemcpy(particles,h_p,h_np*sizeof(PARTICLE),cudaMemcpyHostToDevice);
		checkCUDAError("cudaMemcpy particles H-D");

	cudaMemcpy(springs,h_s,h_ns*sizeof(PARTICLESPRING),cudaMemcpyHostToDevice);
		checkCUDAError("cudaMemcpy springs H-D");
		
	cudaMalloc((void**)&deriv ,h_np * sizeof(PARTICLEDERIVATIVES));
		checkCUDAError(" malloc deriv ") ;


		cudaMalloc( /*(void **)*/ &particles_buf , h_np * sizeof(PARTICLE) );
		checkCUDAError(" cudaMalloc - verts ");

		cudaMemcpy(particles_buf,particles,h_np*sizeof(PARTICLE),cudaMemcpyDeviceToDevice);
		checkCUDAError(" cudaMemcpy verts H-D ");

	// ----------- 
		baseAddress = h_baseAddress ;
	if (ParticleCentricSpringIds != NULL)
		cudaFree(ParticleCentricSpringIds);
	if (ParticleCentricSpringIds_baseAddress != NULL)
		cudaFree(ParticleCentricSpringIds_baseAddress);


	int maxEdgesPerVertex = 8; 
	cudaMalloc( &ParticleCentricSpringIds , baseAddress * sizeof( int ) );
			checkCUDAError("cudaMalloc ParticleCentricSpringIds ");
	cudaMalloc( &ParticleCentricSpringIds_baseAddress , h_np * sizeof( int4) );
			checkCUDAError("cudaMalloc ParticleCentricSpringIds_baseAddress ");

	cudaMemcpy( ParticleCentricSpringIds, h_ParticleCentricSpringIds , baseAddress * sizeof( int ) ,cudaMemcpyHostToDevice ) ;
			checkCUDAError("cudaMemcpy ParticleCentricSpringIds_baseAddress H-D ");
	cudaMemcpy( ParticleCentricSpringIds_baseAddress, h_ParticleCentricSpringIds_baseAddress , h_np * sizeof( int4) ,cudaMemcpyHostToDevice ) ;
			checkCUDAError("cudaMemcpy ParticleCentricSpringIds_baseAddress H-D ");
			
	
}

extern "C" void UpdateSystem( int iterations/*PARTICLE *h_particles ,int &h_nparticles , float &gpuTime*/ /*, float3* dev_pos*/  ) 
{

	double dt = 0.01 ;
	cudaEvent_t start, stop; float time , cumTime = 0;; 
	//int iterations = 10 ;

	#ifdef _VERBOSE

		printf(" -------------------------------- GPU UPDATE_NEW -------------------------------- \n ") ;
		printf( " -------------------------- # NParticles %i , # NSprings %i , #iterations %i  \n" , nparticles , nsprings ,  iterations  );
		timerStart ;
	
	#endif // _VERBOSE
	
	for( int i =0 ; i < iterations ; i++ )
		UpdateParticles(particles,nparticles,physical,springs,nsprings,dt, /*h_particles ,*/ cumTime );
	
	#ifdef _VERBOSE
		timerEnd;
		printf(" GPU : UpdateSystem : time in Milliseconds : %1.4f \n" , time );
	#endif // _VERBOSE

	//gpuTime = cumTime ;


	//printf(" -------------------------------- GPU UPDATE -------------------------------- \n ") ;
}

extern "C" void runStreams( float dt/*PARTICLE *h_particles ,int &h_nparticles , float &gpuTime*/ /*, float3* dev_pos*/  ) 
{

	/*double dt = 0.1 ;*/
	cudaEvent_t start, stop; float time , cumTime = 0;; 
	int iterations = 1 ;

	#ifdef _VERBOSE

		printf(" -------------------------------- GPU UPDATE_NEW -------------------------------- \n ") ;
		printf( " -------------------------- # NParticles %i , # NSprings %i , #iterations %i  \n" , nparticles , nsprings ,  iterations  );
		timerStart ;

	#endif
	//for( int i =0 ; i < iterations ; i++ )
	//	UpdateParticles(particles,nparticles,physical,springs,nsprings,dt, /*h_particles ,*/ cumTime );


	{
		
		//kernel_clearNeighborTags<<< int( sqrt(float(nparticles))) , int( sqrt(float(nparticles)))+2 >>>( particles , nparticles  ); 
			//checkCUDAError(" kernel_clearNeighborTags ");
		kernel_moveToNearest<<< int( sqrt(float(nparticles))) , int( sqrt(float(nparticles)))+2 >>>( particles , particles_buf ,nparticles , dt ); 
			checkCUDAError(" kernel_moveToNearest ");

		cudaMemcpy(particles,particles_buf,nparticles*sizeof(PARTICLE),cudaMemcpyDeviceToDevice);
			checkCUDAError(" cudaMemcpy verts H-D ");
	}


	#ifdef _VERBOSE

		timerEnd;
		printf(" GPU : UpdateSystem : time in Milliseconds : %1.4f \n" , time );

	#endif

	//gpuTime = cumTime ;


	//printf(" -------------------------------- GPU UPDATE -------------------------------- \n ") ;
}


extern "C" void fillVbo( float4 *dev_pos ) 
{
	kernel_fillVbo<<< int(sqrt(float(nparticles))) , int(sqrt(float(nparticles)))+2 >>>(particles,nparticles,dev_pos);
		checkCUDAError("kernel_fillVbo");
}

extern "C" void fillVbo_culled( float4 *dev_pos ,float4 *dev_col, bbox b1 , bbox b2 , bbox b3 , bbox b4 , float angle , bool translate , bool rotate , bool scale ) 
{
	kernel_fillVbo<<< int(sqrt(float(nparticles))) , int(sqrt(float(nparticles)))+2 >>>(particles,nparticles,dev_pos,dev_col, b1 ,  b2 ,  b3 ,  b4 , 
		lamp0,lamp1,lamp2,lamp3, angle , translate , rotate , scale);
	checkCUDAError("kernel_fillVbo");
}

extern "C" void updateToplogy( PARTICLE *h_p , int h_np , PARTICLESPRING *h_s , int h_ns , int *h_ParticleCentricSpringIds , int4 *h_ParticleCentricSpringIds_baseAddress , int h_baseAddress )
{
	//cudaMemset(particles,0,h_np*sizeof(PARTICLE));
	//cudaMemset(springs,0,h_ns*sizeof(PARTICLESPRING));

	//if( nparticles != h_np )
	//{
	//	/*if (particles != NULL)
	//		cudaFree(particles);*/
	//	cudaMalloc( /*(void **)*/ &particles , nparticles * sizeof(PARTICLE) );
	//	checkCUDAError(" cudaMalloc - particles ");
	//}
	//if( nsprings != h_ns )
	//{
	///*	if (springs != NULL)
	//		cudaFree(springs);*/
	//	cudaMalloc( /*(void **)*/ &springs ,nsprings * sizeof(PARTICLESPRING));
	//	checkCUDAError(" cudaMalloc - springs ");
	//}

	//if( baseAddress != h_baseAddress )
	//{
	///*	if (ParticleCentricSpringIds != NULL)
	//		cudaFree(ParticleCentricSpringIds);*/
	//	cudaMalloc( &ParticleCentricSpringIds , baseAddress * sizeof( int ) );
	//	checkCUDAError("cudaMalloc ParticleCentricSpringIds ");
	//}

	//if( nparticles != h_np )
	//{

	//	/*if (ParticleCentricSpringIds_baseAddress != NULL)
	//		cudaFree(ParticleCentricSpringIds_baseAddress);	*/
	//	cudaMalloc( &ParticleCentricSpringIds_baseAddress , h_np * sizeof( int4) );
	//	checkCUDAError("cudaMalloc ParticleCentricSpringIds_baseAddress ");
	//}

	nparticles = h_np ;
	nsprings= h_ns ;
	baseAddress = h_baseAddress ;

	cudaMemcpy(particles,h_p,h_np*sizeof(PARTICLE),cudaMemcpyHostToDevice);
		checkCUDAError("cudaMemcpy particles H-D");

	cudaMemcpy(springs,h_s,h_ns*sizeof(PARTICLESPRING),cudaMemcpyHostToDevice);
		checkCUDAError("cudaMemcpy springs H-D");

	cudaMemcpy( ParticleCentricSpringIds, h_ParticleCentricSpringIds , baseAddress * sizeof( int ) ,cudaMemcpyHostToDevice ) ;
		checkCUDAError("cudaMemcpy ParticleCentricSpringIds_baseAddress H-D ");

	cudaMemcpy( ParticleCentricSpringIds_baseAddress, h_ParticleCentricSpringIds_baseAddress , h_np * sizeof( int4) ,cudaMemcpyHostToDevice ) ;
		checkCUDAError("cudaMemcpy ParticleCentricSpringIds_baseAddress H-D ");

}


extern "C" void updateFixedPtsHt( float craneHts[4][1] )
{
	kernel_updateFixedPts<<<1,1>>>(particles,fixed0,n_f0,craneHts[0][1]);
		checkCUDAError("kernel_updateFixedPts");

	kernel_updateFixedPts<<<1,1>>>(particles,fixed1,n_f1,craneHts[1][1]);
		checkCUDAError("kernel_updateFixedPts");
	kernel_updateFixedPts<<<1,1>>>(particles,fixed2,n_f2,craneHts[2][1]);
		checkCUDAError("kernel_updateFixedPts");
	kernel_updateFixedPts<<<1,1>>>(particles,fixed3,n_f3,craneHts[3][1]);
		checkCUDAError("kernel_updateFixedPts");
}

extern "C" void updateLamps( float lampRgb[4][3] )
{
	lamp0 = make_float3(lampRgb[0][0],lampRgb[0][1],lampRgb[0][2]) ;
	lamp1 = make_float3(lampRgb[1][0],lampRgb[1][1],lampRgb[1][2]) ;
	lamp2 = make_float3(lampRgb[2][0],lampRgb[2][1],lampRgb[2][2]) ;
	lamp3 = make_float3(lampRgb[3][0],lampRgb[3][1],lampRgb[3][2]) ;
}
extern "C" void copyFixedPts( int *h_fixed0 ,  int *h_fixed1 ,  int *h_fixed2 , int *h_fixed3 , int h_n_f0,int h_n_f1,int h_n_f2 ,int h_n_f3 )
{

	n_f0 = h_n_f0;
	n_f1 = h_n_f1;
	n_f2 = h_n_f2;
	n_f3 = h_n_f3;
	
	{
		cudaMalloc((void**)&fixed0 ,n_f0 * sizeof(int));
		checkCUDAError(" malloc fixed0 ") ;

		cudaMemcpy(fixed0,h_fixed0,n_f0*sizeof(int),cudaMemcpyHostToDevice);
		checkCUDAError("cudaMemcpy particles H-D");
	}

	{
		cudaMalloc((void**)&fixed1 ,n_f1 * sizeof(int));
		checkCUDAError(" malloc fixed0 ") ;

		cudaMemcpy(fixed1,h_fixed1,n_f1*sizeof(int),cudaMemcpyHostToDevice);
		checkCUDAError("cudaMemcpy particles H-D");
	}

	{
		cudaMalloc((void**)&fixed2,n_f2 * sizeof(int));
		checkCUDAError(" malloc fixed0 ") ;

		cudaMemcpy(fixed2,h_fixed2,n_f2*sizeof(int),cudaMemcpyHostToDevice);
		checkCUDAError("cudaMemcpy particles H-D");
	}

	{
		cudaMalloc((void**)&fixed3,n_f3 * sizeof(int));
		checkCUDAError(" malloc fixed0 ") ;

		cudaMemcpy(fixed3,h_fixed3,n_f3*sizeof(int),cudaMemcpyHostToDevice);
		checkCUDAError("cudaMemcpy particles H-D");
	}


}

extern "C" void copyParticlePositions( PARTICLE *h_p )
{
	cudaMemcpy(h_p,particles,nparticles*sizeof(PARTICLE),cudaMemcpyDeviceToHost);
		checkCUDAError( "cudamemcy - particles to Host" );
}
extern "C" void CUDA_cleanup()
{

	cudaFree(particles);
		checkCUDAError("cudaFree : particles ");
	cudaFree(springs);
		checkCUDAError("cudaFree : springs ");
	cudaFree(ParticleCentricSpringIds);
		checkCUDAError("cudaFree : springs ");
	cudaFree(ParticleCentricSpringIds_baseAddress);
		checkCUDAError("cudaFree : springs ");
}