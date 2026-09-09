// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058ea45 | Name: FUN_0058ea45


void FUN_0058ea45(code *UNRECOVERED_JUMPTABLE)

{
  ExceptionList = *(void **)ExceptionList;
                    /* WARNING: Could not recover jumptable at 0x0058ea70. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}

