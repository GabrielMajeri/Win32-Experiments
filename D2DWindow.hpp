#pragma once

#include "BaseWindow.hpp"


#include <d2d1.h>
#pragma comment(lib, "d2d1")

class D2DWindow : public BaseWindow<D2DWindow>
{
public:
	bool createWindow();
		
	PCWSTR getClassName() const override;

	LRESULT handleMessage(unsigned uMsg, WPARAM wParam, LPARAM lParam) override;
	
	// Variable that determines when to quit
	bool running{ true };

private:
	ID2D1Factory* factory{ nullptr };
	ID2D1HwndRenderTarget* target{ nullptr };
	ID2D1SolidColorBrush* brush{ nullptr }, * stroke{ nullptr };
	D2D1_ELLIPSE ellipse;

	void calculateLayout();
	HRESULT createGraphics();
	void discardGraphics();
	void onPaint();
	void onResize();

	void drawClockHand(float handLength, float angle, float strokeWidth);
};