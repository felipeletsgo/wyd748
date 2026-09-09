// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058e9c6 | Name: socket


SOCKET socket(int af,int type,int protocol)

{
  SOCKET SVar1;
  
                    /* WARNING: Could not recover jumptable at 0x0058e9c6. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  SVar1 = socket(af,type,protocol);
  return SVar1;
}

