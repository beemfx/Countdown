#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef enum tagWINDOWPOSITION {POS_UNKNOWN=0, UPPER_LEFT, UPPER_RIGHT, LOWER_LEFT, LOWER_RIGHT} WINDOWPOSITION;


signed long ConvertCharToDate(struct tm * sDateOutput, char * szInput)
{
	char szTemp[100];
	struct tm sDate;
	
	ZeroMemory(&sDate, sizeof(struct tm));
	
	if(szInput!=NULL)
		strcpy(szTemp, szInput);
	else
		strcpy(szTemp, "1/1/1900");
	
	sDate.tm_mon=atoi(strtok(szTemp, "/"))-1;
	sDate.tm_mday=atoi(strtok(NULL, "/"));
	sDate.tm_year=atoi(strtok(NULL, "/"))-1900;
	
	sDate.tm_wday=0;
	sDate.tm_yday=0;
	sDate.tm_isdst=-1;
	
	sDate.tm_sec=0;
	sDate.tm_min=0;
	sDate.tm_hour=0;
	
	if(sDateOutput)
		memcpy(sDateOutput, &sDate, sizeof(struct tm));
	
	return mktime(&sDate);
}

signed long SecondsLeft(char* szOutput, const time_t nDepartTime)
{
	time_t nTime=0, nTimeLeft=0;
	char szBeginString[11];
	
	nTime=time(NULL);



	
	nTimeLeft=nDepartTime-nTime;

	if(nTimeLeft>=0)
		strcpy(szBeginString, "Time Left");
	else
		strcpy(szBeginString, "Time Since");

	nTimeLeft=abs(nTimeLeft);
	
	sprintf(szOutput, "%s:\n\nSeconds: %i\nMinutes: %i\nHours:   %i\nDays:    %i\nWeeks:   %i\nMonths:  %.1f",
		szBeginString, 
		nTimeLeft, 
		nTimeLeft/60,
		nTimeLeft/60/60,
		nTimeLeft/60/60/24,
		nTimeLeft/60/60/24/7,
		(float)nTimeLeft/60/60/24/31);

	return (signed long)nDepartTime;
}

int GetInfo(char*szNameOut, time_t*nDate, WINDOWPOSITION*nType)
{
	char szType[256];
	char szDate[256];
	char szName[256];
	//char szTemp[256];
	FILE * fIn=fopen("countdown.inf", "r");
	
	if(fIn==NULL)
		return 0;
		
	if((szName==NULL) || (szDate==NULL) || (szType==NULL)){
		fclose(fIn);
		return 0;
	}
		
	if(fgets(szName, 256, fIn)==NULL){
		fclose(fIn);
		return 0;
	}
	
	if(fgets(szDate, 256, fIn)==NULL){
		fclose(fIn);
		return 0;
	}
	
	if(fgets(szType, 256, fIn)==NULL){
		fclose(fIn);
		return 0;
	}
	
	fclose(fIn);
	
	
	/* Probably should do some error checking for the following code. */
	*nDate=ConvertCharToDate(NULL, szDate);
	if(_strnicmp("ul", szType, 2)==0)
		*nType=UPPER_LEFT;
	else if(_strnicmp("ur", szType, 2)==0)
		*nType=UPPER_RIGHT;
	else if(_strnicmp("ll", szType, 2)==0)
		*nType=LOWER_LEFT;
	else if(_strnicmp("lr", szType, 2)==0)
		*nType=LOWER_RIGHT;
	else
		*nType=POS_UNKNOWN;
		
	strcpy(szNameOut, szName);
	
	szNameOut[strlen(szNameOut)-1]=0;
	
	/*	
	sprintf(szTemp, "Name: %s Date: %s Type: %i", szNameOut, ctime(nDate), *nType);
	MessageBox(0, szTemp, 0, 0);
	*/
	
	return 1;
}


HRESULT PaintWindow(HWND hwnd, HFONT hFont, const time_t nDepartTime)
{
	char szText[256];
	RECT rect;
	HDC hdc=NULL;
	PAINTSTRUCT ps;
	HFONT hOldFont=NULL;
	HPEN hOldPen=NULL;
	HPEN hpen=CreatePen(PS_SOLID, 3, 0x000000FF);
	
	GetClientRect(hwnd, &rect);
	
	/* Get the amount of seconds left. */
	SecondsLeft(szText, nDepartTime);
	
	hdc=BeginPaint(hwnd, &ps);
	hOldPen=SelectObject(hdc, hpen);
	MoveToEx(hdc, 0, 0, NULL);
	LineTo(hdc, rect.right-2, 0);
	LineTo(hdc, rect.right-2, rect.bottom-2);
	LineTo(hdc, 0, rect.bottom-2);
	LineTo(hdc, 0, 0);
	hOldFont=SelectObject(hdc, hFont);
	rect.top+=8;
	rect.left+=8;
	DrawText(hdc, szText, strlen(szText), &rect, 0);
	SelectObject(hdc, hOldFont);
	SelectObject(hdc, hOldPen);
	/* Draw a border */
	EndPaint(hwnd, &ps);
	DeleteObject(hpen);
	return S_OK;
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
   static HFONT hFont=NULL;
	static time_t nDepartTime=0;
	
	switch(nMsg)
    {
	case WM_CREATE:
		
		hFont=CreateFont(
			16, 
			0, 
			0, 
			0, 
			FW_BOLD, 
			FALSE, 
			FALSE, 
			FALSE, 
			ANSI_CHARSET, 
			OUT_DEFAULT_PRECIS, 
			CLIP_DEFAULT_PRECIS, 
			DEFAULT_QUALITY,
			DEFAULT_PITCH, 
			"Courier");
		
		//hFont=GetStockObject(ANSI_FIXED_FONT);
		
		SetTimer(hwnd, 1, 1000, NULL);
		nDepartTime=(time_t)(((LPCREATESTRUCT)lParam)->lpCreateParams);
		
		break;
	/*
	case WM_SYSCOMMAND:
		switch(wParam)
		{
		case SC_MINIMIZE:
			//don't do anything
			break;
		default:
			DefWindowProc(hwnd, nMsg, wParam, lParam);
		}
		break;
	*/
	case WM_LBUTTONDBLCLK:
		SendMessage(hwnd, WM_CLOSE, 0, 0);
		break;
    case WM_PAINT:
		PaintWindow(hwnd, hFont, nDepartTime);
		break;
	case WM_TIMER:
		InvalidateRect(hwnd, NULL, TRUE);
		break;
	case WM_CLOSE:
		if(MessageBox(hwnd, "Are you sure you want to quit?", "Countdown", MB_YESNO|MB_ICONINFORMATION)==IDNO)
			break;
	case WM_DESTROY:
		DeleteObject(hFont);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, nMsg, wParam, lParam);
    }
    return 0l;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR szCmdLine, int nShowCmd)
{
	HWND hwnd=NULL;
    WNDCLASSEX wc;
    MSG msg;
    static const TCHAR szAppName[] = TEXT("Countdown");
	RECT rectDesktop={0, 0, 0, 0};
	RECT rectSize={0, 0, 0, 0};
	
	BOOL bBottomCornerPopup=FALSE;
	WINDOWPOSITION nWindowPos=POS_UNKNOWN;
	
	char szName[256], szWinText[256];
	time_t nDate;	

    wc.cbSize=sizeof(wc);
    wc.style = CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;
    wc.cbClsExtra=0;
    wc.cbWndExtra=0;
    wc.lpfnWndProc=WndProc;
    wc.hInstance=hInst;
    wc.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.hIcon = NULL;
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszMenuName=NULL;
    wc.lpszClassName=szAppName;

    if(!RegisterClassEx(&wc))
	{
		MessageBox(NULL,TEXT("This program requires Windows NT!"),TEXT("Notice"),MB_OK|MB_ICONERROR);
        return 0;
	}
	
	if(!GetInfo(szName, &nDate, &nWindowPos)){
		MessageBox(NULL, "Error: Could not load setup values.", "Error", MB_OK|MB_ICONERROR);
		return 0;
	}
	if(nWindowPos==POS_UNKNOWN)
		bBottomCornerPopup=FALSE;
	else
		bBottomCornerPopup=TRUE;
		
	GetClientRect(GetDesktopWindow(), &rectDesktop);
	rectDesktop.bottom-=GetSystemMetrics(SM_CYCAPTION);
	
	//Set the window rect.
	#define DWINDOW_WIDTH (200+GetSystemMetrics(SM_CXFIXEDFRAME)*2)
	#define DWINDOW_HEIGHT (150+GetSystemMetrics(SM_CYFIXEDFRAME)*2)
	rectSize.bottom=DWINDOW_HEIGHT;
	rectSize.right=DWINDOW_WIDTH;
	if(bBottomCornerPopup){
		switch(nWindowPos)
		{
		case UPPER_LEFT:
			rectSize.left=0;
			rectSize.top=0;//rectDesktop.right-DWINDOW_WIDTH;
			break;
		case UPPER_RIGHT:
			rectSize.top=0;
			rectSize.left=rectDesktop.right-DWINDOW_WIDTH;
			break;
		case LOWER_LEFT:
			rectSize.left=0;
			rectSize.top=rectDesktop.bottom-DWINDOW_HEIGHT;
			break;
		default:
		case LOWER_RIGHT:
			rectSize.left=rectDesktop.right-DWINDOW_WIDTH;
			rectSize.top=rectDesktop.bottom-DWINDOW_HEIGHT;
			break;
		}
	}else{
		rectSize.top=CW_USEDEFAULT;
		rectSize.left=CW_USEDEFAULT;
		rectSize.right=DWINDOW_WIDTH;
		rectSize.bottom=DWINDOW_HEIGHT+GetSystemMetrics(SM_CYCAPTION);
	}

	hwnd = CreateWindowEx(
			(bBottomCornerPopup?WS_EX_TOOLWINDOW:0),
            szAppName,
            szAppName,
            (bBottomCornerPopup?WS_POPUP:(WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX)),
            rectSize.left,
            rectSize.top,
            rectSize.right,
            rectSize.bottom,
            GetDesktopWindow(),
            NULL,
            hInst,
            (void*)nDate);
			
	sprintf(szWinText, "Countdown [%s]", szName);
	SetWindowText(hwnd, szWinText);

    ShowWindow(hwnd, SW_SHOWNORMAL);
	UpdateWindow(hwnd);

    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg); 
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

