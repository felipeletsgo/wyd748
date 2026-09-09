// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042d66e | Name: FUN_0042d66e


undefined4 __fastcall FUN_0042d66e(int param_1)

{
  if (*(int *)(param_1 + 0x2a70c) != 0) {
    FUN_004b7ca2();
  }
  if (*(int *)(param_1 + 0x2a710) != 0) {
    FUN_004afe59();
  }
  if (DAT_013b71e8 != (int *)0x0) {
    (**(code **)(*DAT_013b71e8 + 0x40))();
  }
  if (*(int *)(param_1 + 0x2a704) != 0) {
    (**(code **)(**(int **)(param_1 + 0x2a704) + 8))(*(undefined4 *)(param_1 + 0x2a704));
    *(undefined4 *)(param_1 + 0x2a704) = 0;
  }
  return 0;
}

