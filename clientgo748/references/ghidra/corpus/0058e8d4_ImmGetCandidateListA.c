// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058e8d4 | Name: ImmGetCandidateListA


DWORD ImmGetCandidateListA(HIMC param_1,DWORD deIndex,LPCANDIDATELIST lpCandList,DWORD dwBufLen)

{
  DWORD DVar1;
  
                    /* WARNING: Could not recover jumptable at 0x0058e8d4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  DVar1 = ImmGetCandidateListA(param_1,deIndex,lpCandList,dwBufLen);
  return DVar1;
}

