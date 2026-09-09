// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00595683 | Name: FUN_00595683


undefined4 FUN_00595683(undefined4 param_1)

{
  int iVar1;
  
  if (DAT_013b8050 != (code *)0x0) {
    iVar1 = (*DAT_013b8050)(param_1);
    if (iVar1 != 0) {
      return 1;
    }
  }
  return 0;
}

