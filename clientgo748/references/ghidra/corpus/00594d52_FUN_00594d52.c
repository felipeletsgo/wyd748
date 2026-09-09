// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00594d52 | Name: FUN_00594d52


undefined4 FUN_00594d52(uint param_1,HANDLE param_2)

{
  undefined4 *puVar1;
  int iVar2;
  DWORD nStdHandle;
  
  if (param_1 < DAT_013b8720) {
    iVar2 = (param_1 & 0x1f) * 0x24;
    if (*(int *)((&DAT_013b8620)[(int)param_1 >> 5] + iVar2) == -1) {
      if (DAT_005c9c04 == 1) {
        if (param_1 == 0) {
          nStdHandle = 0xfffffff6;
        }
        else if (param_1 == 1) {
          nStdHandle = 0xfffffff5;
        }
        else {
          if (param_1 != 2) goto LAB_00594dab;
          nStdHandle = 0xfffffff4;
        }
        SetStdHandle(nStdHandle,param_2);
      }
LAB_00594dab:
      *(HANDLE *)((&DAT_013b8620)[(int)param_1 >> 5] + iVar2) = param_2;
      return 0;
    }
  }
  puVar1 = (undefined4 *)FUN_00594c1d();
  *puVar1 = 9;
  puVar1 = (undefined4 *)FUN_00594c26();
  *puVar1 = 0;
  return 0xffffffff;
}

