// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004aec07 | Name: FUN_004aec07


void __fastcall FUN_004aec07(int param_1)

{
  if (*(int *)(param_1 + 0x448) != 0) {
    (**(code **)(**(int **)(param_1 + 8) + 0x20))(*(undefined4 *)(param_1 + 8));
    *(undefined4 *)(param_1 + 0x448) = 0;
  }
  return;
}

