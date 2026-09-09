// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e89e9 | Name: FUN_004e89e9


void __thiscall FUN_004e89e9(int param_1,uint param_2)

{
  *(uint *)(param_1 + 0x80) = param_2 >> 0x18;
  *(uint *)(param_1 + 0x84) = (param_2 & 0xff0000) >> 0x10;
  *(uint *)(param_1 + 0x88) = (param_2 & 0xff00) >> 8;
  *(uint *)(param_1 + 0x8c) = param_2 & 0xff;
  return;
}

