// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00470d43 | Name: FUN_00470d43


void __thiscall FUN_00470d43(int param_1,int param_2)

{
  if ((((-1 < param_2) && (param_2 < 100)) && (*(int *)(param_1 + 0x289a4) != 0)) &&
     (*(int *)(param_1 + 0x289a8 + param_2 * 4) != 0)) {
    *(uint *)(*(int *)(param_1 + 0x289a8 + param_2 * 4) + 0x1e8) =
         (uint)(*(int *)(*(int *)(param_1 + 0x289a8 + param_2 * 4) + 0x1e8) == 0);
    (**(code **)(**(int **)(param_1 + 0x289a8 + param_2 * 4) + 0x54))();
  }
  return;
}

