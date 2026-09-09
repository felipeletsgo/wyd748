// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004cbb6e | Name: FUN_004cbb6e


void __thiscall FUN_004cbb6e(int param_1,uint param_2)

{
  undefined4 local_8;
  
  for (local_8 = 0; local_8 < 4; local_8 = local_8 + 1) {
    *(uint *)(param_1 + 0x8c + local_8 * 0x18) = param_2;
  }
  *(uint *)(param_1 + 0x114) = param_2 >> 0x18;
  *(uint *)(param_1 + 0x118) = (param_2 & 0xff0000) >> 0x10;
  *(uint *)(param_1 + 0x11c) = (param_2 & 0xff00) >> 8;
  *(uint *)(param_1 + 0x120) = param_2 & 0xff;
  return;
}

