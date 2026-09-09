// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00561d24 | Name: FUN_00561d24


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00561d24(void)

{
  FUN_0057133c(1);
                    /* WARNING: Could not recover jumptable at 0x00561d2b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*_DAT_005c5098)();
  return;
}

