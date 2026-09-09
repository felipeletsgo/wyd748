// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00561a5b | Name: FUN_00561a5b


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00561a5b(void)

{
  FUN_0057133c(1);
                    /* WARNING: Could not recover jumptable at 0x00561a62. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*_DAT_005c5094)();
  return;
}

