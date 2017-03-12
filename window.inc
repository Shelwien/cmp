
HWND win;

//HDC  DC; // window's DC
HDC  dibDC;  // bitmap device-context, can be used for any GDI output
PAINTSTRUCT ps;

//LOGBRUSH lb; 
//HGDIOBJ hPen, hPenOld;

void DrawBox( HDC dc, int x1,int y1, int x2,int y2, int dx=0 ) {
  dx %= (x2-x1);
  POINT a[] = { { x1+dx,y1 }, { x2,y1 }, { x2,y2 }, { x1,y2 }, { x1,y1 }, { x1+dx,y1 } };
  Polyline( dc, a, 6 );
}

void DrawLine( HDC dc, HGDIOBJ& hPen, int x1,int y1, int x2,int y2 ) {
  POINT a[] = { { x1,y1 }, { x2,y2 } };
  HGDIOBJ hPenOld = SelectObject( dibDC, hPen );
  Polyline( dc, a, 2 );
  SelectObject( dibDC, hPenOld );
}


void DisplayRedraw( void ) {
  InvalidateRect( win, 0, 0 );
}

void DisplayRedraw( int x, int y, int dx, int dy ) {
  RECT v;
  v.left   = x;
  v.top    = y;
  v.right  = x+dx;
  v.bottom = y+dy;
  InvalidateRect( win, &v, 0 );
}

void ShiftWindow( int x, int y, uint mode=3 ) {
  WINDOWPLACEMENT winpl = { sizeof(WINDOWPLACEMENT) };
  GetWindowPlacement( win, &winpl );
  winpl.rcNormalPosition.left  += (mode&1) ? x : 0;
  winpl.rcNormalPosition.top   += (mode&1) ? y : 0;
  winpl.rcNormalPosition.right += (mode&2) ? x : 0;
  winpl.rcNormalPosition.bottom+= (mode&2) ? y : 0;
  winpl.showCmd = SW_SHOW;
  SetWindowPlacement( win, &winpl );
}

void SetWindow( HWND win, int x, int y, int w, int h ) {
  WINDOWPLACEMENT winpl = { sizeof(WINDOWPLACEMENT) };
  winpl.rcNormalPosition.left  = x;
  winpl.rcNormalPosition.top   = y;
  winpl.rcNormalPosition.right = x+w;
  winpl.rcNormalPosition.bottom= x+h;
  winpl.showCmd = SW_SHOW;
  SetWindowPlacement( win, &winpl );
}

void SetWindowSize( HWND win, int w, int h ) {
  int x,y;
  WINDOWPLACEMENT winpl = { sizeof(WINDOWPLACEMENT) };
  GetWindowPlacement( win, &winpl );
  x = winpl.rcNormalPosition.left;
  y = winpl.rcNormalPosition.top;
  winpl.rcNormalPosition.right = x+w;
  winpl.rcNormalPosition.bottom= x+h;
  winpl.showCmd = SW_SHOW;
  SetWindowPlacement( win, &winpl );
}

