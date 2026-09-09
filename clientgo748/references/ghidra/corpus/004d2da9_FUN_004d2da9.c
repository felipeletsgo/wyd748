// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d2da9 | Name: FUN_004d2da9


void __thiscall FUN_004d2da9(int param_1,uint param_2)

{
  *(uint *)(param_1 + 0x88) = param_2;
  *(uint *)(param_1 + 0xbc) = param_2 >> 0x18;
  *(uint *)(param_1 + 0xc0) = (param_2 & 0xff0000) >> 0x10;
  *(uint *)(param_1 + 0xc4) = (param_2 & 0xff00) >> 8;
  *(uint *)(param_1 + 200) = param_2 & 0xff;
  return;
}

