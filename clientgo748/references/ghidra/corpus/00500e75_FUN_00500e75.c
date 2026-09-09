// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00500e75 | Name: FUN_00500e75


void __thiscall FUN_00500e75(int param_1,undefined4 param_2)

{
  if (*(int *)(param_1 + 0x214) == 0) {
    if (*(int *)(param_1 + 0x24c) == 0x2c) {
      param_2 = 0x40490fdb;
    }
    *(undefined4 *)(param_1 + 0x22c) = param_2;
    *(undefined4 *)(param_1 + 0x224) = *(undefined4 *)(param_1 + 0x34);
  }
  return;
}

