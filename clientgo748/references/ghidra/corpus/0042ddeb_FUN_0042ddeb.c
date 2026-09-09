// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042ddeb | Name: FUN_0042ddeb


void __thiscall FUN_0042ddeb(int param_1,int param_2,int param_3)

{
  if (*(int *)(param_1 + 0x2a894 + param_2 * 4) != param_3) {
    *(int *)(param_1 + 0x2a894 + param_2 * 4) = param_3;
    (**(code **)(**(int **)(param_1 + 0x2a3e4) + 200))
              (*(undefined4 *)(param_1 + 0x2a3e4),param_2,param_3);
  }
  return;
}

