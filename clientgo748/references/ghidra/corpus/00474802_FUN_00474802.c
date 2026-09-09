// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00474802 | Name: FUN_00474802


void __fastcall FUN_00474802(int param_1)

{
  if (*(int *)(param_1 + 0x27d58) != 0) {
    FUN_004091ab();
    *(undefined4 *)(*(int *)(param_1 + 0x27d58) + 0x1f8) = 0;
    *(undefined2 *)(*(int *)(param_1 + 0x27d58) + 0x1ea) = 0;
    (**(code **)(**(int **)(*(int *)(param_1 + 0x27d58) + 0x208) + 0x90))(&DAT_005d0490);
    (**(code **)(**(int **)(param_1 + 0x27d60) + 0x90))(&DAT_005d0494);
  }
  return;
}

