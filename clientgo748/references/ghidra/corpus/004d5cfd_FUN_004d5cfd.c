// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d5cfd | Name: FUN_004d5cfd


void __thiscall FUN_004d5cfd(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  *(undefined4 *)(param_1 + 0x1d8) = param_2;
  *(undefined4 *)(param_1 + 0x1e0) = param_4;
  *(undefined4 *)(param_1 + 0x1dc) = param_3;
  FUN_0053e47d(param_2,param_3,param_4);
  return;
}

