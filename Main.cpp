#ifndef UNICODE
	#define UNICODE
#endif

#include <cstdlib>

#include "D2DWindow.hpp"


/// The application entry point
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// Create a new simple window
	D2DWindow window;

	// Exit if window not created
	if (window.createWindow() == false)
		return EXIT_FAILURE;
	
	// Show the window
	ShowWindow(window.getWindow(), nCmdShow);

	// Center the window
	window.centerWindow();

	// Variable to hold messages
	MSG msg{};

	// The render loop
	while (window.running) 
	{
		// Get messages from the message queue
		if (PeekMessageW(&msg, window.getWindow(), 0, 0, PM_REMOVE))
		{
			// Translate the message for processing
			TranslateMessage(&msg);

			// Send the message to the window
			DispatchMessageW(&msg);
		}
		else
			SendMessageW(window.getWindow(), WM_PAINT, 0, 0);
	}

	// Return 0 on exit
	return EXIT_SUCCESS;
}