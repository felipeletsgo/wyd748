// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00561f6d | Name: FUN_00561f6d


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00561f6d(void)

{
  FUN_0057133c(1);
                    /* WARNING: Could not recover jumptable at 0x00561f74. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*_DAT_005c50d0)();
  return;
}

