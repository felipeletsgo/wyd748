// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00434228 | Name: FUN_00434228


undefined4 FUN_00434228(int param_1)

{
  int iVar1;
  
  if ((param_1 < 0) || (0x5f < param_1)) {
    if ((0x5f < param_1) &&
       ((param_1 < 0x68 &&
        ((*(uint *)(DAT_013b71e8 + 0x9c8) & 1 << ((char)param_1 + 0xb8U & 0x1f)) == 0)))) {
      return 0;
    }
  }
  else {
    if (param_1 / 0x18 != (uint)*(byte *)(DAT_013b71e8 + 0x700)) {
      return 0;
    }
    if ((*(uint *)(DAT_013b71e8 + 0x9c8) & 1 << ((byte)(param_1 % 0x18) & 0x1f)) == 0) {
      return 0;
    }
    iVar1 = ((int)(param_1 % 0x18 + (param_1 % 0x18 >> 0x1f & 7U)) >> 3) + 1;
    if ((iVar1 < 1) || (3 < iVar1)) {
      return 0;
    }
  }
  return 1;
}

