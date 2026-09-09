// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058e9d8 | Name: connect


int connect(SOCKET s,sockaddr *name,int namelen)

{
  int iVar1;
  
                    /* WARNING: Could not recover jumptable at 0x0058e9d8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  iVar1 = connect(s,name,namelen);
  return iVar1;
}

