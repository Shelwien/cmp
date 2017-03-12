
struct myfont {
  HFONT hFont;
  LOGFONT lf;
  uint* fontbuf1;
  int wmin,wmax,hmin,hmax;
  int wbuf[256];
  int hbuf[256];

  void InitFont( void ) {
    hFont=0;
    bzero(lf);
  }

  void Quit( void ) {
    delete fontbuf1;
    DeleteObject( hFont ); hFont=0;
  }

  void DumpLF() {
    uint* x= (uint*)&lf;
    byte* y= (byte*)&lf.lfItalic; 
    uint i;
    printf( "{ " );
    for( i=0; i<5; i++ ) printf( "%i,", x[i] ); printf( "  " );
    for( i=0; i<8; i++ ) printf( "%i,", y[i] ); printf( "  " );
    printf( "\"%s\" }\n", lf.lfFaceName );
  }

  void SelectFont( void ) {
    CHOOSEFONT cf;
    bzero(cf); //ZeroMemory( &cf, sizeof(cf) );
    cf.lStructSize = sizeof(cf);
    cf.hwndOwner = 0;//win; //Globals.hMainWnd;
    cf.lpLogFont = &lf;
    cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_FIXEDPITCHONLY | CF_INITTOLOGFONTSTRUCT | CF_NOVERTFONTS;
    if( ChooseFont(&cf) ) DumpLF();
  }

  void GetFontWidth( HDC dibDC ) {
    char s[2] = {0,0}; SIZE cs;
    int i;
    for( i=0; i<256; i++ ) wbuf[i]=0;

    for( i=0; i<256; i++ ) {
      s[0]=i; cs.cx=cs.cy=0;
      GetTextExtentPoint32( dibDC, s,1, &cs );
      wbuf[i] = cs.cx;
      hbuf[i] = cs.cy;
    }

  //  GetCharWidth32( dibDC, 0, 255, wbuf );

    wmin=wbuf[0]; hmin=hbuf[0];
    wmax=0;       hmax=0;
    for( i=0; i<256; i++ ) {
      wmin=Min(wmin,wbuf[i]), wmax=Max(wmax,wbuf[i]);
      hmin=Min(hmin,hbuf[i]), hmax=Max(hmax,hbuf[i]);
    }

    printf( "wmin=%i wmax=%i\n", wmin,wmax );
    printf( "hmin=%i hmax=%i\n", hmin,hmax );
/*
    for( i=0; i<256; i++ ) {
      printf( "%2i ", wbuf[i] ); 
      if( i%16==15 ) printf( "\n" );
    }
*/
  }

  void SetFont( HDC dibDC, LOGFONT& _lf ) {
    HFONT currfont = hFont;
    lf = _lf;
    hFont = CreateFontIndirect( &lf );
    if( currfont!=0 ) DeleteObject( currfont );
    SelectObject( dibDC, hFont );

    byte* bitmap = 0;

    GetFontWidth(dibDC);

    HDC DC0 = GetDC(0);
    HDC DC = CreateCompatibleDC( DC0 );  // DC=CreateDC( "DISPLAY",0,0, 0 );

    HBITMAP dib = AllocBitmap( DC, bitmap, wmax, hmax );
    if( bitmap ) {
      SelectObject( DC, hFont );
      PrecalcFont( DC, bitmap, wmax );
      DeleteObject(dib);
    }

    ReleaseDC(0,DC0);
    DeleteDC(DC);
  }

  void PrecalcFont( HDC dibDC, byte* bitmap, uint bmX ) {
    uint c,x,y,i,j; char s[2];
    SetBkMode(dibDC, TRANSPARENT);
    SetTextAlign( dibDC, TA_TOP|TA_LEFT );
    SetBkColor(dibDC,   0x000000 );
    SetTextColor(dibDC, 0xFFFFFF );

    uint* fontbuf = new uint[ 256*wmax*hmax ];
    if( fontbuf ) {
      HBRUSH hbr = CreateSolidBrush(0x000000);
      RECT rect = { 0,0,wmax,hmax };

      for( c=0; c<256; c++ ) {

        FillRect(dibDC, &rect, hbr ); 

        s[0]=c; s[1]=0;
        TextOut( dibDC, 0, 0, s, 1 ); 

        for( y=0; y<hmax; y++ ) {
          for( x=0; x<wmax; x++ ) {
            i = (y*bmX + x)*4;
            fontbuf[(c*hmax+y)*wmax+x] = (uint&)bitmap[i];
          } // x
        } // j
      }

      DeleteObject( hbr );
    }

    fontbuf1 = fontbuf;
  }



};


//LOGFONT lf = {-19, 0, 0, 0, 400, 0, 0, 0, 255, 1, 2, 1, 49, "Terminal"};
//LOGFONT lf = {-19,-10, 0, 0, 400, 0, 0, 0, 204, 3, 2, 1, 49, "Consolas"};
//LOGFONT lf_old;










