// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00434315 | Name: FUN_00434315


undefined4 FUN_00434315(int param_1)

{
  undefined4 uVar1;
  
  if ((param_1 < 0) && (0x67 < param_1)) {
    uVar1 = 0;
  }
  else {
    if (param_1 < 0x60) {
      if ((*(uint *)(DAT_013b71e8 + 0x9c8) & 1 << ((byte)(param_1 % 0x18) & 0x1f)) == 0) {
        return 0;
      }
    }
    else if ((*(uint *)(DAT_013b71e8 + 0x9c8) & 1 << ((char)param_1 + 0xb8U & 0x1f)) == 0) {
      return 0;
    }
    uVar1 = 1;
  }
  return uVar1;
}

