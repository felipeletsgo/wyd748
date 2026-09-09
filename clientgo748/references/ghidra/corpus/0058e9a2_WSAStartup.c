// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058e9a2 | Name: WSAStartup


int WSAStartup(WORD wVersionRequired,LPWSADATA lpWSAData)

{
  int iVar1;
  
                    /* WARNING: Could not recover jumptable at 0x0058e9a2. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  iVar1 = WSAStartup(wVersionRequired,lpWSAData);
  return iVar1;
}

