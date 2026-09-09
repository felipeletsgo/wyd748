// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042de3d | Name: FUN_0042de3d


void __thiscall FUN_0042de3d(int param_1,int param_2,int param_3,int param_4)

{
  if (*(int *)(param_1 + 0x2ac94 + param_2 * 0x74 + param_3 * 4) != param_4) {
    *(int *)(param_1 + 0x2ac94 + param_2 * 0x74 + param_3 * 4) = param_4;
    (**(code **)(**(int **)(param_1 + 0x2a3e4) + 0xfc))
              (*(undefined4 *)(param_1 + 0x2a3e4),param_2,param_3,param_4);
  }
  return;
}

