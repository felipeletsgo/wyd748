// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058e8b6 | Name: ImmGetContext


HIMC ImmGetContext(HWND param_1)

{
  HIMC pHVar1;
  
                    /* WARNING: Could not recover jumptable at 0x0058e8b6. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  pHVar1 = ImmGetContext(param_1);
  return pHVar1;
}

