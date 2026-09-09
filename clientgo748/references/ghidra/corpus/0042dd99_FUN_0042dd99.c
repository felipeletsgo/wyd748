// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042dd99 | Name: FUN_0042dd99


void __thiscall FUN_0042dd99(int param_1,int param_2,int param_3)

{
  if (*(int *)(param_1 + 0x2a6c4 + param_2 * 4) != param_3) {
    *(int *)(param_1 + 0x2a6c4 + param_2 * 4) = param_3;
    (**(code **)(**(int **)(param_1 + 0x2a3e4) + 0xf4))
              (*(undefined4 *)(param_1 + 0x2a3e4),param_2,param_3);
  }
  return;
}

