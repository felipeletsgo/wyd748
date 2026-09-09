// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00560fa0 | Name: FUN_00560fa0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00560fa0(void)

{
  FUN_0057133c(1);
                    /* WARNING: Could not recover jumptable at 0x00560fa7. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*_DAT_005c5140)();
  return;
}

