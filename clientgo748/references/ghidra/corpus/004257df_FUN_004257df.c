// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004257df | Name: FUN_004257df


int __thiscall FUN_004257df(int param_1,undefined4 *param_2,undefined4 *param_3)

{
  int iVar1;
  
  *param_2 = 0;
  *param_3 = 0;
  if (*(int *)(param_1 + 0x14) < *(int *)(param_1 + 0x10)) {
    if ((uint)(*(int *)(param_1 + 0x10) - *(int *)(param_1 + 0x14)) < 0x88) {
      iVar1 = 0;
    }
    else {
      iVar1 = *(int *)(param_1 + 8) + *(int *)(param_1 + 0x14);
      *(int *)(param_1 + 0x14) = *(int *)(param_1 + 0x14) + 0x88;
      if (*(int *)(param_1 + 0x10) <= *(int *)(param_1 + 0x14)) {
        *(undefined4 *)(param_1 + 0x10) = 0;
        *(undefined4 *)(param_1 + 0x14) = 0;
      }
    }
  }
  else {
    *(undefined4 *)(param_1 + 0x10) = 0;
    *(undefined4 *)(param_1 + 0x14) = 0;
    iVar1 = 0;
  }
  return iVar1;
}

