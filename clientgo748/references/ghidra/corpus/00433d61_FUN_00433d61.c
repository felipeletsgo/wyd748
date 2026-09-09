// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00433d61 | Name: FUN_00433d61


void FUN_00433d61(void)

{
  int local_418;
  undefined4 local_414;
  int local_410;
  int local_40c;
  undefined1 local_408 [1024];
  undefined4 local_8;
  
  local_8 = 0;
  local_418 = 0;
  local_414 = InternetOpenA(&DAT_005b7884,0,0,0,0);
  local_40c = 0;
  local_40c = FUN_0058f716(s_Guilds_txt_005b788c,&DAT_005b7888);
  local_410 = InternetOpenUrlA(local_414,&DAT_00a3a4f8,0,0,0x4000000,0);
  if ((local_410 == 0) || (local_40c == 0)) {
    GetLastError();
  }
  else {
    _memset(local_408,0,0x400);
    InternetReadFile(local_410,local_408,0x400,&local_418);
    FUN_005904b6(local_408,1,local_418,local_40c);
    while (local_418 != 0) {
      _memset(local_408,0,0x400);
      InternetReadFile(local_410,local_408,0x400,&local_418);
      FUN_005904b6(local_408,1,local_418,local_40c);
    }
    InternetCloseHandle(local_410);
    FUN_0058f62c(local_40c);
    DAT_005be33c = 0xffffffff;
    DAT_005d040c = 2;
    FUN_00590fb3();
  }
  return;
}

