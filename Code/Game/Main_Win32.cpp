#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>					// #ToDo: Remove all references to OpenGL
#include <math.h>
#include <cassert>
#include <crtdbg.h>
#include "Engine/Math/Vector2.hpp"
#include "Game/App.hpp"
#include <time.h>
#include "Engine/Core/ProfileLogScope.hpp"


//-----------------------------------------------------------------------------------------------
#define UNUSED(x) (void)(x);


//-----------------------------------------------------------------------------------------------
const int OFFSET_FROM_WINDOWS_DESKTOP = 50;
const int WINDOW_PHYSICAL_WIDTH = 1600;
const int WINDOW_PHYSICAL_HEIGHT = 900;
const double VIEW_LEFT = 0.0;
const double VIEW_RIGHT = 1600.0;
const double VIEW_BOTTOM = 0.0;
const double VIEW_TOP = VIEW_RIGHT * static_cast< double >(WINDOW_PHYSICAL_HEIGHT) / static_cast< double >(WINDOW_PHYSICAL_WIDTH);


//-----------------------------------------------------------------------------------------------
// App* g_theApp = nullptr;
HWND g_hWnd = nullptr;
HDC g_displayDeviceContext = nullptr;
HGLRC g_openGLRenderingContext = nullptr;
const char* APP_NAME = "Win32 OpenGL Test App";


//-----------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowsMessageHandlingProcedure( HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam )
{
	unsigned char asKey = (unsigned char) wParam;
	switch( wmMessageCode )
	{
		case WM_CLOSE:
		case WM_DESTROY:
		case WM_QUIT:
			if(g_theApp)
				g_theApp->SetIsQuitting(true);
			return 0;

		case WM_KEYDOWN:
			if (g_theApp)
				g_theApp->RegisterKeyDown(asKey);
			break;

		case WM_KEYUP:
			if (g_theApp)
				g_theApp->RegisterKeyUp(asKey);
			break;

		case WM_LBUTTONDOWN:
			if (g_theApp)
				g_theApp->RegisterKeyDown(KEYCODE_LMB);
			break;

		case WM_RBUTTONDOWN:
			if (g_theApp)
				g_theApp->RegisterKeyDown(KEYCODE_RMB);
			break;

		case WM_LBUTTONUP:
			if (g_theApp)
				g_theApp->RegisterKeyUp(KEYCODE_LMB);
			break;

		case WM_RBUTTONUP:
			if (g_theApp)
				g_theApp->RegisterKeyUp(KEYCODE_RMB);
			break;

		case WM_MOUSEWHEEL:
			if(g_theApp)
			{
				g_theInput->SetMouseWheelRotation(GET_WHEEL_DELTA_WPARAM(wParam));
			}
			break;

		case WM_SETFOCUS:
			if (g_theApp)
				g_theApp->OnGainedFocus();
			break;

		case WM_KILLFOCUS:
			if (g_theApp)
				g_theApp->OnLostFocus();
			break;
	}

	return DefWindowProc( windowHandle, wmMessageCode, wParam, lParam );
}


//-----------------------------------------------------------------------------------------------
void CreateOpenGLWindow( HINSTANCE applicationInstanceHandle )
{
	// Define a window class
	WNDCLASSEX windowClassDescription;
	memset( &windowClassDescription, 0, sizeof( windowClassDescription ) );
	windowClassDescription.cbSize = sizeof( windowClassDescription );
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast< WNDPROC >(WindowsMessageHandlingProcedure); // Assign a win32 message-handling function
	windowClassDescription.hInstance = GetModuleHandle( NULL );
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT( "Simple Window Class" );
	RegisterClassEx( &windowClassDescription );

	const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect( desktopWindowHandle, &desktopRect );

	int windowHeight;
	int windowWidth;
	
	if (desktopRect.bottom > desktopRect.right)
	{
		windowWidth = (int)(desktopRect.right * 0.85f);
		windowHeight = (int)(windowWidth * (9.f / 16.f));
	}
	else
	{
		windowHeight = (int)(desktopRect.bottom * 0.85f);
		windowWidth = (int)(windowHeight * (16.f / 9.f));
	}

	int windowHorizontalOffset = (desktopRect.right - windowWidth) / 2;
	int windowVerticalOffset = (desktopRect.bottom - windowHeight) / 2;

	int windowLeft = windowHorizontalOffset;
	int windowRight = windowWidth + windowHorizontalOffset;
	int windowTop = windowVerticalOffset;
	int windowBottom = windowHeight + windowVerticalOffset;

	RECT windowRect = { windowLeft, windowTop, windowRight, windowBottom};
	AdjustWindowRectEx( &windowRect, windowStyleFlags, FALSE, windowStyleExFlags );

	WCHAR windowTitle[ 1024 ];
	MultiByteToWideChar( GetACP(), 0, APP_NAME, -1, windowTitle, sizeof( windowTitle ) / sizeof( windowTitle[ 0 ] ) );
	g_hWnd = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		applicationInstanceHandle,
		NULL );

	ShowWindow( g_hWnd, SW_SHOW );
	SetForegroundWindow( g_hWnd );
	SetFocus( g_hWnd );

	g_displayDeviceContext = GetDC( g_hWnd );

	HCURSOR cursor = LoadCursor( NULL, IDC_ARROW );
	SetCursor( cursor );

	PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
	memset( &pixelFormatDescriptor, 0, sizeof( pixelFormatDescriptor ) );
	pixelFormatDescriptor.nSize = sizeof( pixelFormatDescriptor );
	pixelFormatDescriptor.nVersion = 1;
	pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
	pixelFormatDescriptor.cColorBits = 24;
	pixelFormatDescriptor.cDepthBits = 24;
	pixelFormatDescriptor.cAccumBits = 0;
	pixelFormatDescriptor.cStencilBits = 8;

	int pixelFormatCode = ChoosePixelFormat( g_displayDeviceContext, &pixelFormatDescriptor );
	SetPixelFormat( g_displayDeviceContext, pixelFormatCode, &pixelFormatDescriptor );
	g_openGLRenderingContext = wglCreateContext( g_displayDeviceContext );
	wglMakeCurrent( g_displayDeviceContext, g_openGLRenderingContext );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glLineWidth( 2.f );
	glEnable( GL_LINE_SMOOTH );
}


//-----------------------------------------------------------------------------------------------
// Processes all windows messages (WM_xxx) for this app that have queued up since last frame.
//
void RunMessagePump()
{
	MSG queuedMessage;
	for( ;; )
	{
		const BOOL wasMessagePresent = PeekMessage( &queuedMessage, NULL, 0, 0, PM_REMOVE );
		if( !wasMessagePresent )
		{
			break;
		}

		TranslateMessage( &queuedMessage );
		DispatchMessage( &queuedMessage );
	}
}


//-----------------------------------------------------------------------------------------------
void Initialize( HINSTANCE applicationInstanceHandle )
{
	SetProcessDPIAware();
	CreateOpenGLWindow( applicationInstanceHandle );
	g_theApp = new App();
	g_theApp->OnGainedFocus();
}


//-----------------------------------------------------------------------------------------------
void Shutdown()
{
	delete g_theApp;
	g_theApp = nullptr;
}


//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int )
{
	UNUSED( commandLineString );
	Initialize( applicationInstanceHandle );

	while( !g_theApp->IsQuitting() )
	{
// 		Sleep(1);
		RunMessagePump();
		g_theApp->RunFrame();
		SwapBuffers(g_displayDeviceContext); // Note: call this once at the end of each frame
	}

	Shutdown();
	return 0;
}


