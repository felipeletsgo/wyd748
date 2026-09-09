// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058e9b4 | Name: bind


int bind(SOCKET s,sockaddr *addr,int namelen)

{
  int iVar1;
  
                    /* WARNING: Could not recover jumptable at 0x0058e9b4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  iVar1 = bind(s,addr,namelen);
  return iVar1;
}

