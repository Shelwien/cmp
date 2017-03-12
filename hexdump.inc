
struct hexfile {
  filehandle0 F1;
  qword F1size;

  qword F1pos; // text window position in file

  enum {
    f_vertline=1,
    f_addr64
  };
  uint  flags; // view flags

  uint  BX;    // actual chars-per-line
  uint  F1cpl; // visible chars-per-line
  uint  F1dpl; // unused chars in line
  uint  textlen; // raw bytes in the view
  byte* diffbuf;

  qword viewbeg; // first visible position in databuf
  qword viewend; // end pos of visible part
  enum{ datalen=1<<20, datalign=1<<16 };
//  enum{ datalen=1<<16, datalign=1<<12 };
  qword databeg;
  qword dataend;
  byte  databuf[datalen];

  uint Calc_WCX( uint mBX, uint f_addr64, uint f_vertline ) {
    uint waddr = f_addr64 ? 8+1+8 : 8;
    uint whex = 3*mBX;
    uint wtxt = 1*mBX;
    uint WCX = waddr+1+1+whex+1+wtxt+(f_vertline>0);
    return WCX;
  }

  void SetTextbuf( textblock& tb1, uint _BX, uint _flags=0 ) {
    flags = _flags;
    BX = _BX;

//    tb[i].Init((lf.f_addr64?8+1+8:8)+1+1+(3+1)*lf.BX+2+(i==0),lf.BY, WX,0);

    uint aw = (flags&f_addr64)? 8+1+8 : 8;
    F1cpl = (tb1.WCX-aw-2-1)/(3+1); // chars per line
    F1dpl = (tb1.WCX-aw-2-1)%(3+1);

    textlen = BX*tb1.WCY;
    diffbuf = new byte[textlen];
    if( diffbuf ) bzero( diffbuf, textlen );
  }

  void Quit( void ) {
    delete diffbuf;
  }

  uint viewdata( uint i ) {
    qword ofs = F1pos+i; 
    uint c, f = (ofs>=viewbeg) && (ofs<viewend);
    c = f ? databuf[F1pos-databeg+i] : -1;
    return c;
  }

  void Compare( hexfile& F2 ) {
    uint c1,c2,f1,f2,i;
    qword ofs1,ofs2;

    for( i=0; i<textlen; i++ ) {
      c1 = (*this).viewdata(i);
      c2 = F2.viewdata(i);
//      ofs1 = F1pos+i; 
//      f1 = (ofs1>=viewbeg) && (ofs1<viewend);
//      c1 = f1 ? databuf[F1pos-databeg+i] : -1;

//      ofs2 = F2.F1pos+i;
//      f2 = (ofs2>=F2.viewbeg) && (ofs2<F2.viewend);
//      c2 = f2 ? F2.databuf[F2.F1pos-F2.databeg+i] : -1;

      diffbuf[i] = (c1!=c2);
    }
  }

  void MovePos( uint m_type=0 ) {
//    const int delta[] = { -1,1, -int(F1cpl),int(F1cpl), -int(textlen),int(textlen), -int(F1cpl*4), int(F1cpl*4) };
    const int delta[] = { -1,1, -int(BX),int(BX), -int(textlen),int(textlen), -int(BX*4), int(BX*4) };
    SetFilepos( (m_type==0)? 0 : (m_type==1) ? F1size-textlen : F1pos+sqword(delta[m_type-2]) );
  }

  void MoveFilepos( int newpos=0 ) {
    F1pos += sqword(newpos);
    SetFilepos( F1pos );
  }

  void SetFilepos( qword newpos ) {
    uint len;
    F1pos=newpos;
  
    qword newend = newpos+textlen;

    if( newend>F1size ) newend=F1size;
    if( newend<newpos ) newpos=0; // view overflows, skip to actual start
 
    // [0..F1size) is the file
    // [newpos..newend) has to be cached
    // [databeg..dataend) is already cached
    // let's assume that there won't be any exabyte files, so newpos>=F1size means only part that overflows is visible

    if( (newpos>=databeg) && (newend<=dataend) ) {
      // all necessary data is in buffer already
    } else {

      // have to compute a new cluster-aligned buffer
      databeg = newpos - (newpos % datalign);
      F1.seek( databeg );
      len = F1.read( databuf, datalen );
      dataend = databeg + len;
      if( newend>=dataend ) newend=dataend;

    }

    viewbeg = newpos;
    viewend = newend;
  }

  uint Open( char* fnam ) {
    F1pos=0;
    databeg = dataend=0;
    if( F1.open(fnam) ) {
      F1size = F1.size();
//printf( "F1size=%I64X\n", F1size );
    }
    return uint(F1.f);
  }

  word* HexPrint( word* s, qword x, uint w, uint attr ) {
    uint i,c;
    attr<<=8;
    for( i=w-1; i!=-1; i-- ) {
      c = '0'+((x>>(i*4))&15); if( c>'9' ) c+='A'-('9'+1);
      *s++ = c | attr;
    }
    return s;
  }

//  void hexdump( textblock& tb1, filehandle0 F1,qword F1pos,uint& F1cpl ) {
  void hexdump( textblock& tb1 ) {

    uint c,i,j;
    uint cpl = F1cpl;
    uint dpl = F1dpl;
    uint len = textlen;
    byte* p  = &databuf[F1pos-databeg];

    qword ofs;

    word* s = &tb1.cell(0,0);
    word* _s = s;

    for( j=0; j<tb1.WCY; j++ ) {
      ofs = F1pos+j*BX;

      if( flags&f_addr64 ) {
        s = HexPrint( s, ofs>>32, 8, pal_Addr );
        *s++ = tb1.ch(':',pal_Addr);
      }
      s = HexPrint( s, ofs, 8, pal_Addr );

      *s++ = tb1.ch(':',pal_Addr);
      *s++ = tb1.ch(' ',pal_Hex);

      for( i=0; i<cpl; i++ ) {
        s[2]=s[1]=s[0] = tb1.ch(' ',pal_Hex);
        if( ((ofs+i)>=viewbeg) && ((ofs+i)<viewend) ) {
          HexPrint( s, p[j*BX+i], 2, diffbuf[j*BX+i]?pal_Diff:pal_Hex );
        }
        s+=3;
      }
      if( F1cpl<BX ) s[-1]=tb1.ch('>',pal_Addr);

//      s[1]=s[0] = tb1.ch(' ',pal_Hex); s+=2;
      *s++ = tb1.ch(' ',pal_Hex);

      for( i=0; i<cpl; i++ ) {
        c = tb1.ch(' ',pal_Hex);
        if( ((ofs+i)>=viewbeg) && ((ofs+i)<viewend) ) {
          c = tb1.ch( p[j*BX+i], diffbuf[j*BX+i]?pal_Diff:pal_Hex );
        }
        *s++ = tb1.ch( byte(c),pal_Hex);
      }

      for( i=0; i<dpl; i++ ) *s++=tb1.ch(' ',pal_Hex);

      if( flags & f_vertline ) s[-1]=tb1.ch('|',pal_Sep);

    } // j

  } // end hexdump()


};




