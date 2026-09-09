// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00552ecb | Name: FUN_00552ecb


uint FUN_00552ecb(int param_1,int param_2)

{
  uint in_EAX;
  uint uVar1;
  
  if ((((param_1 < 0) || (0x1000 < param_1)) || (param_2 < 0)) || (0x1000 < param_1)) {
    uVar1 = in_EAX & 0xffffff00;
  }
  else {
    uVar1 = (int)(param_1 + (param_1 >> 0x1f & 3U)) >> 2 & 0x3ff;
    uVar1 = CONCAT31((int3)(uVar1 >> 8),
                     (&DAT_00934178)
                     [uVar1 + ((int)(param_2 + (param_2 >> 0x1f & 3U)) >> 2 & 0x3ffU) * 0x400]);
  }
  return uVar1;
}

