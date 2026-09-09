// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00562da1 | Name: FUN_00562da1


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00562da1(void)

{
  FUN_0057133c(1);
                    /* WARNING: Could not recover jumptable at 0x00562da8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*_DAT_005c50d4)();
  return;
}

