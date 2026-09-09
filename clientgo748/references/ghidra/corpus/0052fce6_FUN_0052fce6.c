// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052fce6 | Name: FUN_0052fce6


void __thiscall FUN_0052fce6(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  *(undefined4 *)(param_1 + 0x160) = param_2;
  *(undefined4 *)(param_1 + 0x164) = param_4;
  FUN_0053e47d(param_2,param_3,param_4);
  return;
}

