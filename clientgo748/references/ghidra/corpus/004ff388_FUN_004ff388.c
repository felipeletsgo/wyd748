// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ff388 | Name: FUN_004ff388


undefined4 FUN_004ff388(void)

{
  undefined4 uVar1;
  
  if (*(char *)(DAT_013b71e8 + 0x74d) == '\0') {
    uVar1 = 0;
  }
  else if ((*(uint *)(DAT_013b71e8 + 0x9c8) & 0x40000000) == 0) {
    uVar1 = 1;
  }
  else {
    uVar1 = 2;
  }
  return uVar1;
}

