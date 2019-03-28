#include <windows.h>			
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <math.h> 
#include <random>
#include <string>
#include <array>

#include "Extension.h"
#include "Camera.h"
#include "TriangleGrid.h"
#include "DCEL.h"
#include "GPR.h"
#include "ModelMatrix.h"
#include "Linear.h"

int height = 600;
int width = 800;
int samplex = 0;
int samplez = 0;
int samplex2 = 0;
int samplez2 = 0;

int i = 0;

int samples = 2;
float px = -3.0;
float pz = -3.0;

POINT g_OldCursorPos;
bool g_enableVerticalSync;
bool g_enableWireframe;

enum DIRECTION {
	DIR_FORWARD = 1,
	DIR_BACKWARD = 2,
	DIR_LEFT = 4,
	DIR_RIGHT = 8,
	DIR_UP = 16,
	DIR_DOWN = 32,

	DIR_FORCE_32BIT = 0x7FFFFFFF
};

GLuint u_projection, u_view, u_model, u_modelView, u_time, positionID, colorID;


GLuint u_projectionMean, u_viewMean, u_modelMean, u_modelViewMean, positionMean, mean;
GLuint u_projectionVariance, u_viewVariance, u_modelVariance, u_modelViewVariance, positionVariance, variance;

GLuint  programMean, programVariance;

Camera *camera;
GPR *gpr;
Linear *linear;
TriangleGrid *triangleGrid;

DCEL *dcel;

std::vector<Triangle*>	g_triangles;
std::vector<Triangle*>	g_trianglesVTK;
std::vector<TriangleGrid*> triangleGrids;

Matrix4f model;
Matrix4f modelView;
ModelMatrix model2;
//prototype funktions
LRESULT CALLBACK winProc(HWND hWnd, UINT message, WPARAM wParma, LPARAM lParam);
void setCursortoMiddle(HWND hwnd);
void processInput(HWND hWnd );

void renderFrame2();

void renderFrame4();

void loadTriangles(const char* filename);

void initApp(HWND hWnd);
void readTextFile(const char *pszFilename, std::string &buffer);
void enableVerticalSync(bool enableVerticalSync);
void enableWireframe(bool enableWireframe);

GLuint loadShaderProgram(GLenum type, const char *pszFilename);
GLuint compileShader(GLenum type, const char *pszSource);
GLuint linkShaders(GLuint vertShader, GLuint fragShader);

GLuint createProgram(const char* vertex, const char* fragment);
void cleanup();
int rand(int lower, int upper);


// the main windows entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	Vector3f camPos{ 0, 5.0f, 0.1f };
	Vector3f xAxis{ 1, 0, 0 };
	Vector3f yAxis{ 0, 1, 0 };
	Vector3f zAxis{ 0, 0, 1 };
	Vector3f target(0.0, 0.0, 0.0);
	Vector3f up(0.0, 1.0, 0.0);

	camera = new Camera(camPos, xAxis, yAxis, zAxis, target, up);

	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen("CON", "w", stdout);
	SetConsoleTitle("Debug console");
	MoveWindow(GetConsoleWindow(), 790, 0, 500, 200, true);

	std::cout << "K : subdivide" << std::endl;
	std::cout << "M : subdivide DCEL" << std::endl;

	WNDCLASSEX		windowClass;		// window class
	HWND			hwnd;				// window handle
	MSG				msg;				// message
	HDC				hdc;				// device context handle

	// fill out the window class structure
	windowClass.cbSize			= sizeof(WNDCLASSEX);
	windowClass.style			= CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc		= winProc;
	windowClass.cbClsExtra		= 0;
	windowClass.cbWndExtra		= 0;
	windowClass.hInstance		= hInstance;
	windowClass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);		// default icon
	windowClass.hCursor			= LoadCursor(NULL, IDC_ARROW);			// default arrow
	windowClass.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);	// white background
	windowClass.lpszMenuName	= NULL;									// no menu
	windowClass.lpszClassName	= "WINDOWCLASS";
	windowClass.hIconSm			= LoadIcon(NULL, IDI_WINLOGO);			// windows logo small icon

	// register the windows class
	if (!RegisterClassEx(&windowClass))
		return 0;

	// class registered, so now create our window
	hwnd = CreateWindowEx(NULL,									// extended style
						  "WINDOWCLASS",						// class name
						  "Cube GlSl",							// app name
						  WS_OVERLAPPEDWINDOW,
						  0, 0,									// x,y coordinate
						  width,
						  height,									// width, height
						  NULL,									// handle to parent
						  NULL,									// handle to menu
						  hInstance,							// application instance
						  NULL);								// no extra params

	// check if window creation failed (hwnd would equal NULL)
	if (!hwnd)
		return 0;

	ShowWindow(hwnd, SW_SHOW);			// display the window
	UpdateWindow(hwnd);					// update the window

	initApp(hwnd);
	
	// main message loop
	while (true) 
    {
        // Did we recieve a message, or are we idling ?
		if ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) {
			// test if this is a quit
			if (msg.message == WM_QUIT) break;
			// translate and dispatch message
			TranslateMessage( &msg );
			DispatchMessage ( &msg );
		} else {
			
			processInput(hwnd);
			renderFrame4();
			hdc=GetDC(hwnd);
			SwapBuffers(hdc);			// bring backbuffer to foreground
			ReleaseDC(hwnd,hdc);
		} // end If messages waiting
    } // end while

	cleanup();
	return msg.wParam;
}

// the Windows Procedure event handler
LRESULT CALLBACK winProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HGLRC hRC;					// rendering context
	static HDC hDC;						// device context
	int width, height;					// window width and height
	POINT pt;
	RECT rect;

	switch(message)
	{
		case WM_DESTROY:{

			PostQuitMessage(0);
			return 0;
		}

		case WM_CREATE:{
			
			GetClientRect(hWnd, &rect);
			g_OldCursorPos.x = rect.right / 2;
			g_OldCursorPos.y = rect.bottom / 2;
			pt.x = rect.right / 2;
			pt.y = rect.bottom / 2;
			SetCursorPos(pt.x, pt.y);

			// set the cursor to the middle of the window and capture the window via "SendMessage"
			SendMessage(hWnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(pt.x, pt.y));
			return 0;
		}break;

		case WM_LBUTTONDOWN:{ // Capture the mouse

			setCursortoMiddle(hWnd);
			SetCapture(hWnd);
			
			return 0;
		} break;

		case WM_KEYDOWN:{

			switch (wParam){

				case VK_ESCAPE:{

					PostQuitMessage(0);
					return 0;

				}break;
				case VK_SPACE: {

					ReleaseCapture();
					return 0;

				}break;
				case 'v': case 'V':{

					enableVerticalSync(!g_enableVerticalSync);
				    return 0;
					
				}break;
				case 'f': case 'F':{

					
					
					return 0;

				}break;
				case 'g': case 'G':{

					
					
					return 0;

				}break;
				
				case 'j': case 'J':{
							 
							  return 0;

				}break;
				case 'n': case 'N':{

							
				}break;
				case 'm': case 'M':{

							  dcel->calcSplitPoints();
							  dcel->globalMean = 0;
							  dcel->globalBha = 0;
							  dcel->calcFaceErrors();
							  std::cout << "Global Mean: " << dcel->globalMean << std::endl;
							  std::cout << "Global BHA: " << dcel->globalBha << std::endl;
							  std::cout << "Face number: " << dcel->m_face->m_id << "  BHA: " << dcel->bha << "  Mean: " << dcel->mean << std::endl;
							  dcel->split();
							  dcel->addFacesToBuffer();
							 
				}break;
				case 'k': case 'K':{

							  dcel->_split();
							  dcel->addFacesToBuffer();
							  			
				}break;
				case 'l': case 'L':{
							
							  dcel->_splitFan();
							  dcel->addFacesToBuffer();
				}break;
				case 'o': case 'O':{

							  dcel->calcGlobalError();
							  std::cout << "Global Mean: " << dcel->globalMean << std::endl;
							  std::cout << "Global BHA: " << dcel->globalBha << std::endl;
							  std::cout << "BHA: " << dcel->bha << "  Mean: " << dcel->mean << std::endl;

				}break;
				case 'p': case 'P':{

							 
				}break;
				case 'z': case 'Z':{

							  enableWireframe(!g_enableWireframe);

				}break;
				return 0;
			}break;

			return 0;
		}break;
		case WM_SIZE:{

			int height2 = HIWORD(lParam);		// retrieve width and height
		    int width2 = LOWORD(lParam);
			
			if (height2 == 0){					// avoid divide by zero
				
				height2 = 1;
			}

			glViewport(0, 0, width2, height2);
			return 0;

		}break;
		default:
		 break;
    }
	return (DefWindowProc(hWnd, message, wParam, lParam));
}

void initApp(HWND hWnd)
{
	static HGLRC hRC;					// rendering context
	static HDC hDC;						// device context

	hDC = GetDC(hWnd);
	int nPixelFormat;					// our pixel format index

	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),	// size of structure
		1,								// default version
		PFD_DRAW_TO_WINDOW |			// window drawing support
		PFD_SUPPORT_OPENGL |			// OpenGL support
		PFD_DOUBLEBUFFER,				// double buffering support
		PFD_TYPE_RGBA,					// RGBA color mode
		32,								// 32 bit color mode
		0, 0, 0, 0, 0, 0,				// ignore color bits, non-palettized mode
		0,								// no alpha buffer
		0,								// ignore shift bit
		0,								// no accumulation buffer
		0, 0, 0, 0,						// ignore accumulation bits
		16,								// 16 bit z-buffer size
		0,								// no stencil buffer
		0,								// no auxiliary buffer
		PFD_MAIN_PLANE,					// main drawing plane
		0,								// reserved
		0, 0, 0 };						// layer masks ignored

	nPixelFormat = ChoosePixelFormat(hDC, &pfd);	// choose best matching pixel format
	SetPixelFormat(hDC, nPixelFormat, &pfd);		// set pixel format to device context


	// create rendering context and make it current
	hRC = wglCreateContext(hDC);
	wglMakeCurrent(hDC, hRC);
	enableVerticalSync(true);
	enableWireframe(false);

	glMatrixMode(GL_PROJECTION);										// set projection matrix current matrix
	glLoadIdentity();													// reset projection matrix
	camera->perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.001f, 8000.0f);

	glLoadMatrixf(&camera->getProjectionMatrix()[0][0]);

	glClearColor(1.0f, 1.0f, 0.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);					// hidden surface removal
	//glEnable(GL_CULL_FACE);						// do not calculate inside of poly's

	

	

	programMean = createProgram("mean.vert", "mean.frag");



	u_projectionMean = glGetUniformLocation(programMean, "u_projection");
	u_viewMean = glGetUniformLocation(programMean, "u_view");
	u_modelMean = glGetUniformLocation(programMean, "u_model");
	u_modelViewMean = glGetUniformLocation(programMean, "u_modelView");
	positionMean = glGetAttribLocation(programMean, "position");
	mean = glGetAttribLocation(programMean, "mean");
	glUseProgram(programMean);
	glUniformMatrix4fv(u_projectionMean, 1, false, &camera->getProjectionMatrix()[0][0]);
	glUseProgram(0);

	programVariance = createProgram("variance.vert", "variance.frag");



	u_projectionVariance = glGetUniformLocation(programVariance, "u_projection");
	u_viewVariance = glGetUniformLocation(programVariance, "u_view");
	u_modelVariance = glGetUniformLocation(programVariance, "u_model");
	u_modelViewVariance = glGetUniformLocation(programVariance, "u_modelView");
	positionVariance = glGetAttribLocation(programVariance, "position");
	variance = glGetAttribLocation(programVariance, "variance");
	glUseProgram(programVariance);
	glUniformMatrix4fv(u_projectionVariance, 1, false, &camera->getProjectionMatrix()[0][0]);
	glUseProgram(0);


	/*std::vector<std::tuple<std::vector<double>, double, double>> trainData = {
		std::tuple<std::vector<double>, double, double>({ 0.0, 0.0 }, 1.0, 1.0),
		std::tuple<std::vector<double>, double, double>({ 1.0, 0.0 }, 1.0, 1.0),
		std::tuple<std::vector<double>, double, double>({ 1.0, -1.0 }, -1.0, 1.0),
		std::tuple<std::vector<double>, double, double>({ 0.0, -1.0 }, 0.0, 1.0)
	};*/

	std::vector<std::tuple<std::vector<double>, double, double>> trainData2 = {
		std::tuple<std::vector<double>, double, double>({ -22.5, -35.1 }, 102581, 677.042),
		std::tuple<std::vector<double>, double, double>({ -20.7, -35.1 }, 102633, 616.653),
		std::tuple<std::vector<double>, double, double>({ -22.5, -33.3 }, 102737, 579.094),
		std::tuple<std::vector<double>, double, double>({ -20.7, -33.3 }, 102778, 543.589),
		std::tuple<std::vector<double>, double, double>({ -18.9, -33.3 }, 102790, 536.816),
		std::tuple<std::vector<double>, double, double>({ -22.5, -31.5 }, 102827, 497.406),
		std::tuple<std::vector<double>, double, double>({ -20.7, -31.5 }, 102849, 484.304),
		std::tuple<std::vector<double>, double, double>({ -18.9, -31.5 }, 102849, 490.87),
		std::tuple<std::vector<double>, double, double>({ -22.5, -29.7 }, 102863, 432.817),
		std::tuple<std::vector<double>, double, double>({ -20.7, -29.7 }, 102865, 433.337),
		std::tuple<std::vector<double>, double, double>({ -18.9, -29.7 }, 102853, 441.038),
		std::tuple<std::vector<double>, double, double>({ -20.7, -27.9 }, 102840, 379.169),
		std::tuple<std::vector<double>, double, double>({ -18.9, -27.9 }, 102829, 383.42)
	};

	
	std::vector<Triangle*> triangles;

	/*triangles.push_back(new Triangle(Vector3f(0.0, 0.0, -1.0), Vector3f(1.0, 0.0, -1.0), Vector3f(1.0, 0.0, 0.0),
		0.0, -1.0, 1.0,
		1.0, 1.0, 1.0, true));*/

	triangles.push_back(new Triangle(Vector3f(0.0, 0.0, -1.0), Vector3f(1.0, 0.0, 0.0), Vector3f(0.0, 0.0, 0.0),
		0.0, 1.0, 1.0,
		1.0, 1.0, 1.0, true));

	//2649.32
	triangleGrid = new TriangleGrid(new Linear(triangles), new GPR(trainData2, 2649.32, 0.7), 0.000001);
	
	//loadTriangles("grid4.txt");
	
	dcel = new DCEL(triangleGrid);

	dcel->buildDCEL("grid1.vtk");
	//dcel->buildDCEL(g_triangles);
	dcel->setGPR(3, 1.0, 0.2);
	
	dcel->addFacesToBuffer();

	//triangleGrid->addTriangles(g_triangles);
	triangleGrids.push_back(triangleGrid);
}
	

void setCursortoMiddle(HWND hwnd){

	RECT rect;

	GetClientRect(hwnd, &rect);
	SetCursorPos(rect.right / 2, rect.bottom / 2);

}
void enableVerticalSync(bool enableVerticalSync){

	// WGL_EXT_swap_control.

	typedef BOOL(WINAPI * PFNWGLSWAPINTERVALEXTPROC)(GLint);

	static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT =
		reinterpret_cast<PFNWGLSWAPINTERVALEXTPROC>(
		wglGetProcAddress("wglSwapIntervalEXT"));

	if (wglSwapIntervalEXT)
	{
		wglSwapIntervalEXT(enableVerticalSync ? 1 : 0);
		g_enableVerticalSync = enableVerticalSync;
	}
}

void enableWireframe(bool enableWireframe){

	g_enableWireframe = enableWireframe;

	if (g_enableWireframe){

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	}
	else{

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	}

}

void readTextFile(const char *pszFilename, std::string &buffer){

	std::ifstream file(pszFilename, std::ios::binary);

	if (file.is_open())
	{
		file.seekg(0, std::ios::end);

		std::ifstream::pos_type fileSize = file.tellg();

		buffer.resize(static_cast<unsigned int>(fileSize));
		file.seekg(0, std::ios::beg);
		file.read(&buffer[0], fileSize);
	}
}

GLuint loadShaderProgram(GLenum type, const char *pszFilename){

	GLuint shader = 0;
	std::string buffer;
	readTextFile(pszFilename, buffer);

	if (buffer.length() > 0)
	{


		shader = compileShader(type, reinterpret_cast<const char *>(&buffer[0]));
	}

	return shader;
}

GLuint compileShader(GLenum type, const char *pszSource){

	GLuint shader = glCreateShader(type);

	//std::cout << pszSource << std::endl;

	if (shader)
	{
		GLint compiled = 0;

		glShaderSource(shader, 1, &pszSource, NULL);
		glCompileShader(shader);
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

		if (!compiled)
		{
			GLsizei infoLogSize = 0;
			std::string infoLog;

			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogSize);
			infoLog.resize(infoLogSize);
			glGetShaderInfoLog(shader, infoLogSize, &infoLogSize, &infoLog[0]);
			std::cout << "Compile status: \n" << &infoLog << std::endl;

		}

	}
	return shader;
}

GLuint linkShaders(GLuint vertShader, GLuint fragShader){

	GLuint program = glCreateProgram();

	if (program)
	{
		GLint linked = 0;

		if (vertShader)
			glAttachShader(program, vertShader);

		if (fragShader)
			glAttachShader(program, fragShader);

		glLinkProgram(program);

		glGetProgramiv(program, GL_LINK_STATUS, &linked);

		if (!linked)
		{
			GLsizei infoLogSize = 0;
			std::string infoLog;

			glGetShaderiv(program, GL_INFO_LOG_LENGTH, &infoLogSize);
			infoLog.resize(infoLogSize);
			glGetShaderInfoLog(program, infoLogSize, &infoLogSize, &infoLog[0]);
			std::cout << "Compile status: \n" << &infoLog << std::endl;
		}

		// Mark the two attached shaders for deletion. These two shaders aren't
		// deleted right now because both are already attached to a shader
		// program. When the shader program is deleted these two shaders will
		// be automatically detached and deleted.

		if (vertShader)
			glDeleteShader(vertShader);

		if (fragShader)
			glDeleteShader(fragShader);

	}

	return program;
}

GLuint createProgram(const char* vertex, const char* fragment) {


	GLuint vshader = loadShaderProgram(GL_VERTEX_SHADER, vertex);
	GLuint fshader = loadShaderProgram(GL_FRAGMENT_SHADER, fragment);

	return linkShaders(vshader, fshader);

}

void cleanup(){

	
	if (programMean){

		glDeleteProgram(programMean);
		programMean = 0;
	}

	if (programVariance){

		glDeleteProgram(programVariance);
		programVariance = 0;
	}
}

int rand(int lower, int upper){

	return (rand() % (upper - lower + 1)) + lower;

}


void processInput(HWND hWnd )
{
    static UCHAR pKeyBuffer[ 256 ];
    ULONG        Direction = 0;
    POINT        CursorPos;
    float        X = 0.0f, Y = 0.0f;

    // Retrieve keyboard state
    if ( !GetKeyboardState( pKeyBuffer ) ) return;

	// Check the relevant keys
    if ( pKeyBuffer['W'] & 0xF0 ) Direction |= DIR_FORWARD;
    if ( pKeyBuffer['S'] & 0xF0 ) Direction |= DIR_BACKWARD;
    if ( pKeyBuffer['A'] & 0xF0 ) Direction |= DIR_LEFT;
    if ( pKeyBuffer['D'] & 0xF0 ) Direction |= DIR_RIGHT;
    if ( pKeyBuffer['E'] & 0xF0 ) Direction |= DIR_UP;
    if ( pKeyBuffer['Q'] & 0xF0 ) Direction |= DIR_DOWN;

	 // Now process the mouse (if the button is pressed)
    if ( GetCapture() == hWnd )
    {
        // Hide the mouse pointer
        SetCursor( NULL );

        // Retrieve the cursor position
        GetCursorPos( &CursorPos );

        // Calculate mouse rotational values
		X = (float)(g_OldCursorPos.x - CursorPos.x) * 0.1;
		Y = (float)(g_OldCursorPos.y - CursorPos.y) * 0.1;

        // Reset our cursor position so we can keep going forever :)
        SetCursorPos( g_OldCursorPos.x, g_OldCursorPos.y );

		if ( Direction > 0 || X != 0.0f || Y != 0.0f )
		{
			// Rotate camera
			if ( X || Y ) 
			{
				camera->rotate( X, Y, 0.0f );
            
        
			} // End if any rotation


			if ( Direction ) 
			{
				float dx = 0, dy = 0, dz = 0, speed = 0.2;

				if (Direction & DIR_FORWARD) dz = speed;
				if (Direction & DIR_BACKWARD) dz = -speed;
				if (Direction & DIR_LEFT) dx = -speed;
				if (Direction & DIR_RIGHT) dx = speed;
				if (Direction & DIR_UP) dy = speed;
				if (Direction & DIR_DOWN) dy = -speed;

				camera->move(dx,dy,dz);

			} 
	
		}// End if any movement
	} // End if Captured
}




void renderFrame4(){

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glUseProgram(programMean);
	glUniformMatrix4fv(u_viewMean, 1, false, &camera->getViewMatrix()[0][0]);

	model.translate(-2.0, 0.0, -1.0);
	glUniformMatrix4fv(u_modelMean, 1, true, &model[0][0]);

	glEnableVertexAttribArray(positionMean);
	glBindBuffer(GL_ARRAY_BUFFER, triangleGrids[0]->m_vertex);

	glVertexAttribPointer(
		positionMean,		// attribute
		3,                  // number of elements per vertex, here (x,y)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		0,                  // no extra data between each position
		0                   // offset of first element
		);

	glEnableVertexAttribArray(mean);
	glBindBuffer(GL_ARRAY_BUFFER, triangleGrids[0]->m_meanGPR);
	glVertexAttribPointer(
		mean,				 // attribute
		1,                   // number of elements per vertex, here (r,g,b)
		GL_FLOAT,            // the type of each element
		GL_FALSE,            // take our values as-is
		0,					 // stride
		0					 // offset
		);


	glDrawArrays(GL_TRIANGLES, 0, triangleGrids[0]->m_vertexBuffer.size() / 3);

	glDisableVertexAttribArray(mean);
	glDisableVertexAttribArray(positionMean);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	glUseProgram(programMean);
	glUniformMatrix4fv(u_viewMean, 1, false, &camera->getViewMatrix()[0][0]);

	model.translate(-0.5, 0.0, -1.0);
	glUniformMatrix4fv(u_modelMean, 1, true, &model[0][0]);

	glEnableVertexAttribArray(positionMean);
	glBindBuffer(GL_ARRAY_BUFFER, triangleGrids[0]->m_vertex);
	glVertexAttribPointer(
		positionMean,			// attribute
		3,                  // number of elements per vertex, here (x,y)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		0,                  // no extra data between each position
		0                   // offset of first element
		);

	glEnableVertexAttribArray(mean);
	glBindBuffer(GL_ARRAY_BUFFER, triangleGrids[0]->m_meanHybrid);
	glVertexAttribPointer(
		mean,				 // attribute
		1,                   // number of elements per vertex, here (r,g,b)
		GL_FLOAT,            // the type of each element
		GL_FALSE,            // take our values as-is
		0,					 // stride
		0					 // offset
		);

	glDrawArrays(GL_TRIANGLES, 0, triangleGrids[0]->m_vertexBuffer.size() / 3);


	glDisableVertexAttribArray(positionMean);
	glDisableVertexAttribArray(mean);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(programVariance);
	glUniformMatrix4fv(u_viewVariance, 1, false, &camera->getViewMatrix()[0][0]);

	model.translate(-2.0, 0.0, 0.5);
	glUniformMatrix4fv(u_modelVariance, 1, true, &model[0][0]);

	glEnableVertexAttribArray(positionVariance);
	glBindBuffer(GL_ARRAY_BUFFER, triangleGrids[0]->m_vertex);
	glVertexAttribPointer(
		positionVariance,	// attribute
		3,                  // number of elements per vertex, here (x,y)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		0,                  // no extra data between each position
		0                   // offset of first element
		);

	glEnableVertexAttribArray(variance);
	glBindBuffer(GL_ARRAY_BUFFER, triangleGrids[0]->m_varianceGPR);
	glVertexAttribPointer(
		variance,			 // attribute
		1,                   // number of elements per vertex, here (r,g,b)
		GL_FLOAT,            // the type of each element
		GL_FALSE,            // take our values as-is
		0,					 // stride
		0					 // offset
		);

	glDrawArrays(GL_TRIANGLES, 0, triangleGrids[0]->m_vertexBuffer.size() / 3);

	glDisableVertexAttribArray(variance);
	glDisableVertexAttribArray(positionVariance);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	glUseProgram(programVariance);
	glUniformMatrix4fv(u_viewVariance, 1, false, &camera->getViewMatrix()[0][0]);

	model.translate(-0.5, 0.0, 0.5);
	glUniformMatrix4fv(u_modelVariance, 1, true, &model[0][0]);


	glEnableVertexAttribArray(positionVariance);
	glBindBuffer(GL_ARRAY_BUFFER, triangleGrids[0]->m_vertex);
	glVertexAttribPointer(
		positionVariance,			// attribute
		3,                  // number of elements per vertex, here (x,y)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		0,                  // no extra data between each position
		0                   // offset of first element
		);

	glEnableVertexAttribArray(variance);
	glBindBuffer(GL_ARRAY_BUFFER, triangleGrids[0]->m_varianceHybrid);
	glVertexAttribPointer(
		variance,			 // attribute
		1,                   // number of elements per vertex, here (r,g,b)
		GL_FLOAT,            // the type of each element
		GL_FALSE,            // take our values as-is
		0,					 // stride
		0					 // offset
		);

	glDrawArrays(GL_TRIANGLES, 0, triangleGrids[0]->m_vertexBuffer.size() / 3);

	glDisableVertexAttribArray(variance);
	glDisableVertexAttribArray(positionVariance);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

void renderFrame2(){

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&camera->getViewMatrix()[0][0]);

	glTranslatef(-2.0, 0.0, -1.0);
	glBegin(GL_TRIANGLES);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, -1.0);
	glVertex3f(1.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glEnd();
	
	glTranslatef(0.0, 0.0, 1.5);
	glPointSize(3.0f);

	glBegin(GL_POINTS); 
	glVertex3f(0.0, 0.0, -1.0);
	glVertex3f(1.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glEnd();
	

	std::random_device rd1;
	std::default_random_engine generator1(rd1());
	std::uniform_real_distribution<> dis1(0.0, 1.0);

	std::random_device rd2;
	std::default_random_engine generator2(rd2());
	std::uniform_real_distribution<> dis2(0.0, 1.0);

	glColor3f(0.0, 1.0, 0.0);
	glPointSize(1.0f);
	glBegin(GL_POINTS);
	for (int i = 0; i < 1000; i++){

		double r1 = dis1(generator1);
		double r2 = dis2(generator2);

		Vector3f sample = Vector3f(0.0, 0.0, -1.0) * (1 - sqrt(r1)) + Vector3f(1.0, 0.0, 0.0) * sqrt(r1) * (1 - r2) + Vector3f(0.0, 0.0, 0.0)* sqrt(r1) *r2;

		glVertex3f(sample[0], 0.0, sample[2]);
	}
	glEnd();
}


void loadTriangles(const char* filename){

	char buffer[250];

	FILE * pFile = fopen(filename, "r");
	if (pFile == NULL){
		std::cout << "File not found" << std::endl;
		return;
	}


	while (fscanf(pFile, "%s", buffer) != EOF){
	
		switch (buffer[0]){

				case 'f': {

					  fgets(buffer, sizeof(buffer), pFile);
					  float a0, a2, b0, b2, c0, c2;
					  int subdivide;
					  sscanf(buffer, "%f %f %f %f %f %f %d", &a0, &a2, &b0, &b2, &c0, &c2, &subdivide);

					  g_triangles.push_back(new Triangle(Vector3f(a0, 0.0, a2), Vector3f(b0, 0.0, b2), Vector3f(c0, 0.0, c2), subdivide));
				}
		}
	}

}

