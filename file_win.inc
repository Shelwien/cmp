
enum {
  ffWRITE_THROUGH         =0x80000000,
  ffOVERLAPPED            =0x40000000,
  ffNO_BUFFERING          =0x20000000,
  ffRANDOM_ACCESS         =0x10000000,
  ffSEQUENTIAL_SCAN       =0x08000000,
  ffDELETE_ON_CLOSE       =0x04000000,
  ffBACKUP_SEMANTICS      =0x02000000,
  ffPOSIX_SEMANTICS       =0x01000000,
  ffOPEN_REPARSE_POINT    =0x00200000,
  ffOPEN_NO_RECALL        =0x00100000
};

// Log = Open( PswLog, OPEN_ALWAYS, ffWRITE_THROUGH );
HANDLE Win32_Open( const wchar_t* s, uint Flags, uint Attrs ) {
  return CreateFileW( 
    s,
    GENERIC_READ | GENERIC_WRITE,
    0, //FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
    0, // security
    Flags,
    Attrs,
    0
  );
}

HANDLE file_opendir( const wchar_t* s ) {
  HANDLE r = CreateFileW( 
     s,
     GENERIC_WRITE,
     FILE_SHARE_READ | FILE_SHARE_WRITE, //FILE_SHARE_DELETE,
     0, // security
     OPEN_EXISTING,
     FILE_FLAG_BACKUP_SEMANTICS,
     0
   );
  return r!=INVALID_HANDLE_VALUE ? r : 0;
}

int file_mkdir( const wchar_t* name ) {
  return CreateDirectoryW( name, 0 );
}

uint file_open_mode = GENERIC_READ;
void file_open_mode_r( void ) { file_open_mode = GENERIC_READ; }
void file_open_mode_rw( void ) { file_open_mode = GENERIC_READ | GENERIC_WRITE; }

uint file_make_mode = GENERIC_WRITE;
void file_make_mode_w( void ) { file_make_mode = GENERIC_WRITE; }
void file_make_mode_rw( void ) { file_make_mode = GENERIC_READ | GENERIC_WRITE; }

uint file_make_cmode = CREATE_ALWAYS;
void file_make_open( void ) { file_make_cmode = OPEN_ALWAYS; }
void file_make_create( void ) { file_make_cmode = CREATE_ALWAYS; }

HANDLE file_open( const wchar_t* name ) { 
  HANDLE r = CreateFileW( 
     name,
     file_open_mode,
     FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
     0, // security
     OPEN_EXISTING,
     0,
     0
  );
  return r!=INVALID_HANDLE_VALUE ? r : 0;
}

HANDLE file_open( const char* name ) { 
  HANDLE r = CreateFileA(
     name,
     file_open_mode,
     FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
     0, // security
     OPEN_EXISTING,
     0,
     0
  );
  return r!=INVALID_HANDLE_VALUE ? r : 0;
}

HANDLE file_make( const wchar_t* name ) { 
  HANDLE r = CreateFileW( 
     name,
     file_make_mode,
     FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
     0, // security
     file_make_cmode, //CREATE_ALWAYS,
     0,
     0
  );
  return r!=INVALID_HANDLE_VALUE ? r : 0;
}

HANDLE file_make( const char* name ) { 
  HANDLE r = CreateFileA( 
     name,
     file_make_mode,
     FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
     0, // security
     file_make_cmode, //CREATE_ALWAYS,
     0,
     0
  );
  return r!=INVALID_HANDLE_VALUE ? r : 0;
}

int file_close( HANDLE file ) {
  return CloseHandle( file );
}

uint file_read( HANDLE file, void* _buf, uint len ) {
  byte* buf = (byte*)_buf;
  uint r = 0;
  ReadFile( file, buf, len, (LPDWORD)&r, 0 );
  return r;
}

uint file_sread( HANDLE file, void* _buf, uint len ) {
  byte* buf = (byte*)_buf;
  uint r;
  uint flag=1;
  uint l = 0;

  do {
    r = 0;
    flag = ReadFile( file, buf+l, len-l, (LPDWORD)&r, 0 );
    l += r;
  } while( (r>0) && (l<len) && flag );

  return l;
}

uint file_writ( HANDLE file, void* _buf, uint len ) {
  byte* buf = (byte*)_buf;
  uint r = 0;
  WriteFile( file, buf, len, (LPDWORD)&r, 0 );
  return r;
}

qword file_seek( HANDLE file, qword ofs, int typ = FILE_BEGIN ) {
  uint lo,hi;
  lo = uint(ofs);
  hi = uint(ofs>>32);
  lo = SetFilePointer( file, lo, (PLONG)&hi, typ );
  ofs = hi;
  ofs = (ofs<<32) + lo;
  return ofs;
}

qword file_tell( HANDLE file ) {
  return file_seek( file, 0, FILE_CURRENT );
}

qword getfilesize( HANDLE f ) {
  qword pos=-1LL;
  const uint bufsize=1<<16,bufalign=1<<12;
  byte* _buf = new byte[bufsize+bufalign];
  if( _buf ) {
    byte* buf = _buf + bufalign-(uint(_buf)%bufalign);
    uint bit,len;
    pos = 0;
    for( bit=62; bit!=-1; bit-- ) {
      pos |= 1ULL << bit;
      file_seek(f,pos);
      len = file_read(f,buf,bufsize);
      if( (len!=0) && (len<bufsize) ) { pos+=len; break; }
      if( len>=bufsize ) continue;
      pos &= ~(1ULL<<bit);
    }
    delete[] _buf;
  }
  return pos;
}

qword file_size( HANDLE file ) {
  qword t = file_tell(file);
  qword r = file_seek( file, 0, FILE_END );
  if( uint(r)==0xFFFFFFFF ) {
    // SetFilePointer returns this when there's an error
    if( GetLastError()!=0 ) r = getfilesize(file);
  }
  t = file_seek( file, t );
  return r;
}


char* GetErrorText( void ) { 
  wchar_t* lpMsgBuf;
  DWORD dw = GetLastError(); 
  FormatMessageW(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMsgBuf, 0, NULL 
  );
  static char out[32768];
  int wl=wcslen(lpMsgBuf);
  WideCharToMultiByte( CP_OEMCP, 0, lpMsgBuf,wl+1, out,sizeof(out), 0,0 );
  LocalFree(lpMsgBuf);
  wl = strlen(out);
  for( --wl; wl>=0; wl-- ) if( (byte&)out[wl]<32 ) out[wl]=' ';
  return out;
}

uint ExpandPath( wchar_t* path, wchar_t* w, uint wsize ) {
  wcscpy( w, L"\\\\?\\" );
  GetFullPathNameW( path, wsize/sizeof(wchar_t)-4,&w[4], 0 );
  if( (w[4]=='\\') && (w[5]=='\\') ) wcscpy( &w[0], &w[4] );
  return wcslen(w);
}

