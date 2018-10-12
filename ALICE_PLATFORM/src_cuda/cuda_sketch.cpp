#define _CUDA_
#include "main.h"


bool run = false;
bool streams = false ;
SliderGroup s;

class nodalSurf : public isoSurf
{
public:
	double a1 , a2 , a3 , a4 , a5 , a6 ; 

	nodalSurf(){} ;
	nodalSurf( int _iDataSetSize , float _fTargetValue  )
	{
		a1 = a2 = a3 = a4 = a5 = a6 = 1.0f ;
		boxDim = 1.0 ; ;
		iDataSetSize = _iDataSetSize ;
	}

	virtual void  boundingBox()
	{
		#define EPSILON pow(10.0f,-3.0f)
		b_min = vec(EPSILON,EPSILON,EPSILON);
		b_max =  vec(PI-EPSILON,PI-EPSILON,PI-EPSILON);
		boxDim = b_max.x - b_min.x ; 

		fStepSize = boxDim/float(iDataSetSize);
	}

	virtual float  fSample(float x, float y, float z) 
	{
		float fResult = (a1 * cos (1*x) * cos(2*y) * cos(3*z)) +
			(a3 * cos (2*x) * cos(1*y) * cos(3*z)) +
			(a4 * cos (2*x) * cos(3*y) * cos(1*z)) +
			(a5 * sin (3*x) * sin(1*y) * sin(2*z)) +
			(a2 * sin (1*x) * sin(3*y) * sin(2*z))+
			(a6 * sin (3*x) * sin(2*y) * sin(1*z));

		return fResult ;
	}
};

nodalSurf iso ;
int n;
vec cen;

void setup() 
{

	iso = *new nodalSurf(15,0.0);

	s = * new SliderGroup( ) ;

	for( int i =0 ; i < 7 ; i++ )s.addSlider() ;

	s.sliders[0].attachToVariable(&iso.a1,-(PI/2.0),(PI/2.0));
	s.sliders[1].attachToVariable(&iso.a2,-(PI/2.0),(PI/2.0));
	s.sliders[2].attachToVariable(&iso.a3,-(PI/2.0),(PI/2.0));
	s.sliders[3].attachToVariable(&iso.a4,-(PI/2.0),(PI/2.0));
	s.sliders[4].attachToVariable(&iso.a5,-(PI/2.0),(PI/2.0));
	s.sliders[5].attachToVariable(&iso.a6,-(PI/2.0),(PI/2.0));

	//s.sliders[6].attachToVariable(&iso.fTargetValue,-(PI/2.0),(PI/2.0));

}
void update( int value )
{
	if(run) fps = runStreams();

	cen = (iso.b_max + iso.b_min) * 0.5;
	cen *= -1;
}


void draw()	
{

	backGround(1.0) ;
	//drawGrid(20.0);
	
	// ------------- draw sliders ;
	s.draw() ;


	// ------------- draw HUD text
	char s[200];
	setup2d();


		sprintf(s, "compute time in ms : %1.2f  RES : %i ", fps, iso.iDataSetSize );
		drawString(s, winW * 0.5, 50);
		sprintf(s, " number of pts : %d , num of distance calcs (NN) : %d million ", n, int(float( n * n) * 1.0 / pow(10,6)) );
		drawString(s, winW * 0.35, 75);

	restore3d();

	// ------------- draw isoSurface ;
	
	start_timing();

		glPushMatrix();
	
		glScalef(10, 10, 10);
		glTranslatef(cen.x, cen.y, cen.z);
	
			if(!streams)iso.display( true , true ) ;

		// ------------- draw CUDA 
	
	

			if(streams)
			{
				// ---------------------------------------------------------------- map device pointer to vbo , fill vbo from device , upmap ;
				cud::float4 *dPosPtr;
				//cud::float4 *dColPtr;

				cud::cudaGraphicsMapResources(1, &posVbo_res, 0);
				size_t num_bytes_pos;
				cud::cudaGraphicsResourceGetMappedPointer((void **)&dPosPtr, &num_bytes_pos, posVbo_res);

				fillVbo(dPosPtr);

				cud::cudaGraphicsUnmapResources(1, &posVbo_res, 0);
				//cud::cudaGraphicsUnmapResources(1, &colVbo_res, 0);

				// ---------------------------------------------------------------- draw ;

				glPointSize(1);

				glEnableClientState(GL_VERTEX_ARRAY);
				glEnableClientState(GL_COLOR_ARRAY);
				// pos vbo , updated via device code;
				glBindBufferARB(GL_ARRAY_BUFFER_ARB, posVbo );
				glVertexPointer(4, GL_FLOAT, 0, 0);
				// col vbo , currently using host data ;
				glBindBufferARB(GL_ARRAY_BUFFER_ARB, colVbo );
				glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, MAX_DEVICE_ARRAY_SIZE * 4 * sizeof(float), col);
				glColorPointer(4, GL_FLOAT, 0, 0);

				glDrawArrays(GL_POINTS, 0 , MAX_DEVICE_ARRAY_SIZE );	

				glDisableClientState(GL_VERTEX_ARRAY);
				glDisableClientState(GL_COLOR_ARRAY);
			}

	fps = stop_timing();

	glPopMatrix();

	//////////////////////////////////////////////////////////////////////////

}

void keyPress (unsigned char k, int xm, int ym)
{
	if( k == 'r')run = !run ;
	if( k == 'R')generateHostDataAndCopyToDevice();


	if( k == '=' ) // increase threshold
		iso.fTargetValue *=1.1 ;

	if( k == '-' )// decrease threshold
		iso.fTargetValue /=1.1 ;

	cout << iso.fTargetValue << endl ;

	if( k == 'D') // increase density
		iso.increaseDensity();

	if( k == 'd') // increase density
		iso.decreaseDensity();


	if( k == 'w') 
	{
		// write isoSurface as OBJ ;
		iso.writeFile("isoSurf.obj") ;

		// write points from CUDA as text ;

		CUDA_STRUCT *h_data;
		int n ;
		h_data = copyData_DeviceToHost(h_data,n) ;

		ofstream myfile;
		myfile.open ( "streams.txt" , ios::out  );		

		for( int i = 0 ; i < n ; i++ )
		{
			char s[200] ;					
			sprintf( s, "%1.4f %1.4f %1.4f ", h_data[i].p.x , h_data[i].p.y , h_data[i].p.z );

			myfile << s << endl; 

		}


		myfile.close();

	}

	if( k == 'S')
	{
		// ----------------------------- convert isoSurface to points and copy to GPU for manipulation by GPU ;
		// ------ get triangle points from isoSurface
		vec *iso_pts ;

		iso_pts = iso.getTrianglePoints(iso_pts , n );
		
		// format appropriately  for copy to GPU ;
		CUDA_STRUCT *data = new CUDA_STRUCT[n];
		for( int i =0 ; i < n-3 ; i+=3 )
		{
			#define rX  ofRandom( -1,1  ) 
			vec cen =  (iso_pts[i] + iso_pts[i+1] + iso_pts[i+2]) / 3.0 ;
			data[i].p = cud::make_float3(cen.x,cen.y,cen.z) ;
		}
		
		cout << n << endl; 
		if( n < MAX_DEVICE_ARRAY_SIZE)
		{
			copyData_HostToDevice(data, n );
			streams = !streams ;
		}
		delete data ;
		
	}
	
}

void mousePress(int b,int state,int x,int y) 
{
	if( GLUT_LEFT_BUTTON == b && GLUT_DOWN == state && !streams )
	{
		s.performSelection(x,y,HUDSelectOn) ;
		iso.fillCubeValues(0);
	}
}

void mouseMotion(  int x, int y ) 
{
	//if( GLUT_LEFT_BUTTON == b && GLUT_DOWN == state )
	if(!streams)
	{
		s.performSelection(x,y,HUDSelectOn) ;
		long start = GetTickCount();

		iso.fillCubeValues(0);
		//iso.display() ;

		long end = GetTickCount();
		elapsedTime = end - startTime ;
		long time = (end-start) ;
		//if( time < 10 )time = 10 ;
		float fps = 1000.0f / float(time) ;
		cout << time << endl ;
	}
}