// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00562855 | Name: FUN_00562855


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00562855(void)

{
  FUN_0057133c(1);
                    /* WARNING: Could not recover jumptable at 0x00562860. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*_DAT_005c5124)();
  return;
}

