// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058e8ce | Name: ImmGetCompositionStringA


LONG ImmGetCompositionStringA(HIMC param_1,DWORD param_2,LPVOID lpBuf,DWORD dwBufLen)

{
  LONG LVar1;
  
                    /* WARNING: Could not recover jumptable at 0x0058e8ce. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  LVar1 = ImmGetCompositionStringA(param_1,param_2,lpBuf,dwBufLen);
  return LVar1;
}

