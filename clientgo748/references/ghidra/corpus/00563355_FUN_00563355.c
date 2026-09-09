// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00563355 | Name: FUN_00563355


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00563355(void)

{
  FUN_0057133c(1);
                    /* WARNING: Could not recover jumptable at 0x0056335c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*_DAT_005c50dc)();
  return;
}

