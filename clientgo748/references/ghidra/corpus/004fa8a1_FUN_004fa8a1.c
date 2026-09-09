// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004fa8a1 | Name: FUN_004fa8a1


void __thiscall FUN_004fa8a1(int param_1,int param_2)

{
  int iVar1;
  
  if ((*(char *)(param_1 + 0x79c) == '\x01') && (param_2 == 0)) {
    if (*(int *)(DAT_0067cf38 + 0x4c) == param_1) {
      FUN_013d3c00();
      return;
    }
  }
  else {
    iVar1 = FUN_0054bf7e(param_1 + 0x444);
    *(float *)(param_1 + 0x220) = (float)iVar1;
    if (*(int *)(DAT_0067cf38 + 0x4c) == param_1) {
      DAT_005bb418 = __ftol();
    }
  }
  return;
}

