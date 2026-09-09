// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00571112 | Name: FUN_00571112


int FUN_00571112(void)

{
  LSTATUS LVar1;
  int iVar2;
  int local_14;
  DWORD local_10 [2];
  HKEY local_8;
  
  LVar1 = RegOpenKeyA((HKEY)0x80000002,"Software\\Microsoft\\Direct3D",&local_8);
  if (LVar1 == 0) {
    local_10[1] = 4;
    LVar1 = RegQueryValueExA(local_8,"DisableMMX",(LPDWORD)0x0,local_10,(LPBYTE)&local_14,
                             local_10 + 1);
    if (((LVar1 == 0) && (local_10[0] == 4)) && (local_14 != 0)) {
      RegCloseKey(local_8);
      DAT_005c6070 = 0;
      return 0;
    }
    RegCloseKey(local_8);
  }
  if (DAT_005c6070 < 0) {
    DAT_005c6070 = 0;
    iVar2 = FUN_005710ed();
    if (iVar2 != 0) {
      DAT_005c6070 = 1;
    }
  }
  return DAT_005c6070;
}

