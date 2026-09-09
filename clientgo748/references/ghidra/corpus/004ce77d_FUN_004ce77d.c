// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ce77d | Name: FUN_004ce77d


void __thiscall FUN_004ce77d(int param_1,uint param_2)

{
  *(uint *)(param_1 + 0x88) = param_2 >> 0x18;
  *(uint *)(param_1 + 0x8c) = (param_2 & 0xff0000) >> 0x10;
  *(uint *)(param_1 + 0x90) = (param_2 & 0xff00) >> 8;
  *(uint *)(param_1 + 0x94) = param_2 & 0xff;
  return;
}

