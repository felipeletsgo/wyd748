// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00500e15 | Name: FUN_00500e15


void __thiscall FUN_00500e15(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  if (*(int *)(param_1 + 0x214) == 0) {
    if (*(int *)(param_1 + 0x24c) == 0x2c) {
      param_3 = 0x40490fdb;
    }
    FUN_0053e98a(param_2,param_3,param_4);
    *(undefined4 *)(param_1 + 0x22c) = *(undefined4 *)(param_1 + 0x34);
    *(undefined4 *)(param_1 + 0x224) = *(undefined4 *)(param_1 + 0x34);
  }
  return;
}

