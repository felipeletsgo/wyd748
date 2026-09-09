// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00591258 | Name: FUN_00591258


uint FUN_00591258(int param_1)

{
  uint uVar1;
  
  if (1 < DAT_005cc31c) {
    uVar1 = FUN_005980ea(param_1,4);
    return uVar1;
  }
  return (byte)PTR_DAT_005cc110[param_1 * 2] & 4;
}

