// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005611dd | Name: FUN_005611dd


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_005611dd(void)

{
  FUN_0057133c(1);
                    /* WARNING: Could not recover jumptable at 0x005611e4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*_DAT_005c50a4)();
  return;
}

