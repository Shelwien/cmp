
uint* cache_ptr[pal_MAX][256];

void SymbOut_Init( void ) {
  bzero( cache_ptr );
}

//      SymbOut( ch1, &bm[PY*bmX+PX+i*ch1.wmax],bmX, _s[i], i<9?pal_Addr: (((i-9)/3)%13==0)?pal_Diff:pal_Hex );
//void SymbOut( myfont ch1, uint* outp,uint tx, uint chridx, uint palidx ) {

template< class myfont, class mybitmap >
void SymbOut( myfont& ch1, mybitmap& bm1,uint PX,uint PY, word cp ) {
  uint i,j,k;

  uint chridx=byte(cp), palidx=cp>>8;

  uint tx = bm1.bmX;
  uint* outp = &bm1.pixel(PX,PY); // &((uint*)bm1.bitmap)[PY*bm1.bmX+PX];

  uint* fontbuf = ch1.fontbuf1;
  uint sx=ch1.wmax;
  uint sy=ch1.hmax;

  uint*& pcache = cache_ptr[palidx][chridx];

  if( pcache ) {

    uint* __restrict p = pcache;
    uint* __restrict o = outp;
    __assume_aligned(p,4);
    __assume_aligned(o,4);
    pcache = outp;
    for( k=0; k<sy; k++ ) {
      for( i=0; i<sx; i++ ) o[i]=p[i];
      p += tx;
      o += tx;
    }

  } else {

    byte* __restrict p = (byte*)&fontbuf[chridx*sy*sx];;
    byte* __restrict o = (byte*)outp;
    pcache = outp;
    ALIGN(16) byte q[4]; 
    ALIGN(16) byte r[4];
    __assume_aligned(p,4);
    __assume_aligned(o,4);

    for( i=0; i<sizeof(q); i+=4 ) (uint&)q[i]=palette[palidx].fg, (uint&)r[i]=palette[palidx].bk;

    for( k=0; k<sy; k++ ) {
      for( i=0; i<sx*4; i+=sizeof(q) ) {
        for( j=0; j<3; j++ ) {
          word x = r[j]*255 + (int(q[j])-int(r[j]))*p[i+j];
          o[i+j] = (x+255)>>8;
        }
      }
      p += sx*4;
      o += tx*4;
    }

  }

}
