// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005958b8 | Name: FUN_005958b8


uint FUN_005958b8(int param_1)

{
  uint uVar1;
  
  uVar1 = DAT_013b85fc;
  while( true ) {
    if (DAT_013b85fc + DAT_013b85f8 * 0x14 <= uVar1) {
      return 0;
    }
    if ((uint)(param_1 - *(int *)(uVar1 + 0xc)) < 0x100000) break;
    uVar1 = uVar1 + 0x14;
  }
  return uVar1;
}

