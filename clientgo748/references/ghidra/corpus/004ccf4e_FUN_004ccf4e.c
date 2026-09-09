// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ccf4e | Name: FUN_004ccf4e


void __thiscall FUN_004ccf4e(int param_1,uint param_2)

{
  undefined4 local_8;
  
  for (local_8 = 0; local_8 < 4; local_8 = local_8 + 1) {
    *(uint *)(param_1 + 0x94 + local_8 * 0x18) = param_2;
    *(uint *)(param_1 + 0xf4 + local_8 * 0x18) = param_2;
  }
  *(uint *)(param_1 + 0x154) = param_2 >> 0x18;
  *(uint *)(param_1 + 0x158) = (param_2 & 0xff0000) >> 0x10;
  *(uint *)(param_1 + 0x15c) = (param_2 & 0xff00) >> 8;
  *(uint *)(param_1 + 0x160) = param_2 & 0xff;
  return;
}

