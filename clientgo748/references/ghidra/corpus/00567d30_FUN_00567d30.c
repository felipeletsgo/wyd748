// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00567d30 | Name: FUN_00567d30


void FUN_00567d30(void)

{
  int iVar1;
  code *UNRECOVERED_JUMPTABLE;
  
  iVar1 = FUN_00571112();
  if (iVar1 == 0) {
    PTR_FUN_005c57c0 = FUN_005679a3;
    UNRECOVERED_JUMPTABLE = FUN_00567afa;
  }
  else {
    UNRECOVERED_JUMPTABLE = FUN_00567be2;
    PTR_FUN_005c57c0 = FUN_00567be2;
  }
  PTR_FUN_005c57c4 = UNRECOVERED_JUMPTABLE;
                    /* WARNING: Could not recover jumptable at 0x00567d5d. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}

