// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ae945 | Name: FUN_004ae945


void __fastcall FUN_004ae945(int param_1)

{
  if (*(int *)(param_1 + 8) != 0) {
    (**(code **)(**(int **)(param_1 + 8) + 0x20))(*(undefined4 *)(param_1 + 8));
    *(undefined4 *)(param_1 + 0x448) = 0;
  }
  if (*(int *)(param_1 + 8) != 0) {
    (**(code **)(**(int **)(param_1 + 8) + 8))(*(undefined4 *)(param_1 + 8));
    *(undefined4 *)(param_1 + 8) = 0;
  }
  if (*(int *)(param_1 + 4) != 0) {
    (**(code **)(**(int **)(param_1 + 4) + 8))(*(undefined4 *)(param_1 + 4));
    *(undefined4 *)(param_1 + 4) = 0;
  }
  return;
}

