#ifndef _PARTICLE_CUDA
#define _PARTICLE_CUDA

//
#include <stdio.h>
#include <sstream>
#include "windows.h" 
//IMP !!! have to include this before cuda_gl_interop.h .. strange errors otherwise


namespace cud
{
	#include "cuda_runtime_api.h"
	#include "vector_types.h"	
	#include "vector_functions.h"
	#include <cuda_gl_interop.h>
}


#define timerStart \
	timer.beginTimer();\

#define timerEnd \
	timer.endTimer();\
	time = timer.elapsedTime(); \
	cumTime+=time;\

typedef struct 
{
	double m;      /*8 bytes*/
	cud::float3 p;      /*12 bytes*/
	float dummy[3];/*12 bytes*/

} CUDA_STRUCT;

#define  MAX_DEVICE_ARRAY_SIZE 75000

//////////////////////////////////////////////////////////////////////////  -------------------------------------------  EXTERNS

//DATA-TRANSFER

extern "C" void initialiseDeviceMemory() ;
extern "C" void copyData_HostToDevice( CUDA_STRUCT *h_dataArray , int nElems ) ;
extern "C" CUDA_STRUCT * copyData_DeviceToHost( CUDA_STRUCT *h_dataArray , int &n ) ;

// UPDATE DATA

extern "C" void updateDeviceData() ;
extern "C" float runStreams() ;

//DISPLAY

extern "C" void fillVbo( cud::float4 *dev_pos ) ;

// EXIT
extern "C" void CUDA_cleanup() ;


//////////////////////////////////////////////////////////////////////////

#endif