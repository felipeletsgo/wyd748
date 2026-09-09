// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042b510 | Name: FUN_0042b510


int __thiscall FUN_0042b510(int *param_1,LPSTR param_2,undefined4 param_3,int param_4)

{
  HMMIO pHVar1;
  HRSRC local_64;
  _MMIOINFO local_60;
  uint local_18;
  LPVOID local_14;
  HPSTR local_10;
  HGLOBAL local_c;
  int local_8;
  
  param_1[0x1f] = param_4;
  param_1[0x20] = 0;
  if (param_1[0x1f] == 1) {
    if (param_2 == (LPSTR)0x0) {
      local_8 = -0x7ff8ffa9;
    }
    else {
      if (*param_1 != 0) {
        FUN_0058ea3a(*param_1);
        *param_1 = 0;
      }
      pHVar1 = mmioOpenA(param_2,(LPMMIOINFO)0x0,0x10000);
      param_1[1] = (int)pHVar1;
      if (param_1[1] == 0) {
        local_64 = FindResourceA((HMODULE)0x0,param_2,&DAT_005b48dc);
        if ((local_64 == (HRSRC)0x0) &&
           (local_64 = FindResourceA((HMODULE)0x0,param_2,&DAT_005b48e4), local_64 == (HRSRC)0x0)) {
          return -0x7fffbffb;
        }
        local_c = LoadResource((HMODULE)0x0,local_64);
        if (local_c == (HGLOBAL)0x0) {
          return -0x7fffbffb;
        }
        local_18 = SizeofResource((HMODULE)0x0,local_64);
        if (local_18 == 0) {
          return -0x7fffbffb;
        }
        local_14 = LockResource(local_c);
        if (local_14 == (LPVOID)0x0) {
          return -0x7fffbffb;
        }
        local_10 = operator_new(local_18);
        FUN_0058f220(local_10,local_14,local_18);
        _memset(&local_60,0,0x48);
        local_60.fccIOProc = 0x204d454d;
        local_60.cchBuffer = local_18;
        local_60.pchBuffer = local_10;
        pHVar1 = mmioOpenA((LPSTR)0x0,&local_60,0x10000);
        param_1[1] = (int)pHVar1;
      }
      local_8 = FUN_0042b7a7();
      if (local_8 < 0) {
        mmioClose((HMMIO)param_1[1],0);
      }
      else {
        local_8 = FUN_0042b9e9();
        if (-1 < local_8) {
          param_1[0xc] = param_1[3];
        }
      }
    }
  }
  else {
    pHVar1 = mmioOpenA(param_2,(LPMMIOINFO)0x0,0x11002);
    param_1[1] = (int)pHVar1;
    if (param_1[1] == 0) {
      local_8 = -0x7fffbffb;
    }
    else {
      local_8 = FUN_0042be46(param_3);
      if (local_8 < 0) {
        mmioClose((HMMIO)param_1[1],0);
      }
      else {
        local_8 = FUN_0042b9e9();
      }
    }
  }
  return local_8;
}

