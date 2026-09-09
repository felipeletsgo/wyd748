// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058e9d2 | Name: WSAGetLastError


int WSAGetLastError(void)

{
  int iVar1;
  
                    /* WARNING: Could not recover jumptable at 0x0058e9d2. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  iVar1 = WSAGetLastError();
  return iVar1;
}

