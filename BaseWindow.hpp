#pragma once

#include <Windows.h>

template <typename T>
class BaseWindow {
public:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// Try to get a pointer to the current instance
		T* pThis{ nullptr };

		// If this is the first call to the window process
		if (uMsg == WM_NCCREATE) {
			// Obtain the data from the pointer
			CREATESTRUCT* create = reinterpret_cast<CREATESTRUCT*>(lParam);

			// Extract the pointer to the instance
			pThis = reinterpret_cast<T*>(create->lpCreateParams);

			// Store the variable for further uses
			SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG>(pThis));

			// Store the window handle
			pThis->hWnd = hWnd;
		}
		else
			// Just retrieve the data
			pThis = reinterpret_cast<T*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
		
		// If we have a valid instance
		if (pThis)
			return pThis->handleMessage(uMsg, wParam, lParam);
		
		// Fallback in case we do not have an instance
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}

	BaseWindow() : hWnd{ nullptr } { }

	bool createWindow(PCWSTR windowName, DWORD style, DWORD exStyle = 0,
		unsigned x = CW_USEDEFAULT, unsigned y = CW_USEDEFAULT,
		unsigned width = CW_USEDEFAULT, unsigned height = CW_USEDEFAULT,
		HWND hWndParent = 0, HMENU hMenu = 0)
	{
		// Create a new window class with default values
		WNDCLASS wc{};

		// Set the window process
		wc.lpfnWndProc = T::WindowProc;
		// Set the instance handle
		wc.hInstance = GetModuleHandleW(nullptr);
		// Set the class name
		wc.lpszClassName = getClassName();
		// Set the cursor inside the window
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

		// Register this window with Windows
		RegisterClassW(&wc);

		// Actually create a window
		hWnd = CreateWindowExW(
			exStyle, // Optional window styles
			getClassName(), // Class name
			windowName, // Title
			style, // Window style

			x, y, // The X and Y coordinates
			width, height, // The width and height

			hWndParent, // The parent window, if any
			hMenu, // The menu of this window
			GetModuleHandleW(nullptr), // The handle to this instance
			this // Additional data
		);

		// Store the window dimensions for later use
		windowWidth = width;
		windowHeight = height;

		return hWnd ? true : false;
	}

	// Returns the window's handle
	HWND getWindow() const 
	{
		return hWnd;
	}

	// Sets the window's position
	void setPosition(unsigned x, unsigned y)
	{
		SetWindowPos(hWnd, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

	// Centers the window
	void centerWindow()
	{
		int xPos = (GetSystemMetrics(SM_CXSCREEN) - windowWidth) / 2,
			yPos = (GetSystemMetrics(SM_CYSCREEN) - windowHeight) / 2;
		setPosition(xPos, yPos);
	}

	
protected:
	// Helper function to display errors more easily
	void displayError(PWSTR err)
	{
		MessageBox(hWnd, err, L"Error!", MB_ICONERROR);
	}

	HWND hWnd;
	unsigned windowWidth, windowHeight;

	virtual PCWSTR getClassName() const = 0;

	virtual LRESULT handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
};

template <typename T>
void safeRelease(T** ptr)
{
	if (*ptr != nullptr)
	{
		(*ptr)->Release();
		*ptr = nullptr;
	}
}