
struct textblock {
  uint  WCX,WCY; // window sizes in chars

  uint  WPX,WPY; // position on screen
  uint  WSX,WSY; // x/y size in pixels

  uint  blksize;
  word* text; // window text buf

  void Clear( void ) {
    uint i;
    if( text ) for( i=0; i<blksize; i++ ) text[i]=0;
  }

  template< class myfont >
  void Init( myfont& ch1, uint CX, uint CY, uint PX, uint PY ) {
    WCX=CX; WCY=CY; blksize=WCY*WCX;

    WPX=PX; WPY=PY;

    WSX = WCX*ch1.wmax;
    WSY = WCY*ch1.hmax;

    text = new word[WCY*WCX];
  }

  void Quit( void ) {
    delete text;
  }

  word& cell( uint x, uint y ) { 
    return (word&)text[y*WCX+x]; 
  }

  word ch( byte c, byte attr ) {
    return c | (attr<<8);
  }

  void Fill( uint c, uint atr ) {
    uint i;
    for( i=0; i<blksize; i++ ) text[i]=ch(c,atr);
  }

  // print text to textblock
  void Print( char* s, uint atr1, uint atr2=0, uint X=0, uint Y=0 ) {
    uint c,a=atr1,x=X,y=Y;
    while( c=*s++ ) {
      if( c=='\n' ) { x=X; y++; continue; }
      if( c=='~' ) { a^=atr1^atr2; continue; }
      if( (x<WCX) && (y<WCY) ) cell(x,y) = ch(c,a);
      x++;
    }
  }

  void textsize( char* s, uint* tb_SX, uint* tb_SY ) {
    uint c,x=0,y=0;
    while( c=*s++ ) {
      if( c=='\n' ) { x=0; y++; continue; }
      if( c=='~' ) continue;
      x++;
    }
    if(tb_SX) *tb_SX=x;
    if(tb_SY) *tb_SY=y;
  }

  // print textblock to bitmap
  template< class myfont, class mybitmap >
  void Print( myfont& ch1, mybitmap& bm1 ) {
    uint i,j;
    word* s = text;
    uint PX=WPX, PY=WPY;

    SymbOut_Init(); // per screen

    for( j=0; j<WCY; j++ ) {
      for( i=0; i<WCX; i++ ) SymbOut( ch1, bm1,PX+i*ch1.wmax,PY, s[i] );
      PY+=ch1.hmax;
      s+=WCX;
    }
  }

#include "textprint.inc"

};
