#pragma once

#include "BaseWindow.hpp"
#include <ShObjIdl.h>
#include <atlbase.h>

#include <stdexcept>

class COMWindow final : public BaseWindow<COMWindow> {
public:
	COMWindow() : BaseWindow()
	{
		// Initialize the COM library
		HRESULT hrCOM = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

		// If we can't initialize the COM library, just exit
		if (FAILED(hrCOM))
			throw std::exception("Could not initialize COM library.");
	}

	~COMWindow()
	{
		// Uninitialize the COM library
		CoUninitialize();
	}

	bool createWindow()
	{
		// Create the showcase window with some simple settings
		return BaseWindow::createWindow(
			L"COM Showcase", WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX & ~WS_MAXIMIZEBOX, 0,
			0, 0,
			400, 300,
			0, 0);
	}

	PCWSTR getClassName() const override
	{
		return L"Simple Window with COM file dialog";
	}

	LRESULT handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
			// If the user tries to destroy the window
		case WM_DESTROY:
			// Send a quit message with value of false to exit the loop
			PostQuitMessage(false);
			return 0;

			// If the user clicks the window
		case WM_LBUTTONDOWN:
		{
			// Pointer for the file dialog
			CComPtr<IFileOpenDialog> fileDialog;

			// Try to create the dialog
			HRESULT hr = fileDialog.CoCreateInstance(__uuidof(FileOpenDialog));

			// If it failed to create the file dialog
			if (FAILED(hr)) {
				displayError(L"Could not open file selection dialog!");
				return 0;
			}

			// Ask the user to select a file
			hr = fileDialog->Show(nullptr);

			// If the user chose Cancel
			if (FAILED(hr)) {
				displayError(L"No file selected!");
				return 0;
			}

			// The selected file
			CComPtr<IShellItem> item;

			// Get the selected file
			hr = fileDialog->GetResult(&item);

			if (FAILED(hr)) {
				displayError(L"File selection failed!");
				return 0;
			}

			PWSTR filePath;
			// Get the absolute name of the chosen file
			hr = item->GetDisplayName(SIGDN_FILESYSPATH, &filePath);

			if (FAILED(hr)) {
				displayError(L"Could not get file path!");
				return 0;
			}

			// Display the absolute file path
			MessageBox(hWnd, filePath, L"File selected", MB_OK);

			// Release memory from the string
			CoTaskMemFree(filePath);

			return 0;
		}

		// If Windows wants to repaint the app
		case WM_PAINT:
		{
			PAINTSTRUCT ps;

			// Fill in the paint structure with info from the area to repaint
			HDC hDC = BeginPaint(hWnd, &ps);

			// Create a new solid brush for the background
			HBRUSH color{ CreateSolidBrush(RGB(120, 120, 120)) };

			// Paint the window
			FillRect(hDC, &ps.rcPaint, color);

			// Stop painting
			EndPaint(hWnd, &ps);

			// Delete the brush to save memory
			DeleteObject(color);
		}

		return 0;

		default:
			// If we don't handle the message, use the default window process
			return DefWindowProcW(hWnd, uMsg, wParam, lParam);
		}
	}
};