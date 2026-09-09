// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00591280 | Name: FUN_00591280


uint FUN_00591280(int param_1)

{
  uint uVar1;
  
  if (1 < DAT_005cc31c) {
    uVar1 = FUN_005980ea(param_1,0x80);
    return uVar1;
  }
  return (byte)PTR_DAT_005cc110[param_1 * 2] & 0x80;
}

