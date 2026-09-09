// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005912d5 | Name: FUN_005912d5


uint FUN_005912d5(int param_1)

{
  uint uVar1;
  
  if (1 < DAT_005cc31c) {
    uVar1 = FUN_005980ea(param_1,0x107);
    return uVar1;
  }
  return *(ushort *)(PTR_DAT_005cc110 + param_1 * 2) & 0x107;
}

