// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004aebd1 | Name: FUN_004aebd1


void __fastcall FUN_004aebd1(int param_1)

{
  if (*(int *)(param_1 + 0x448) == 0) {
    (**(code **)(**(int **)(param_1 + 8) + 0x1c))(*(undefined4 *)(param_1 + 8));
    *(undefined4 *)(param_1 + 0x448) = 1;
  }
  return;
}

