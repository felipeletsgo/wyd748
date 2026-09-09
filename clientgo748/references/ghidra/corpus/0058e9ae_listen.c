// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058e9ae | Name: listen


int listen(SOCKET s,int backlog)

{
  int iVar1;
  
                    /* WARNING: Could not recover jumptable at 0x0058e9ae. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  iVar1 = listen(s,backlog);
  return iVar1;
}

