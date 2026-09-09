// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058e9e4 | Name: recv


int recv(SOCKET s,char *buf,int len,int flags)

{
  int iVar1;
  
                    /* WARNING: Could not recover jumptable at 0x0058e9e4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  iVar1 = recv(s,buf,len,flags);
  return iVar1;
}

