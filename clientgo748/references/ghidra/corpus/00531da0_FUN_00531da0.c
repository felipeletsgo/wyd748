// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00531da0 | Name: FUN_00531da0


void __thiscall FUN_00531da0(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  FUN_0053e47d(param_2,param_3,param_4);
  if (*(int *)(param_1 + 400) != 0) {
    *(undefined4 *)(*(int *)(param_1 + 400) + 0x74) = param_2;
    *(undefined4 *)(*(int *)(param_1 + 400) + 0x78) = param_3;
    *(undefined4 *)(*(int *)(param_1 + 400) + 0x7c) = param_4;
  }
  return;
}

