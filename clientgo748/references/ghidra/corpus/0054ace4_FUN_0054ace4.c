// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054ace4 | Name: FUN_0054ace4


void __thiscall FUN_0054ace4(int param_1,int param_2)

{
  *(undefined4 *)(param_2 + 4) = *(undefined4 *)(param_1 + 4);
  *(int *)(param_2 + 0xc) = param_1;
  *(int *)(*(int *)(param_1 + 8) + 0xc) = param_2;
  *(undefined4 *)(param_2 + 8) = *(undefined4 *)(param_1 + 8);
  *(int *)(param_1 + 8) = param_2;
  if (*(int *)(*(int *)(param_1 + 4) + 0x10) == param_1) {
    *(int *)(*(int *)(param_1 + 4) + 0x10) = param_2;
  }
  return;
}

