// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d9b96 | Name: FUN_004d9b96


void __thiscall FUN_004d9b96(int param_1,uint param_2)

{
  *(uint *)(param_1 + 0x2908) = (param_2 & 0xff0000) >> 0x10;
  *(uint *)(param_1 + 0x290c) = (param_2 & 0xff00) >> 8;
  *(uint *)(param_1 + 0x2910) = param_2 & 0xff;
  return;
}

