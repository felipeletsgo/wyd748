// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004cdb05 | Name: FUN_004cdb05


void __thiscall FUN_004cdb05(int param_1,uint param_2)

{
  undefined4 local_8;
  
  for (local_8 = 0; local_8 < 4; local_8 = local_8 + 1) {
    *(uint *)(param_1 + 0x90 + local_8 * 0x1c) = param_2;
  }
  *(uint *)(param_1 + 0x128) = param_2 >> 0x18;
  *(uint *)(param_1 + 300) = (param_2 & 0xff0000) >> 0x10;
  *(uint *)(param_1 + 0x130) = (param_2 & 0xff00) >> 8;
  *(uint *)(param_1 + 0x134) = param_2 & 0xff;
  return;
}

