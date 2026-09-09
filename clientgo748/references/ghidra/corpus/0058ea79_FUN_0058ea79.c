// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058ea79 | Name: FUN_0058ea79


void FUN_0058ea79(undefined4 param_1,code *UNRECOVERED_JUMPTABLE)

{
  LOCK();
  UNLOCK();
                    /* WARNING: Could not recover jumptable at 0x0058ea7e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}

