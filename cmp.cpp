
#pragma comment(lib,"icnewfeat.lib")
//extern "C" void __cdecl __intel_new_feature_proc_init( void ) {}
//extern "C" int __cdecl strcat_s( char * _Dst, size_t _SizeInBytes, const char * _Src ) { return 0; }

#include <windows.h>
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"advapi32.lib")
#pragma comment(lib,"comdlg32.lib") // for choosefont

#include "common.inc"
#include "file_api.inc"
#include "rdtsc.inc"
#include "thread.inc"

#include "bitmap.inc"
#include "setfont.inc"
#include "palette.inc"
#include "textblock.inc"
#include "hexdump.inc"

#include "window.inc"

#include "config.inc"

#include "help.inc"

enum{ N_VIEWS=8 };

MSG  msg;
uint lastkey = 0;
uint curtim, lasttim = 0;

mybitmap  bm1;
myfont    ch1;
hexfile   F[N_VIEWS];
textblock tb[N_VIEWS];
uint F_num;

textblock tb_help;
uint help_SY;

uint f_started = 0;
volatile uint f_busy = 0;

struct DiffScan : thread<DiffScan> {

  typedef thread<DiffScan> base;

  uint start( void ) {
    f_busy=1;
    return base::start();
  }

  void thread( void ) {
    uint c,i,j,d,x[N_VIEWS],ff_num,delta,flag;
    qword pos_delta=0;

    for(i=0;i<F_num;i++) F[i].MoveFilepos(F[0].textlen);

    while( f_busy ) {
      delta=0;
      for( j=0; j<F[0].textlen; j++ ) {
        c=0; d=-1; ff_num=0; 
        for(i=0;i<F_num;i++) {
          x[i] = F[i].viewdata(j);
          ff_num += (x[i]==-1);
          if( x[i]!=-1 ) c|=x[i],d&=x[i];
        }
        for(flag=1,i=0;i<F_num;i++) flag &= ((c==x[i])&&(d==x[i]));
        delta += flag;
      }

      if( ff_num>=F_num ) break;
      pos_delta += delta;
      if( delta<F[0].textlen ) break;
      for(i=0;i<F_num;i++) F[i].MoveFilepos(delta);
    }

    f_busy=0;
    DisplayRedraw();
//printf( "thread stop!\n" );
    return;
  }

};

DiffScan diffscan;

HINSTANCE g_hInstance = GetModuleHandle(0);

LRESULT CALLBACK wndproc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
  return DefWindowProc(hWnd, message, wParam, lParam);
}

WNDCLASS wndclass = {
  0, // style
  wndproc,
  0,0, // clsextra, wndextra
  g_hInstance, // hinstance
  0,0, // icon,cursor
  0, // background
  0, // menu
  "wndclass"
};


int __stdcall WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ) {
  int argc=__argc; char **argv=__argv;

//int main( int argc, char **argv ) {

  uint c,i,j,l;

  char* fil1 = argv[0]; //if( argc>1 ) fil1=argv[1];
//  char* fil2 = argv[0]; if( argc>2 ) fil2=argv[2], F_num++;

//  if( F[0].Open(fil1)==0 ) return 1;
//  if( (F_num>1) && (F[1].Open(fil2)==0) ) return 1;

  for( i=1; i<Min(DIM(F)+1,Max(2,argc)); i++ ) {
    if( i<argc ) fil1=argv[i];
    if( F[i-1].Open(fil1)==0 ) return 1;
    if( F[i-1].F1size>0xFFFFFFFFU ) lf.f_addr64=hexfile::f_addr64;
  } 
  F_num = i-1;

//  if( lf.lf.lfWidth==0 ) lf.lf.lfWidth = -ch1.wmax;

//  lf.BX = Max(1U,2*lf.BX/F_num);

  LoadConfig();
  tb_help.textsize( helptext, 0, &help_SY );

  const int wfr_x = GetSystemMetrics(SM_CXFIXEDFRAME);
  const int wfr_y = GetSystemMetrics(SM_CYFIXEDFRAME);
  const int wfr_c = GetSystemMetrics(SM_CYCAPTION);
  RECT scr; SystemParametersInfo( SPI_GETWORKAREA,0,&scr,0 );
  const int scr_w = scr.right-scr.left;
  const int scr_h = scr.bottom-scr.top;
//printf( "w=%i h=%i %i %i %i %i\n", scr_w,scr_h, scr.left,scr.top, scr.right,scr.bottom );
//  ch1.SelectFont();

  dibDC = CreateCompatibleDC( GetDC(0) );  // DC=CreateDC( "DISPLAY",0,0, 0 );
  bm1.AllocBitmap( dibDC, scr_w, scr_h );

  RegisterClass( &wndclass );

  win = CreateWindow(
   "wndclass",             // class
    "File Comparator  (F1=help)",   // title
    WS_SYSMENU/*0x90C0*/,         // style
//    0,0,                 // x,y
    scr.left,scr.top,                 // x,y
//    WX+2*wfr_x,WY+2*wfr_y+wfr_c, // w,h
    scr_w,scr_h, // w,h
    0,                   // parent
    0,                   // menu
    HINSTANCE(0x400000), // hInstance,
    0                    // creation data
  );
//  GetWindowPlacement( win, &winpl );

  HICON ico = LoadIcon( HINSTANCE(0x400000), MAKEINTRESOURCE(500) );
  SetClassLong( win, GCL_HICON, LONG(ico) );
  DeleteObject(ico);

  LOGBRUSH lb; 
  HGDIOBJ hPen, hPenOld, hPen_help;
  lb.lbStyle = BS_SOLID; 
  lb.lbColor = RGB(0,0xFF,0);
  lb.lbHatch = HS_CROSS; 
  hPen = ExtCreatePen( PS_GEOMETRIC|PS_DASH, 5, &lb, 0, NULL ); 
  uint pen_shift = 0;

  lb.lbStyle = BS_SOLID; 
  lb.lbColor = RGB(0xFF,0xFF,0xFF);
  lb.lbHatch = HS_HORIZONTAL; 
  hPen_help = ExtCreatePen( PS_GEOMETRIC, 2, &lb, 0, NULL ); 




  int delta;
  int rp1,rp,alt,ctr;
  uint mBX,mBY;
  int  WX,WY; // window sizes

  if(0) {
    Restart:
    for(i=0;i<F_num;i++ ) {
      tb[i].Quit();
      F[i].Quit();
    }
    ch1.Quit();
  }

  memcpy( &ch1.lf, &lf.lf, sizeof(ch1.lf) );
  ch1.DumpLF();
  
  ch1.InitFont();
  ch1.SetFont(dibDC,lf.lf);

  mBX=0; mBY=0;
  for( j=15; j!=-1; j-- ) {
    mBX |= (1<<j);
    mBY |= (1<<j);

    WX = 2*wfr_x;
    for(i=0;i<F_num;i++) WX += F[i].Calc_WCX( mBX, lf.f_addr64, (i!=F_num-1) ) * ch1.wmax;
    WY = mBY*ch1.hmax;
    if( lf.f_help ) {
      WY += help_SY*ch1.hmax; 
    }
    WY+= 2*wfr_y+wfr_c;

    if( WX>bm1.bmX ) mBX&=~(1<<j);
    if( WY>bm1.bmY ) mBY&=~(1<<j);
  }
  mBX = Max(1U,mBX);
  mBY = Max(1U,mBY);
  mBX = Min(mBX,lf.BX);
  if( f_started==0 ) lf.BX=mBX;
  if( lf.BY>mBY ) lf.BY=mBY;
  if( lf.BX*lf.BY>hexfile::datalen-hexfile::datalign ) lf.BX=(hexfile::datalen-hexfile::datalign)/lf.BY;

printf( "mBX=%i mBY=%i BX=%i BY=%i\n", mBX, mBY, lf.BX, lf.BY );

  WX=2*wfr_x;
  for(i=0;i<F_num;i++ ) {
    uint WCX = F[i].Calc_WCX( mBX, lf.f_addr64, (i!=F_num-1) );
    tb[i].Init( ch1, WCX,lf.BY, WX,0 );
    WX += WCX*ch1.wmax;
    F[i].SetTextbuf( tb[i], lf.BX, ((i!=F_num-1)?hexfile::f_vertline:0) | lf.f_addr64);
    F[i].SetFilepos( F[i].F1pos );
  }
  WY = tb[0].WCY*ch1.hmax;
  if( lf.f_help ) {
    tb_help.Init( ch1, WX/ch1.wmax,help_SY, 0,WY );
    tb_help.Fill( ' ', pal_Help1 );
    tb_help.Print( helptext, pal_Help1,pal_Help2, 1,0 );
    WY += help_SY*ch1.hmax; 
  }
  WY += 2*wfr_y+wfr_c;


printf( "WX=%i WY=%i bmX=%i bmY=%i\n", WX,WY,bm1.bmX,bm1.bmY );

/*
  if( (WX>bm1.bmX) || (WY>bm1.bmY) ) { 
    if( f_started==0 ) {
      if( lf.lf.lfHeight<-1 ) { lf.lf.lfWidth=0; lf.lf.lfHeight++; goto Restart; } else exit(1);
    }
    lf=lf_old; goto Restart; 
  }
*/

  SetWindowSize( win, WX,WY );
//  SetWindowSize( win, WX+2*wfr_x,WY+2*wfr_y+wfr_c );

  SetFocus( win );
  ShowWindow( win, SW_SHOW );
  SetTimer( win, 0, 100, 0 );
  DisplayRedraw();
  f_started=1;
  lf_old = lf;

  while( GetMessage(&msg,win,0,0) ) {

    if(0) { m_break: break; }

    switch( msg.message ) {

    case WM_MOUSEWHEEL:
      delta = (short)HIWORD(msg.wParam);
      delta = (delta<0) ? 9 : 8; goto MovePos;

    case WM_CLOSE: case WM_NULL:
      goto m_break;

    case WM_TIMER: 
      DisplayRedraw();
      break;

    case WM_PAINT:

      if( f_busy==0 ) {

        bm1.Reset();
//        if( F_num==2 ) for(i=0;i<F_num;i++) F[i].Compare(F[(i+1)%F_num]);
        {
          uint d,x[N_VIEWS];
          for( j=0; j<F[0].textlen; j++ ) {
            c=0; d=-1;
            for(i=0;i<F_num;i++) {
              x[i] = F[i].viewdata(j);
              if( x[i]!=-1 ) // do we count symbols after eof as diff?
              c|=x[i],d&=x[i];
            }
            for(i=0;i<F_num;i++) F[i].diffbuf[j]=((c!=x[i])||(d!=x[i]));
          }
        }

        for(i=0;i<F_num;i++) F[i].hexdump(tb[i]);
//lasttsc = rdtsc();
        for(i=0;i<F_num;i++) tb[i].Print(ch1,bm1);

        if( lf.f_help ) {
          DrawLine(dibDC,hPen_help, tb_help.WPX,tb_help.WPY, tb_help.WPX+tb_help.WSX,tb_help.WPY );
          tb_help.Print(ch1,bm1);
        }

        if( (lf.cur_view>=0) && (lf.cur_view<F_num) ) {
          i = lf.cur_view;
          hPenOld = SelectObject( dibDC, hPen );
          DrawBox( dibDC, tb[i].WPX-2,tb[i].WPY, tb[i].WPX-2+tb[i].WSX-5,tb[i].WPY+tb[i].WSY-5, pen_shift );
          SelectObject( dibDC, hPenOld );
          pen_shift++;
        }

//curtsc = rdtsc()-lasttsc; static FILE* dbg = fopen( "tsc.txt", "wb" ); fprintf(dbg, "%08I64X\n", curtsc ); fflush(dbg);
      }

      {
        HDC DC = BeginPaint( win, &ps );
        BitBlt( DC, 0,0, WX,WY,  dibDC, 0,0, SRCCOPY );
        EndPaint( win, &ps );
      }
      break;

    case WM_KEYDOWN: case WM_SYSKEYDOWN:
      curtim = GetTickCount();
      {
        rp1 = (msg.wParam==lastkey) && (curtim>lasttim+1000);
        rp  = ((msg.lParam>>30)&1) ? 4*(rp1+1) : 1; // key repeat flag
        alt = ((msg.lParam>>29)&1);
        ctr = (GetKeyState(VK_CONTROL)>>31)&1;
        c = msg.wParam;
        if( (c>='0') && (c<='9') ) c='0';

printf( "WM_KEYDOWN: w=%04X l=%04X c=%X alt=%i ctr=%i rp1=%i f_busy=%X\n", msg.wParam, msg.lParam, c,alt,ctr,rp1, f_busy );
        if( f_busy ) { f_busy=0; diffscan.quit(); }

        switch( c ) {
          case 0x001B: exit(0);

          case 0x0070: // F1=help
            lf.f_help ^= 1;
            goto Restart;

          case '\t': 
            lf.cur_view = ((lf.cur_view+1+1) % (F_num+1)) -1;
            goto Redraw;

          case ' ': case 0x0075/*F6*/: 
            diffscan.start();
            break;

          case 'R': // 'R' for Reload
            for(j=0;j<F_num;j++) {
              i = (lf.cur_view==-1) ? j : lf.cur_view;
              F[i].databeg=F[i].dataend=0;
            }
            goto MovePos0;

          case 'C': // 'C' for SelectFont
            ch1.SelectFont();
            lf.lf=ch1.lf;
            goto Restart;

          case 'X': // 'X' for 32/64-bit switch
            lf.f_addr64 ^= hexfile::f_addr64;
            goto Restart;

          case 'S': // save config;
            SaveConfig();
            goto Restart;

          case 'L': // save config;
            LoadConfig();
            goto Restart;

          case 0x00BD: case 0x006D: // '-',Gray-
            if( ( ctr) || (!alt) ) if( lf.lf.lfHeight<-1 ) lf.lf.lfHeight++;
            if( (!ctr) || ( alt) ) if( lf.lf.lfWidth<-1 ) lf.lf.lfWidth++; 
            goto Restart;

          case 0x00BB: case 0x006B: // '+',Gray+
            if( ( ctr) || (!alt) ) lf.lf.lfHeight--; 
            if( (!ctr) || ( alt) ) lf.lf.lfWidth--; 
            goto Restart;

          case 0x0025: // left
            if( ctr ) { if( lf.BX>1 ) lf.BX--; goto Restart; }
            delta=2; goto MovePos;
//            break;

          case 0x0027: // right
            if( ctr ) { lf.BX++; goto Restart; }
            delta=3; goto MovePos;
//            break;

          case 0x0026: // up
            if( ctr ) { if( lf.BY>1 ) lf.BY--; goto Restart; }
            delta=4; goto MovePos;

          case 0x0028: // down
            if( ctr ) { lf.BY++; goto Restart; }
            delta=5; goto MovePos;

          case 0x0021: // pgup
            delta=6; goto MovePos;

          case 0x0022: // pgup
            delta=7; goto MovePos;

          case 0x0023: // end
            delta=1; goto MovePos;

          case 0x0024: // home
MovePos0:
            delta=0;
            MovePos:

            if( lf.cur_view==-1 ) {
              for(i=0;i<F_num;i++) F[i].MovePos(delta);
            } else {
              F[lf.cur_view].MovePos(delta);
            }
Redraw:
            DisplayRedraw(); break;

          default:;
        }
      }

      if( lastkey!=msg.wParam ) lasttim = curtim;
      lastkey = msg.wParam;
      break;

    default:
if( msg.message!=WM_MOUSEMOVE ) printf( "msg=%X\n", msg.message );
      DispatchMessage(&msg);
    }
  }


  return 0;
}

