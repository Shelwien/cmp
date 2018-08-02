
//LOGFONT lf = {-19, 0, 0, 0, 400, 0, 0, 0, 255, 1, 2, 1, 49, "Terminal"};
//LOGFONT lf = {-19,-10, 0, 0, 400, 0, 0, 0, 204, 3, 2, 1, 49, "Consolas"};

struct viewstate {
  LOGFONT lf;
  uint BX,BY;
  int  cur_view;
  uint f_addr64;
  uint f_help;
};

viewstate lf = { {-19,-10, 0, 0, 400, 0, 0, 0, 204, 3, 2, 1, 49, "Consolas"}, 32,255, -1, 0, 0 };
viewstate lf_old;

void SaveConfig( ) {
  uint r,disp;
  HKEY hk;
  r = RegCreateKeyEx( 
    HKEY_CURRENT_USER, 
    "Software\\SRC\\cmp_01", 0, 0,
    REG_OPTION_NON_VOLATILE,
    KEY_ALL_ACCESS,
    0, // security
    &hk, (LPDWORD)&disp
  );
  if( r==ERROR_SUCCESS ) {
    r = RegSetValueEx( hk, "config", 0, REG_BINARY, (byte*)&lf, sizeof(lf) );
    RegCloseKey( hk );
  } else {
    SetLastError(r);
    printf( "r=%X disp=%X\nError=<%s>\n", r, disp, GetErrorText() );
  }
}

void LoadConfig( ) {
  uint r,l=sizeof(lf);
  HKEY hk;
  r = RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\SRC\\cmp_01", 0, KEY_READ, &hk );
  if( r==ERROR_SUCCESS ) {
    r = RegQueryValueEx( hk, "config", 0, 0, (byte*)&lf, (LPDWORD)&l );
    RegCloseKey( hk );
  } else {
    SetLastError(r);
    printf( "r=%X\nConfig Error=<%s>\n", r, GetErrorText() );
  }
}
