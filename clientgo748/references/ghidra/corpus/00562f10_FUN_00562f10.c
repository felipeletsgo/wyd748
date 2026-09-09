// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00562f10 | Name: FUN_00562f10


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00562f10(void)

{
  FUN_0057133c(1);
                    /* WARNING: Could not recover jumptable at 0x00562f17. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*_DAT_005c5138)();
  return;
}

