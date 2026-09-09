// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040927a | Name: FUN_0040927a


void __thiscall FUN_0040927a(int param_1,int param_2)

{
  if (*(int *)(param_1 + 0x210) - *(int *)(param_1 + 500) < param_2) {
    *(int *)(param_1 + 0x1f8) = (*(int *)(param_1 + 0x210) - *(int *)(param_1 + 500)) + 1;
    if (*(int *)(param_1 + 0x1f8) < 0) {
      *(undefined4 *)(param_1 + 0x1f8) = 0;
    }
  }
  else {
    *(int *)(param_1 + 0x1f8) = param_2;
  }
  return;
}

