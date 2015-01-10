// 159.240 Particle Visualiser
//
// This is a hint for assignment 1. It does the x-coordinate correctly, but not the y-coordinate.
// To get full marks, you have to:
// 1. Update the y-coordinate the same way
// 2. Implement your own random number generator and use it instead of rand().

#include <windows.h>   
#include <stdlib.h>
#include <time.h>
#include <string>
#include <math.h>

using namespace std;

void CleanUp();
bool UpdateAgents();
void RenderScene(HDC surface);
int RandInt(int x,int y);
double RandFloat();

// Some global variables
bool g_fastRender = false;
int g_windowWidth = 640;
int g_windowHeight = 480;


// How many agents do we want?
const int g_agentCount = 80;

// This is our definition of an agent, or particle
struct Agent {
	int x,y;
};

// This is the variable that will hold all our agents.
Agent g_agents[g_agentCount];

/////////////////////////////////////////////////////////////////
// You have to put your own random number generator in here - 
// it can be an LCG, or any other random number generator.
// You then need to replace rand() with myrand() and make sure 
// it works.
unsigned long long previousRand;
unsigned long long myrand() {
	return 0; // change me
}

// This function gets called automatically all the time. It has to 
// update all the circles one after the other, and move them around.
// At the moment, this generates exponential deviates, and applies
// them to the X coordinates only. You need to update it to change the Y
// coordinates too.

// Remember that the Y-coordinate runs down from the top of the screen.
bool UpdateAgents() {
	for (int i=0; i < g_agentCount; ++i) { 				// loop through all the circles
		
		float left = rand() / (RAND_MAX + 1.0f);			// make a uniform random float
		
		// you need to make a uniform random float between (0,1]
		// so that you can pass it through the formula -ln(a)/0.3
		
		float a = rand() / (RAND_MAX + 1.0f);				// make another uniform random float
		
		float exponential_deviate = -log(a) / 0.3f;		// turn one into an exponential deviate
		
		// At this point, you have an exponential deviate, but it will always be
		// positive. You need to randomise the direction (left or right, up or down).		
		
		if (left < 0.5f) { // This test will be true 50% of the time
			exponential_deviate = exponential_deviate * -1.0f;
			// multiply by -1.0f to turn it around.
		}
		
		// Finally, add the exponential deviate to the X coordinate of all circles.
		// Notice the round() function use. The array of agent structs has coordinates
		// stored as ints, so you need to round the floats, otherwise they will be 
		// truncated.
		g_agents[i].x += round(exponential_deviate);
		
		// This line might give you the warning "converting to `int' from `double'. This is ok.
		
	}
	
	
	return true;
}
/////////////////////////////////////////////////////////////////




char* szApplicationName = "Agent Viewer";
char* szWindowClassName = "agents";

// In order to draw on a Win32 window, we need some "Pens" to draw with.
HPEN				g_redPen;
HPEN				g_bluePen;
HPEN				g_greenPen;
HPEN				g_oldPen;



// This is the famous WndProc function which handles the Windows message loop.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

//	Entry point for our windows application. This is the equivalent of int main()
int WINAPI WinMain(	HINSTANCE hinstance, HINSTANCE hprevinstance, LPSTR lpcmdline, int ncmdshow);

// This function draws on the surface of the window.
void RenderScene(HDC surface) {
	//render the stats
	//string s = "Generation:          123";
	//TextOut(surface, 5, 0, s.c_str(), s.size());

	//do not render if running at accelerated speed
	if (!g_fastRender) {
		//keep a record of the old pen
		g_oldPen = (HPEN)SelectObject(surface, g_greenPen);
		
		for (int i = 0; i < g_agentCount; i++) {
				
			Ellipse(surface, g_agents[i].x, g_agents[i].y, g_agents[i].x+10,g_agents[i].y+10);

	    	//put the old pen back
		    SelectObject(surface, g_oldPen);

		}
  	}
}


int RandInt(int x,int y) { 
	return rand()%(y-x+1)+x; 
}
double RandFloat() { 
	return (rand())/(RAND_MAX+1.0); 
}

// This is the famous WndProc function which handles the Windows message loop.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	//these hold the dimensions of the client window area
	static int cxClient, cyClient;

	//used to create the back buffer
	static HDC		  hdcBackBuffer;
	static HBITMAP	hBitmap;
	static HBITMAP	hOldBitmap; 

	HDC hdc;
	switch(msg) {	
		case WM_CREATE: 
		{
			//seed the random number generator
			srand((unsigned) time(NULL));

			//get the size of the client window
			RECT rect;
			GetClientRect(hwnd, &rect);

			cxClient = rect.right;
			cyClient = rect.bottom;

				//create a surface for us to render to(backbuffer)
			hdcBackBuffer = CreateCompatibleDC(NULL);

			hdc = GetDC(hwnd);

			hBitmap = CreateCompatibleBitmap(hdc,cxClient,cyClient);
			ReleaseDC(hwnd, hdc);

			hOldBitmap = (HBITMAP)SelectObject(hdcBackBuffer, hBitmap); 
		} 
			
		break;
		
		//check key press messages
		case WM_KEYUP:
			{
				switch(wparam) {
					case VK_ESCAPE:
						PostQuitMessage(0);
						break;
					case 'F':
						g_fastRender = !g_fastRender;
						break;
					case 'R':
						break;
				}
			}
			break;

		//has the user resized the client area?
		case WM_SIZE:
			cxClient = LOWORD(lparam);
			cyClient = HIWORD(lparam);

			//resize the backbuffer accordingly
			SelectObject(hdcBackBuffer, hOldBitmap);

			hdc = GetDC(hwnd);

			hBitmap = CreateCompatibleBitmap(hdc,cxClient,cyClient);
			ReleaseDC(hwnd, hdc);
			
			hOldBitmap = (HBITMAP)SelectObject(hdcBackBuffer, hBitmap); 
			break;

		case WM_PAINT: 
	  		PAINTSTRUCT ps;
	  
			BeginPaint(hwnd, &ps);
		
			//fill our backbuffer with white
			BitBlt(hdcBackBuffer, 0, 0, cxClient, cyClient, NULL, 0, 0, WHITENESS);

			//render the mines and sweepers
			RenderScene(hdcBackBuffer);
			
			//now blit backbuffer to front
			BitBlt(ps.hdc, 0, 0, cxClient, cyClient, hdcBackBuffer, 0, 0, SRCCOPY); 

			EndPaint(hwnd, &ps);
			break;

		case WM_DESTROY: 
			SelectObject(hdcBackBuffer, hOldBitmap);
			
			//clean up our backbuffer objects
			DeleteDC(hdcBackBuffer);
			DeleteObject(hBitmap); 

			// kill the application, this sends a WM_QUIT message 
			PostQuitMessage(0);
			break;

		default:
			break;

	}


	// default msg handler 
	return (DefWindowProc(hwnd, msg, wparam, lparam));
}


int WINAPI WinMain(	HINSTANCE hinstance, HINSTANCE hprevinstance, LPSTR lpcmdline, int ncmdshow) {

	WNDCLASSEX winclass; 
	HWND	   hwnd;	 
	MSG		   msg;		 

	srand(time(NULL));

	//create a pen for the graph drawing
	g_bluePen  = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
	g_redPen   = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	g_greenPen = CreatePen(PS_SOLID, 1, RGB(0, 150, 0));

	for (int i = 0; i < g_agentCount; ++i) {
		g_agents[i].x = (int)(RandFloat() * g_windowWidth);
		g_agents[i].y = (int)(RandFloat() * g_windowHeight);
	}

	// first fill in the window class stucture
	winclass.cbSize       = sizeof(WNDCLASSEX);
	winclass.style			  = CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc	= WindowProc;
	winclass.cbClsExtra		= 0;
	winclass.cbWndExtra		= 0;
	winclass.hInstance		= hinstance;
	winclass.hIcon = NULL;
	//winclass.hIcon			  = LoadIcon(hinstance, MAKEINTRESOURCE(IDI_ICON1));
	winclass.hCursor		  = LoadCursor(NULL, IDC_ARROW); 
	winclass.hbrBackground= NULL; 
	winclass.lpszMenuName	= NULL;
	winclass.lpszClassName= szWindowClassName;
	winclass.hIconSm = NULL;
	//winclass.hIconSm      = LoadIcon(hinstance, MAKEINTRESOURCE(IDI_ICON1));


	// register the window class
	if (!RegisterClassEx(&winclass)) {
		MessageBox(NULL, "Error Registering Class!", "Error", 0);
		return 0;
	}

	// create the window (one that cannot be resized)
	if (!(hwnd = CreateWindowEx(0,									
								szWindowClassName,						
								szApplicationName,						
								WS_OVERLAPPED | WS_VISIBLE | WS_CAPTION | WS_SYSMENU,
								GetSystemMetrics(SM_CXSCREEN)/2 - g_windowWidth/2,
								GetSystemMetrics(SM_CYSCREEN)/2 - g_windowHeight/2,
								g_windowWidth,
								g_windowHeight,				
								NULL,									
								NULL,								
								hinstance,								
								NULL)))	{
		MessageBox(NULL, "Error Creating Window!", "Error", 0);
		return 0;
	}
	
	//Show the window
	ShowWindow(hwnd, SW_SHOWDEFAULT );
	UpdateWindow(hwnd);


	// Setup a timer.
	LONGLONG perfCountFreq = 0, currentTime = 0, nextTime = 0, frameTime = 0, lastTime = 0;
	double timeElapsed = 0.0, timeScale = 0.0;

	QueryPerformanceFrequency((LARGE_INTEGER*) &perfCountFreq);
	timeScale = 1.0f/perfCountFreq;
	frameTime = (LONGLONG)(perfCountFreq / 60); // 60 FPS


	// Enter the message loop
	bool bDone = FALSE;

	while(!bDone) {
		while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) {
			if( msg.message == WM_QUIT ) {
				//Stop loop if it's a quit message
				bDone = TRUE;
			} else {
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}
					
		QueryPerformanceCounter( (LARGE_INTEGER*) &currentTime);
		if (currentTime > nextTime || g_fastRender) {

			if (!UpdateAgents()) {
				bDone = TRUE;
			}

			if (currentTime > nextTime) {
				timeElapsed = (currentTime - lastTime) * timeScale;
				lastTime = currentTime;
				nextTime = currentTime + frameTime;
			}

			//this will call WM_PAINT which will render our scene
			InvalidateRect(hwnd, NULL, TRUE);
			UpdateWindow(hwnd);
		}


					
	}//end while
	

	// Clean up everything and exit the app
	CleanUp();
	UnregisterClass( szWindowClassName, winclass.hInstance );
	
	return 0;

}
void CleanUp() {
	DeleteObject(g_bluePen);
	DeleteObject(g_redPen);
	DeleteObject(g_greenPen);
	DeleteObject(g_oldPen);
}
