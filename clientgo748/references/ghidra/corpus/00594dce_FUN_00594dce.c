// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00594dce | Name: FUN_00594dce


undefined4 FUN_00594dce(uint param_1)

{
  int *piVar1;
  undefined4 *puVar2;
  int iVar3;
  DWORD nStdHandle;
  
  if (param_1 < DAT_013b8720) {
    iVar3 = (param_1 & 0x1f) * 0x24;
    piVar1 = (int *)((&DAT_013b8620)[(int)param_1 >> 5] + iVar3);
    if (((*(byte *)(piVar1 + 1) & 1) != 0) && (*piVar1 != -1)) {
      if (DAT_005c9c04 == 1) {
        if (param_1 == 0) {
          nStdHandle = 0xfffffff6;
        }
        else if (param_1 == 1) {
          nStdHandle = 0xfffffff5;
        }
        else {
          if (param_1 != 2) goto LAB_00594e2a;
          nStdHandle = 0xfffffff4;
        }
        SetStdHandle(nStdHandle,(HANDLE)0x0);
      }
LAB_00594e2a:
      *(undefined4 *)((&DAT_013b8620)[(int)param_1 >> 5] + iVar3) = 0xffffffff;
      return 0;
    }
  }
  puVar2 = (undefined4 *)FUN_00594c1d();
  *puVar2 = 9;
  puVar2 = (undefined4 *)FUN_00594c26();
  *puVar2 = 0;
  return 0xffffffff;
}

