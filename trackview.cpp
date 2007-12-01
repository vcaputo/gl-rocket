#include "stdafx.h"

#include "trackview.h"

void TrackView::onCreate(HWND hwnd)
{
	scrollPosX = 0;
	scrollPosY = 0;
	windowWidth  = -1;
	windowHeight = -1;
	setupScrollBars(hwnd);
}

void TrackView::onPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	
	SetBkMode(hdc, OPAQUE);
//	SetBkColor(hdc, RGB(255, 0, 0));

	/*
	RECT margin = rect;
	
	margin.left		= 0;
	margin.right	= LeftMarginWidth();
	rect.left	   += LeftMarginWidth();

	PaintMargin(hdc, nLineNo, &margin);
*/
	
	paintTracks(hdc, ps.rcPaint);
	
	EndPaint(hwnd, &ps);
}

void TrackView::paintTracks(HDC hdc, RECT rcTracks)
{
	char temp[256];
	
	int firstLine = max((rcTracks.top - topMarginHeight) / fontHeight, 0);
	int lastLine  = scrollPosY + ((rcTracks.bottom - topMarginHeight) + (fontHeight - 1)) / fontHeight;

	lastLine = min(lastLine, lines);
	
	SetBkMode(hdc, TRANSPARENT);

	for (int y = firstLine; y < lastLine; ++y)
	{
		RECT leftMargin;
		leftMargin.left = 0;
		leftMargin.right = leftMarginWidth;
		leftMargin.top = topMarginHeight + (y - scrollPosY) * fontHeight;
		leftMargin.bottom = leftMargin.top + fontHeight;

		FillRect(hdc, &leftMargin, (HBRUSH)GetStockObject(LTGRAY_BRUSH));

		if ((y % 16) == 0)      SetTextColor(hdc, RGB(0, 0, 0));
		else if ((y % 8) == 0)  SetTextColor(hdc, RGB(32, 32, 32));
		else if ((y % 4) == 0)  SetTextColor(hdc, RGB(64, 64, 64));
		else                    SetTextColor(hdc, RGB(128, 128, 128));

		_snprintf_s(temp, 256, "%04Xh", y);
		TextOut(hdc,
			leftMargin.left, leftMargin.top,
			temp, int(strlen(temp))
		);

		ExcludeClipRect(hdc, leftMargin.left, leftMargin.top, leftMargin.right, leftMargin.bottom);
	}
	
	SetTextColor(hdc, RGB(0, 0, 0));
	
	int trackLeft = leftMarginWidth - scrollPosX;
	for (int x = 0; x < tracks; ++x)
	{
		int trackWidth = fontWidth * 5;
		
		RECT topMargin;

		topMargin.top = 0;
		topMargin.bottom = topMarginHeight;

		topMargin.left = trackLeft;
		topMargin.right = trackLeft + trackWidth;
		
		FillRect( hdc, &topMargin, (HBRUSH)GetStockObject(LTGRAY_BRUSH));

		/* format the text */
		_snprintf_s(temp, 256, "track %d", x);
		TextOut(hdc,
			topMargin.left, 0,
			temp, int(strlen(temp))
		);
		ExcludeClipRect(hdc, topMargin.left, topMargin.top, topMargin.right, topMargin.bottom);

//		SetBkColor(hdc, RGB(255, 0, 0));
		int trackTop = topMarginHeight + (firstLine - scrollPosY) * fontHeight;
		for (int y = firstLine; y < lastLine; ++y)
		{
			RECT patternDataRect;
			patternDataRect.left = trackLeft;
			patternDataRect.right = trackLeft + trackWidth;
			patternDataRect.top = trackTop;
			patternDataRect.bottom = patternDataRect.top + fontHeight;
			FillRect( hdc, &patternDataRect, (HBRUSH)GetStockObject(WHITE_BRUSH));
			
			bool key = (y % 8 == 0);
			float val = (float(y) / 16);
			
			/* format the text */
			if (!key) _snprintf_s(temp, 256, " - - -");
			else _snprintf_s(temp, 256, "%2.2f", val);
			TextOut(hdc,
				trackLeft, trackTop,
				temp, int(strlen(temp))
			);
			trackTop += fontHeight;
		}

		RECT bottomMargin;
		bottomMargin.top = trackTop;
		bottomMargin.bottom = rcTracks.bottom;
		bottomMargin.left = trackLeft;
		bottomMargin.right = trackLeft + trackWidth;
		FillRect( hdc, &bottomMargin, (HBRUSH)GetStockObject(LTGRAY_BRUSH));

		trackLeft += trackWidth;
	}

	/* pad top margin to the left edge */
	RECT topMargin;
	topMargin.top = 0;
	topMargin.bottom = topMarginHeight;
	topMargin.left = trackLeft;
	topMargin.right = rcTracks.right;
	FillRect( hdc, &topMargin, (HBRUSH)GetStockObject(LTGRAY_BRUSH));

	/* right margin */
	RECT rightMargin;
	rightMargin.top    = topMarginHeight;
	rightMargin.bottom = rcTracks.bottom;
	rightMargin.left  = trackLeft;
	rightMargin.right = rcTracks.right;
	FillRect( hdc, &rightMargin, (HBRUSH)GetStockObject(LTGRAY_BRUSH));
}

void TrackView::setupScrollBars(HWND hwnd)
{
	SCROLLINFO si = { sizeof(si) };
	si.fMask = SIF_POS | SIF_PAGE | SIF_RANGE | SIF_DISABLENOSCROLL;
	si.nPos  = scrollPosY;
	si.nPage = windowLines;
	si.nMin  = 0;
	si.nMax  = lines - 1;
	SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
	
	si.fMask = SIF_POS | SIF_PAGE | SIF_RANGE | SIF_DISABLENOSCROLL;
	si.nPos  = scrollPosX;
	si.nPage = windowWidth;
	si.nMin  = 0;
	si.nMax  = windowWidth * 2; // 0x80;
	SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
}

void TrackView::setScrollPos(HWND hwnd, int newScrollPosX, int newScrollPosY)
{
	const int oldScrollPosX = scrollPosX;
	const int oldScrollPosY = scrollPosY;

	// update scrollPos
	scrollPosX = newScrollPosX;
	scrollPosY = newScrollPosY;

	// clamp scrollPos
	scrollPosX = max(scrollPosX, 0);
	scrollPosY = max(scrollPosY, 0);

	scrollPosY = min(scrollPosY, (lines - 1) - windowLines);

	if (oldScrollPosX != scrollPosX || oldScrollPosY != scrollPosY)
	{
		int scrollX = oldScrollPosX - scrollPosX;
		int scrollY = (oldScrollPosY - scrollPosY) * fontHeight;
		
		RECT clip;
		GetClientRect(hwnd, &clip);
		
		if (scrollX == 0) clip.top  = topMarginHeight; /* don't scroll the top line */
		if (scrollY == 0) clip.left = leftMarginWidth; /* don't scroll the top line */
		
		ScrollWindowEx(
			hwnd,
			scrollX,
			scrollY,
			NULL,
			&clip,
			NULL,
			NULL,
			SW_INVALIDATE
		);
		setupScrollBars(hwnd);
	}
}

static int getScrollPos(HWND hwnd, int bar)
{
	SCROLLINFO si = { sizeof(si), SIF_TRACKPOS };
	GetScrollInfo(hwnd, bar, &si);
	return int(si.nTrackPos);
}

void TrackView::onVScroll(HWND hwnd, UINT sbCode, int newPos)
{
	switch (sbCode)
	{
	case SB_TOP:
		setScrollPos(hwnd, scrollPosX, 0);
	break;
	
	case SB_LINEUP:
		setScrollPos(hwnd, scrollPosX, scrollPosY - 1);
	break;
	
	case SB_LINEDOWN:
		setScrollPos(hwnd, scrollPosX, scrollPosY + 1);
	break;
	
	case SB_PAGEUP:
		setScrollPos(hwnd, scrollPosX, scrollPosY - windowLines);
	break;
	
	case SB_PAGEDOWN:
		setScrollPos(hwnd, scrollPosX, scrollPosY + windowLines);
	break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		setScrollPos(hwnd, scrollPosX, getScrollPos(hwnd, SB_VERT));
	break;
	}
}

void TrackView::onHScroll(HWND hwnd, UINT sbCode, int newPos)
{
	switch (sbCode)
	{
	case SB_LEFT:
		setScrollPos(hwnd, 0, scrollPosY);
	break;
	// SB_RIGHT currently missing.
	
	case SB_LINELEFT:
		setScrollPos(hwnd, scrollPosX - fontWidth, scrollPosY);
	break;
	
	case SB_LINERIGHT:
		setScrollPos(hwnd, scrollPosX + fontWidth, scrollPosY);
	break;
	
	case SB_PAGELEFT:
		setScrollPos(hwnd, scrollPosX - 20, scrollPosY);
	break;
	
	case SB_PAGEDOWN:
		setScrollPos(hwnd, scrollPosX + 20, scrollPosY);
	break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		setScrollPos(hwnd, getScrollPos(hwnd, SB_HORZ), scrollPosY);
	break;
	}
}

void TrackView::onSize(HWND hwnd, int width, int height)
{
	const int oldWindowWidth = windowWidth;
	const int oldWindowHeight = windowHeight;

	windowWidth  = width;
	windowHeight = height;

	windowLines   = (height - topMarginHeight) / fontHeight;
//	windowLines   = min((height - topMarginHeight) / fontHeight, lines);
	setupScrollBars(hwnd);
}

