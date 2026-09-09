// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053e435 | Name: FUN_0053e435


void __thiscall FUN_0053e435(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  *(undefined4 *)(param_1 + 0x28) = param_2;
  *(undefined4 *)(param_1 + 0x2c) = param_4;
  *(undefined4 *)(param_1 + 0x30) = param_3;
  if (*(int *)(param_1 + 0x60) != 0) {
    FUN_004be17b(param_2,param_3,param_4);
  }
  return;
}

