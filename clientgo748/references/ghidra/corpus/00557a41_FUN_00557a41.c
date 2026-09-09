// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00557a41 | Name: FUN_00557a41


undefined4 FUN_00557a41(undefined4 param_1,int param_2,undefined4 param_3)

{
  undefined4 uVar1;
  uint local_10;
  int local_c;
  int local_8;
  
  local_10 = 0;
  local_c = 0;
  local_c = InternetOpenA(&DAT_005c483c,0,0,0,0);
  if (local_c == 0) {
    uVar1 = 0;
  }
  else {
    local_8 = InternetOpenUrlA(local_c,param_1,0,0,0x4000000,0);
    if (local_8 == 0) {
      GetLastError();
      InternetCloseHandle(local_c);
      uVar1 = 0;
    }
    else {
      InternetReadFile(local_8,param_2,param_3,&local_10);
      InternetCloseHandle(local_8);
      if (0x3ff < local_10) {
        local_10 = 0x3ff;
      }
      *(undefined1 *)(param_2 + local_10) = 0;
      InternetCloseHandle(local_c);
      uVar1 = 1;
    }
  }
  return uVar1;
}

