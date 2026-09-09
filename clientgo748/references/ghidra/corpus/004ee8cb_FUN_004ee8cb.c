// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ee8cb | Name: FUN_004ee8cb


void __thiscall FUN_004ee8cb(int param_1,uint param_2)

{
  *(uint *)(param_1 + 0x98) = param_2;
  *(uint *)(param_1 + 0x9c) = param_2 >> 0x18;
  *(uint *)(param_1 + 0xa0) = (param_2 & 0xff0000) >> 0x10;
  *(uint *)(param_1 + 0xa4) = (param_2 & 0xff00) >> 8;
  *(uint *)(param_1 + 0xa8) = param_2 & 0xff;
  return;
}

