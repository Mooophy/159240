#include <windows.h>   
#include <vector>

#include <string>

using namespace std;



struct vector2f {
	float x,y;
};

const int modelVertexCount = 4;
vector2f model[modelVertexCount];



void CleanUp();
bool UpdateAgents();
void RenderScene(HDC surface);
int RandInt(int x,int y);
double RandFloat();


// Some global variables
bool g_fastRender = false;






struct Bullet {
	float x,y;
	float vx,vy;
	Ship* origin_ship;
	bool destroyed;
};

struct Explosion {
	float x,y;
	int step; // step in explosion
};



void FireBullet(float x, float y, float vx, float vy, Ship* fromship);
void CheckBullets();

void MakeExplosion(float x, float y);

// This is the variable that will hold all our agents.
vector<Ship*> g_agents; 
vector<Bullet*> g_bullets;
vector<Explosion*> g_explosions;

bool OutOfBounds(Ship *s) {
	return s->x < 0 || s->x > g_windowWidth ||
		s->y < 0 || s->y > g_windowHeight;
}

void DeleteShip(vector<Ship*>::iterator &it) {
	for (vector<Ship*>::iterator nit = g_agents.begin(); nit != g_agents.end(); nit ++) {
		if ((*nit)->target == *it)
			(*nit)->target = NULL;
	}
	
	g_ships_destroyed ++;
	
	delete *it;
	g_agents.erase(it);
}

void CleanUpSprites() {
	for (vector<Bullet*>::iterator it = g_bullets.begin(); it != g_bullets.end() ; ) {
		if ((*it)->destroyed) {
			
			delete *it;
			g_bullets.erase(it);
			
		} else {
			it ++;
		}
	}
	
	for (vector<Ship*>::iterator it = g_agents.begin(); it != g_agents.end(); ) {
		if ((*it)->destroyed) {
			DeleteShip(it);
			
			
			
		} else if(OutOfBounds(*it)) {
			DeleteShip(it);
		} else {
			it ++;
		}
	}
}




const char* szApplicationName = "Agent Viewer";
const char* szWindowClassName = "agents";

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
	char s[4096];
	sprintf(s, "Ships created: %7d, Ships destroyed: %7d", g_ships_created, g_ships_destroyed);
	
	TextOut(surface, 5, 0, s, strlen(s));

	//do not render if running at accelerated speed
	if (!g_fastRender) {
		//keep a record of the old pen
		g_oldPen = (HPEN)SelectObject(surface, g_greenPen);
		
		float x,y,angle;
		for (unsigned int i = 0; i < g_agents.size(); i++) {
			if (g_agents[i]->team == 0) {
				SelectObject(surface, g_bluePen);
			} else {
				SelectObject(surface, g_redPen);
			}
			//Ellipse(surface, g_agents[i].x, g_agents[i].y, g_agents[i].x+10,g_agents[i].y+10);
			MoveToEx(surface, g_agents[i]->x, g_agents[i]->y, NULL);
			
			angle = atan2(g_agents[i]->vy,g_agents[i]->vx) + M_PI / 2.0f;
			//angle = 2.5f;
			
			for (int j=0; j < modelVertexCount; j++) {
				x = model[j].x * cos(angle) - model[j].y * sin(angle);
				y = model[j].x * sin(angle) + model[j].y * cos(angle);
				
				x *= g_scale;
				y *= g_scale;
				
				LineTo(surface, g_agents[i]->x+x, g_agents[i]->y+y);
			}
			
	    	//put the old pen back
		    SelectObject(surface, g_oldPen);

		}
		
		for (unsigned int i=0; i < g_bullets.size(); ++i) {
			MoveToEx(surface, g_bullets[i]->x, g_bullets[i]->y,NULL);
			LineTo(surface, g_bullets[i]->x + g_bullets[i]->vx, g_bullets[i]->y + g_bullets[i]->vy);
		}
  	}
}




void addShip(int x, int y, int team) {
	Ship* a = new Ship;
	a->target = NULL;
	a->team = team;
	a->destroyed = false;
	a->x = (int)(RandFloat() * 10.0f - 5.0f + x);
	a->y = (int)(RandFloat() * 10.0f - 5.0f + y);

	a->vx = RandFloat() * (g_initial_speed) - g_initial_speed / 2.0f;
	a->vy = RandFloat() * (g_initial_speed) - g_initial_speed / 2.0f;
	a->target_acquired = false;
	g_agents.push_back(a);
	
	g_ships_created ++;
}

bool rmousedown = false;
bool lmousedown = false;
// This is the famous WndProc function which handles the Windows message loop.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	//these hold the dimensions of the client window area
	static int cxClient, cyClient;

	//used to create the back buffer
	static HDC		  hdcBackBuffer;
	static HBITMAP	hBitmap;
	static HBITMAP	hOldBitmap; 

	int mouseX,mouseY;

	
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

		case WM_MOUSEMOVE:
			mouseX = LOWORD(lparam);
			mouseY = HIWORD(lparam);
			if (lmousedown) addShip(mouseX,mouseY,0);
			else if (rmousedown) addShip(mouseX, mouseY, 1);
			break;
		case WM_LBUTTONDOWN:
			lmousedown = true;
			mouseX = LOWORD(lparam);
			mouseY = HIWORD(lparam);
			addShip(mouseX, mouseY,0);
			PostMessage(GetParent(hwnd), WM_COMMAND, GetDlgCtrlID(hwnd), (LPARAM)hwnd);
		break;
		
		case WM_RBUTTONDOWN:
			rmousedown = true;
			mouseX = LOWORD(lparam);
			mouseY = HIWORD(lparam);	
			addShip(mouseX, mouseY, 1);
		break;
		
		case WM_LBUTTONUP:
			lmousedown = false;
			break;
		case WM_RBUTTONUP:
			rmousedown = false;
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

void FireBullet(float x, float y, float vx, float vy, Ship* fromship) {
	Bullet* bullet = new Bullet;
	bullet->destroyed = false;
	bullet->x = x;
	bullet->y = y;
	bullet->vx = vx;
	bullet->vy = vy;
	
	bullet->origin_ship = fromship;
	
	g_bullets.push_back(bullet);
}

void CheckBullets() {
	
	for (unsigned int i = 0; i < g_bullets.size(); ++i) {
		// check if a bullet is close to a g_agent.
		for (unsigned int j=0; j < g_agents.size(); ++j) {
			// compute distance from bullet i to agent j
			
			float bx = g_bullets[i]->x;
			float by = g_bullets[i]->y;
			
			float ax = g_agents[j]->x;
			float ay = g_agents[j]->y;
			
			if (g_bullets[i]->origin_ship == g_agents[j]) continue;
			// sqrt() is slow, i'm just multiplying the other side instead.
			if ((bx-ax)*(bx-ax) + (by-ay)*(by-ay) < g_bullet_close_distance*g_bullet_close_distance) {
				 if (g_bullets[i]->origin_ship != NULL && g_bullets[i]->origin_ship->team == g_agents[j]->team && !g_friendly_fire) continue;
				
				for (unsigned int a=0; a < g_bullets.size(); ++a) {
					
					if (g_bullets[a]->origin_ship == g_agents[j]) {
						g_bullets[a]->origin_ship = NULL;
					}
				}
				
				
				if (g_bullets[i]->origin_ship != NULL)
					g_bullets[i]->origin_ship->target_acquired = false;
				
				
				MakeExplosion(g_agents[j]->x, g_agents[j]->y);
				
				
				g_bullets[i]->destroyed = true;
				g_agents[j]->destroyed = true;
				//delete g_bullets[i];
				//delete g_agents[j];
				//g_bullets.erase(g_bullets.begin()+i);
				//g_agents.erase(g_agents.begin()+j);
				
			}
		}
		g_bullets[i]->x += g_bullets[i]->vx * g_global_speed;
		g_bullets[i]->y += g_bullets[i]->vy * g_global_speed;
		
		if (g_bullets[i]->x < 0 || g_bullets[i]->x > g_windowWidth ||
				g_bullets[i]->y < 0 || g_bullets[i]->y > g_windowHeight) {
					g_bullets[i]->destroyed = true;
					//delete g_bullets[i];
					//g_bullets.erase(g_bullets.begin() + i);
		}
	}
}

void MakeExplosion(float x, float y) {
	Explosion *a = new Explosion;
	a->x = RandFloat() * 5.0f - 10.0f + x;
	a->y = RandFloat() * 5.0f - 10.0f + y;
	a->step = 0;

	// explosions don't need a velocity
	g_explosions.push_back(a);
}

void UpdateExplosions() {/*
	
	for (int i=0; i < g_explosions.size(); i ++) {
		if (g_explosions[i]->step >= 10) 
		g_explosions[i].step ++;
	}*/
}



int WINAPI WinMain(	HINSTANCE hinstance, HINSTANCE hprevinstance, LPSTR lpcmdline, int ncmdshow) {
//	g_agents.resize(g_agent_count);
	for (int i=0; i < g_initial_agent_count; ++i) {
		Ship* newagent = new Ship;
		newagent->target = NULL;
		newagent->team = RandInt(0, g_team_count-1);
		newagent->destroyed = false;
		newagent->x = (int)(RandFloat() * g_windowWidth);
		newagent->y = (int)(RandFloat() * g_windowHeight);
		
		newagent->vx = RandFloat() * g_initial_speed - g_initial_speed/2.0f;
		newagent->vy = RandFloat() * g_initial_speed - g_initial_speed/2.0f;
		g_agents.push_back(newagent);
	}
	
	


//	for (int i = 0; i < g_agentCount; ++i) {
//		g_agents[i].x = (int)(RandFloat() * g_windowWidth);
//		g_agents[i].y = (int)(RandFloat() * g_windowHeight);
//	}

	
	model[0].x = -1.0f;   model[1].x = 0.0f;
	model[0].y =  1.5f;   model[1].y = -1.5f;
	
	model[2].x =   1.0f;   model[3].x = 0.0f;
	model[2].y =   1.5f;   model[3].y = 0.0f; 
	
	
	
	
	WNDCLASSEX winclass; 
	HWND	   hwnd;	 
	MSG		   msg;		 

	srand(time(NULL));

	//create a pen for the graph drawing
	g_bluePen  = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
	g_redPen   = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	g_greenPen = CreatePen(PS_SOLID, 1, RGB(0, 150, 0));


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
	LONGLONG perfCountFreq = 0, currentTime = 0, nextTime = 0, frameTime = 0;//, lastTime = 0;
	//double timeElapsed = 0.0;
	//double timeScale = 0.0;

	QueryPerformanceFrequency((LARGE_INTEGER*) &perfCountFreq);
	//timeScale = 1.0f/perfCountFreq;
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
			
			CheckBullets();
			CleanUpSprites();

			if (currentTime > nextTime) {
				//timeElapsed = (currentTime - lastTime) * timeScale;
				//lastTime = currentTime;
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
