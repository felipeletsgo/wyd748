// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005912ad | Name: FUN_005912ad


uint FUN_005912ad(int param_1)

{
  uint uVar1;
  
  if (1 < DAT_005cc31c) {
    uVar1 = FUN_005980ea(param_1,8);
    return uVar1;
  }
  return (byte)PTR_DAT_005cc110[param_1 * 2] & 8;
}

