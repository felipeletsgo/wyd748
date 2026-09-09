// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058e8b0 | Name: ImmGetConversionStatus


BOOL ImmGetConversionStatus(HIMC param_1,LPDWORD lpfdwConversion,LPDWORD lpfdwSentence)

{
  BOOL BVar1;
  
                    /* WARNING: Could not recover jumptable at 0x0058e8b0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  BVar1 = ImmGetConversionStatus(param_1,lpfdwConversion,lpfdwSentence);
  return BVar1;
}

