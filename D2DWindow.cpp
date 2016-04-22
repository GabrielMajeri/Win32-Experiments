#include "D2DWindow.hpp"
#include <stdexcept>

bool D2DWindow::createWindow()
{
	return BaseWindow::createWindow(L"Direct2D Drawing", WS_OVERLAPPEDWINDOW, 0, 0, 0, 600, 600, 0, 0);
}

PCWSTR D2DWindow::getClassName() const
{
	return L"D2D Window Class";
}

LRESULT D2DWindow::handleMessage(unsigned uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PAINT:
		onPaint();
		return 0;

	case WM_SIZE:
		onResize();
		return 0;

	case WM_CREATE:
		// Try to create the Direct2D factory
		if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory)))
			return -1;
		
		return 0;

	// Release all resources and destroy the window
	case WM_DESTROY:
		discardGraphics();
		safeRelease(&factory);
		PostQuitMessage(0);
		running = false;
		return 0;
	
	// The default handler
	default:
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}
}

// Draws a clock hand
void D2DWindow::drawClockHand(float handLength, float angle, float strokeWidth)
{
	// Set the transform
	target->SetTransform(D2D1::Matrix3x2F::Rotation(angle, ellipse.point));

	// Determine the endpoint
	D2D_POINT_2F endPoint = D2D1::Point2F(
		ellipse.point.x, 
		ellipse.point.y - (ellipse.radiusY * handLength));

	// Stroke the line
	target->DrawLine(ellipse.point, endPoint, stroke, strokeWidth);
}

// Calculates the new layout of the window after a resize
void D2DWindow::calculateLayout()
{
	if (target) {
		// Get the new size of the window
		D2D1_SIZE_F size = target->GetSize();

		// Get the x and y of the center
		const float x = size.width / 2.f, y = size.height / 2.f,
			// Make sure the ellipse will fit
			radius = min(x, y);

		// Recalculate the ellipse's bounds
		ellipse = D2D1::Ellipse({ x, y }, radius, radius);
	}
}

// Creates the graphics resources
HRESULT D2DWindow::createGraphics()
{
	HRESULT hr = S_OK;

	// Skip if we already created the target
	if (target != nullptr)
		return hr;

	// Get the size of the window
	RECT rc;
	GetClientRect(hWnd, &rc);
	D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

	// Create a render target on the window
	hr = factory->CreateHwndRenderTarget(
		// Use default render target options
		D2D1::RenderTargetProperties(),
		// Pass the window handle and size
		D2D1::HwndRenderTargetProperties(hWnd, size),
		// The target to create
		&target);

	if (FAILED(hr))
		return hr;

	// Create the brushes
	hr = target->CreateSolidColorBrush(D2D1::ColorF(1.f, 1.f, 0.f), &brush);

	if (FAILED(hr))
		return hr;

	hr = target->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0), &stroke);

	if (FAILED(hr))
		return hr;
	
	// Update the layout
	calculateLayout();
	
	return hr;
}

// Releases the used resources
void D2DWindow::discardGraphics()
{
	// Release the render target
	safeRelease(&target);

	// Release the brushes
	safeRelease(&brush);
	safeRelease(&stroke);
}

void D2DWindow::onPaint()
{
	HRESULT hr = createGraphics();

	// Throw an exception and exit if we can't create graphics
	if (FAILED(hr))
		throw std::exception("Could not create graphics.");

	// Start painting
	PAINTSTRUCT ps;
	BeginPaint(hWnd, &ps);

	target->BeginDraw();

	// Fill the ellipse
	target->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));
	target->FillEllipse(ellipse, brush);

	SYSTEMTIME time;
	GetLocalTime(&time);

	// Determine the various hands angles
	const float hourAngle = (360.f / 12.f) * time.wHour + (time.wMinute * 0.5f),
		minutesAngle = (360.f / 60.f) * time.wMinute,
		secondsAngle = (360.f / 60.f) * time.wSecond + (time.wMilliseconds * (360 / 60000.f));

	// Draw the clock hands
	drawClockHand(0.6f, hourAngle, 6);
	drawClockHand(0.8f, minutesAngle, 4);
	drawClockHand(0.9f, secondsAngle, 2);

	// Stop painting
	hr = target->EndDraw();

	// If we failed to draw, then we discard the current render target
	if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
		discardGraphics();
	
	EndPaint(hWnd, &ps);
}

void D2DWindow::onResize()
{
	// Determine the window size
	RECT rc;
	GetClientRect(hWnd, &rc);
	D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

	windowWidth = size.width;
	windowHeight = size.height;

	// Nothing to do if we couldn't create the render target
	if (target == nullptr)
		return;

	// Resize the render target
	target->Resize(size);

	// Recalculate the layout
	calculateLayout();

	// Force Windows to refresh an area
	InvalidateRect(hWnd, &rc, false);
}
