#ifndef _APP
#define _APP 

#include "ALICE_DLL.h"
using namespace Alice;

#include "AL_gl2psUtils.h" // vector screen capture 

#include "ALICE_ROBOT_DLL.h" // robot kinematics
using namespace ROBOTICS;

#include "CONTROLLER.h" // keyboard and mouse tracker
#include "MODEL.h"// picking and selection

#include "utilities.h"

// ----------------------------------------------------------------------------- CUDA INCLUDES & DECLARATIONS

#ifdef _CUDA_



//#include "windows.h" 
////IMP !!! have to include this before cuda_gl_interop.h .. strange errors otherwise
//
//namespace cud
//{
//
////#include "channel_descriptor.h"
//#include "cuda_runtime_api.h"
//#include "vector_types.h"	
//#include "vector_functions.h"
//#include <cuda_gl_interop.h>
//
//}
#include "cuda_functions.h"
#define GL_GLEXT_PROTOTYPES

float	pos[MAX_DEVICE_ARRAY_SIZE][4];
float	col[MAX_DEVICE_ARRAY_SIZE][4];

struct cud::cudaGraphicsResource* posVbo_res;
struct cud::cudaGraphicsResource* colVbo_res;

#endif // _CUDA

float  					gTotalTimeElapsed = 0;
int 					gTotalFrames = 0;
GLuint 					gTimer;



void init_timer()
{
	glGenQueries(1, &gTimer);
}

void start_timing()
{
	glBeginQuery(GL_TIME_ELAPSED, gTimer);
}

float stop_timing()
{
	glEndQuery(GL_TIME_ELAPSED);

	GLint available = GL_FALSE;
	while (available == GL_FALSE)
		glGetQueryObjectiv(gTimer, GL_QUERY_RESULT_AVAILABLE, &available);

	GLint result;
	glGetQueryObjectiv(gTimer, GL_QUERY_RESULT, &result);

	float timeElapsed = result / (1000.0f * 1000.0f * 1000.0f);
	return timeElapsed;
}

void displayFPS(float timeElapsed)
{
	gTotalFrames++;
	gTotalTimeElapsed += timeElapsed;
	float fps = gTotalFrames / gTotalTimeElapsed;
	char string[1024] = { 0 };
	sprintf(string, "FPS: %0.2f FPS", fps);
	glutSetWindowTitle(string);
}


#ifdef _CUDA_


//------------------------------------------------------------------------------- CUDA 

bool vboInited = false;
bool glewInited = false;
bool cudaInited = false;
bool CUDA_INITED = false;

GLuint posVbo;
GLuint colVbo;

static void checkCUDAError(const char *msg)
{
	cud::cudaError_t err = cud::cudaGetLastError();
	if (cud::cudaSuccess != err) {
		fprintf(stderr, "Cuda error: %s: %s.\n", msg, cud::cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}
}
inline int _ConvertSMVer2Cores(int major, int minor)
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
	{ -1, -1 }
	};

	int index = 0;
	while (nGpuArchCoresPerSM[index].SM != -1) {
		if (nGpuArchCoresPerSM[index].SM == ((major << 4) + minor)) {
			return nGpuArchCoresPerSM[index].Cores;
		}
		index++;
	}
	printf("MapSMtoCores undefined SM %d.%d is undefined (please update to the latest SDK)!\n", major, minor);
	return -1;
}
int gpuGetMaxGflopsDeviceId()
{
	int current_device = 0, sm_per_multiproc = 0;
	int max_compute_perf = 0, max_perf_device = 0;
	int device_count = 0, best_SM_arch = 0;
	cud::cudaDeviceProp deviceProp;
	cud::cudaGetDeviceCount(&device_count);

	// Find the best major SM Architecture GPU device
	while (current_device < device_count)
	{
		cud::cudaGetDeviceProperties(&deviceProp, current_device);
		if (deviceProp.major > 0 && deviceProp.major < 9999)
		{
			best_SM_arch = MAX(best_SM_arch, deviceProp.major);
		}
		current_device++;
	}

	// Find the best CUDA capable GPU device
	current_device = 0;
	while (current_device < device_count)
	{
		cud::cudaGetDeviceProperties(&deviceProp, current_device);
		if (deviceProp.major == 9999 && deviceProp.minor == 9999)
		{
			sm_per_multiproc = 1;
		}
		else
		{
			sm_per_multiproc = _ConvertSMVer2Cores(deviceProp.major, deviceProp.minor);
		}

		int compute_perf = deviceProp.multiProcessorCount * sm_per_multiproc * deviceProp.clockRate;

		if (compute_perf > max_compute_perf)
		{
			// If we find GPU with SM major > 2, search only these
			if (best_SM_arch > 2)
			{
				// If our device==dest_SM_arch, choose this, or else pass
				if (deviceProp.major == best_SM_arch)
				{
					max_compute_perf = compute_perf;
					max_perf_device = current_device;
				}
			}
			else
			{
				max_compute_perf = compute_perf;
				max_perf_device = current_device;
			}
		}
		++current_device;
	}
	return max_perf_device;
}
void initialiseHostDrawArrays()
{
	for (int i = 0; i < MAX_DEVICE_ARRAY_SIZE; i++)
	{
		pos[i][0] = col[i][0] = 0; //ofRandom(0, 1);
		pos[i][1] = col[i][1] = 0; //ofRandom(0, 1);
		pos[i][2] = col[i][2] = 0; //ofRandom(0, 1);
		pos[i][3] = col[i][3] = 1;
	}

}
void generateHostDataAndCopyToDevice()
{
	CUDA_STRUCT *h_dataArray;
	h_dataArray = new CUDA_STRUCT[MAX_DEVICE_ARRAY_SIZE];

	for (int i = 0; i < MAX_DEVICE_ARRAY_SIZE; i++)
	{
		h_dataArray[i].m = 1.0;
		h_dataArray[i].p = cud::make_float3(ofRandom(-100, 100), ofRandom(-100, 100), ofRandom(-100, 100));
	}

	copyData_HostToDevice(h_dataArray, MAX_DEVICE_ARRAY_SIZE);
	delete h_dataArray;
}
int InitialiseCUDADeviceAndBuffers()
{
	int maxNumParticles = MAX_POINTS;

	printf(" -------------------------------------- CUDA INIT ///////// --------------------------------------   \n");


	if (!cudaInited)
	{
		cudaInited = true;
		cud::cudaGLSetGLDevice(gpuGetMaxGflopsDeviceId());
		checkCUDAError(" cudaGLSetGLDevice ");
		printf("device set : %i \n", gpuGetMaxGflopsDeviceId());
	}


	if (!vboInited)
	{

		glewInit();
		if (glewIsSupported("GL_VERSION_2_0"))
			printf("Ready for OpenGL 2.0\n");
		else
		{
			printf("OpenGL 2.0 not supported\n");
			exit(1);
		}


		glGenBuffersARB(1, &posVbo);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, posVbo);
		//glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(cud::float4)*maxNumParticles, 0, GL_DYNAMIC_DRAW_ARB);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(pos), col, GL_STREAM_DRAW_ARB);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffersARB(1, &colVbo);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, colVbo);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(col), col, GL_STREAM_DRAW_ARB);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		cud::cudaGraphicsGLRegisterBuffer(&posVbo_res, posVbo, cud::cudaGraphicsMapFlagsWriteDiscard);
		cud::cudaGraphicsGLRegisterBuffer(&colVbo_res, colVbo, cud::cudaGraphicsMapFlagsWriteDiscard);

		// --------------- 


		vboInited = true;

	}


	return 1;
}

#endif // _CUDA_

//------------------------------------------------------------------------------- FORWARD DECLARATIONS for functions

void setup();
void update(int value);
void draw() ;
void keyPress(unsigned char k, int xm, int ym);
void mousePress(int b,int s,int x,int y) ;
void mouseMotion( int x, int y ) ;

//------------------------------------------------------------------------------- APPLICATION VARIABLES


bool HUDSelectOn = false;
bool updateCam = true;
int counter = 0;
string inFile = "";
CONTROLLER CONTROLLERS;
MODEL SCENE;
ButtonGroup B;
SliderGroup S;
//------------------------------------------------------------------------------- CALLBACKS

void updateCallBack( int value )
{
	long start = GetTickCount();

		update( value ) ;
		CONTROLLERS.update(SCENE);

		frame ++ ;
		winW = glutGet(GLUT_WINDOW_WIDTH);
		winH = glutGet(GLUT_WINDOW_HEIGHT);
		
		glutPostRedisplay() ;// refresh your screen i.e internally this will call draw() ;
		glutTimerFunc(10,updateCallBack,0); // call update every 10 millisecs ;

	long end = GetTickCount();
	elapsedTime = end - startTime ;
	long time = (end-start) ;
	if( time < 10 )time = 10 ;

}

void drawCallBack()
{
	
	long start = GetTickCount();
	
		float currentColor[4];
		glGetFloatv(GL_CLEAR,currentColor) ;

		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); // background(255) ;
		if(saveF)
		{
			buffer.Init( screenW , screenH ); // specify dimensions of the image file, max : 4000 x 4000 pixels ;	
			buffer.beginRecord(0.95); // record to offScreen texture , additionally specify a background clr, default is black
	
		}
		/*else
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0 );*/
		//////////////////////////////////////////////////////////////////////////

			if(updateCam)updateCamera() ;
			glColor3f(1,1,1);
			//drawGrid( gridSz );	

			draw() ;



//			SCENE.performWindowSelection(CONTROLLERS);
			SCENE.draw();
			CONTROLLERS.update(SCENE); // !!!! temp
			CONTROLLERS.draw(SCENE);
			S.draw();
			B.draw();
		//////////////////////////////////////////////////////////////////////////

		if (saveF)
		{
			buffer.endRecord(true, numFrames); // stop recording, additionally specify save texture to disk, and a max number of frames to save ;
			buffer.drawTexture(proj_matrix, mv_matrix); // NOT WORKING draw recorded off-screen texture as image unto current screen
		}

		glutSwapBuffers();
	
	long end = GetTickCount();
	elapsedTime = end - startTime ;
	long time = (end-start) ;
	if( time < 10 )time = 10 ;

	
}

void keyPressCallBack(unsigned char k, int xm, int ym)
{

	CONTROLLERS.keyPress(k, xm, ym ,SCENE);

	if (k == 'R')setup();
	if( k == 'X' )exit(0) ;
	if( k == 'V' )resetCamera() ;
	if( k == 'T' )topCamera();
	if( k == 'F' )
	{
		numFrames = 1200 ; 
		int nf = 25 ;
		screenW = winW * 1 ;
		screenH = winH * 1 ;
		setPrintScreenAttribs(screenW,screenH,nf,true);
		
		saveF = !saveF ;
		setSaveFrame(saveF);

		if( saveF ) cout << " printing screen " << endl ;
		else cout << " NOT printing screen " << endl ;
	}
	if (k == 'E')
	{
		FILE *fp;
		int state = GL2PS_OVERFLOW, buffsize = 0;


		string file = "";
		file += "data/out";
		file += "_";
		char s[20];
		itoa(counter, s, 10);
		file += s;
		file += ".eps";

		fp = fopen(file.c_str(), "w");
		cout << file.c_str() << endl;
		printf("Writing 'out.eps'... ");

		while (state == GL2PS_OVERFLOW)
		{
			buffsize += winW * winH;
			gl2psBeginPage("test", "gl2psTestSimple", NULL, GL2PS_EPS, GL2PS_NO_SORT,
				GL2PS_USE_CURRENT_VIEWPORT,
				GL_RGBA, 0, NULL, 0, 0, 0, buffsize, fp, file.c_str());

			draw();



//			SCENE.performWindowSelection(CONTROLLERS);
			SCENE.draw();
			CONTROLLERS.draw(SCENE);

			state = gl2psEndPage();
		}

		fclose(fp);
		printf("Done!\n");

		counter++;

	}
	
	
	keyPress(k,xm,ym);
}

void mousePressCallBack(int b,int s,int x,int y)
{

	 CONTROLLERS.mousePress(b, s, x, y);


	 if (GLUT_LEFT_BUTTON == b && GLUT_DOWN == s)
	 {

		 B.performSelection(x, y);
		 S.performSelection(x, y, HUDSelectOn);
	 }

	 mousePress( b, s, x, y) ;

	 updateCam = (glutGetModifiers() == GLUT_ACTIVE_ALT) ? false : true;
	 
	 if(updateCam )Mouse( b, s, x, y) ;
}

void motionCallBack( int x, int y )
{

	CONTROLLERS.mouseMotion(x, y);

	S.performSelection(x, y, HUDSelectOn);
	B.performSelection(x, y);

	mouseMotion(x,y);
	if(!HUDSelectOn && updateCam)Motion( x, y) ;
}

//------------------------------------------------------------------------------- ENTRY POINT 

int main(int argc,char** argv)
{
	

	if (argc > 1)inFile = argv[1];

	

	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
	glutInitWindowSize(winW,winH);
	glutInitWindowPosition(200,100);
	glutCreateWindow("Procedural Building");

	// register event methods ;
	glutDisplayFunc(drawCallBack); // register a drawing code 
	glutReshapeFunc(reshape); // register a reshape 
	glutMouseFunc(mousePressCallBack);
	glutMotionFunc(motionCallBack);
	glutIdleFunc(idle);//(NEW) calls our idle function
	glutTimerFunc(0,updateCallBack,0); // call update once in main
	glutKeyboardFunc( keyPressCallBack ); // register keypress function;

	{
		glewInit();

		if (glewIsSupported("GL_VERSION_2_0"))
		{
			printf("Ready for OpenGL 2.0\n");
			//glewInited = true ;
		}
		else 
			printf("OpenGL 2.0 not supported\n");
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POINT_SMOOTH);

	
#ifdef _CUDA_

	// ---------------------------- CUDA INIT

	initialiseHostDrawArrays();
	InitialiseCUDADeviceAndBuffers();
	initialiseDeviceMemory();
	generateHostDataAndCopyToDevice();
	CUDA_INITED = true;

#endif // _CUDA_
	{
		B = *new ButtonGroup(vec(50, 450, 0));
		setup();
	}

	init_timer();
	glutMainLoop();

	
}

//#include <qapplication.h>
//
//int main(int argc, char** argv)
//{
//	// Read command lines arguments.
//	QApplication application(argc, argv);
//
//	// Instantiate the viewer.
//	Viewer viewer;
//
//	viewer.setWindowTitle("simpleViewer");
//
//	// Make the viewer window visible on screen.
//	viewer.show();
//
//	// Run main loop.
//	return application.exec();
//}
//
//
#endif