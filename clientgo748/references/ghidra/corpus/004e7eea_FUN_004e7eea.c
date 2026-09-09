// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e7eea | Name: FUN_004e7eea


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004e7eea(int param_1)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0xa0) == 0) {
    uVar1 = 0;
  }
  else if (_DAT_005a439c <= *(float *)(param_1 + 0xb0)) {
    if (*(int *)(param_1 + 0x28) == 1) {
      (**(code **)(**(int **)(param_1 + 0xa0) + 0x24))();
      if (*(int *)(param_1 + 0x9c) != 0) {
        (**(code **)(**(int **)(param_1 + 0x9c) + 0x40))();
      }
    }
    uVar1 = 1;
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}

