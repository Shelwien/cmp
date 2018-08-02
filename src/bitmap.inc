
HBITMAP AllocBitmap( HDC dibDC, byte*& bitmap, uint bmX, uint bmY ) {
//  bmX = WX+100; bmY = WY+100;
  BITMAPINFO bm = { sizeof(BITMAPINFOHEADER), int(bmX), -int(bmY), 1, 32, BI_RGB, 0, 0, 0, 0, 0 };
  HBITMAP dib = CreateDIBSection( dibDC, &bm, DIB_RGB_COLORS, (void**)&bitmap, 0,0 );
  SelectObject( dibDC, dib );

//  HRGN hrgn = CreateRectRgn( 0+0,0+0,bmX-0,bmY-0 );
//  HRGN hrgn = CreateRectRgn( 0-1,0-1,bmX-0,bmY-0 );
//  SelectClipRgn( dibDC, hrgn );
//  DeleteObject( hrgn );

  return dib;
}

struct mybitmap {

  uint bmX, bmY; // bitmap's width and height
  byte* bitmap; // window DIB
  HBITMAP dib;

  void AllocBitmap( HDC dibDC, uint _bmX, uint _bmY ) {
    bitmap = 0;
    bmX = _bmX; bmY = _bmY;
    dib = ::AllocBitmap( dibDC, bitmap, bmX, bmY );
  }

  void Reset() {
    memset( bitmap, 0, 4*bmX*bmY );
  }

  uint& pixel( uint x, uint y ) { 
    return (uint&)bitmap[(y*bmX+x)*4]; 
  }

};
