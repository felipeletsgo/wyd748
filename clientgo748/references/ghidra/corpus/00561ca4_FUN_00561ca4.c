// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00561ca4 | Name: FUN_00561ca4


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00561ca4(void)

{
  FUN_0057133c(1);
                    /* WARNING: Could not recover jumptable at 0x00561cab. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*_DAT_005c511c)();
  return;
}

