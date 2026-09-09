// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058e89e | Name: ImmAssociateContext


HIMC ImmAssociateContext(HWND param_1,HIMC param_2)

{
  HIMC pHVar1;
  
                    /* WARNING: Could not recover jumptable at 0x0058e89e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  pHVar1 = ImmAssociateContext(param_1,param_2);
  return pHVar1;
}

