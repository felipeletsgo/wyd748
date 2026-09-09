// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00423dd8 | Name: FUN_00423dd8


bool FUN_00423dd8(void)

{
  int iVar1;
  WSADATA local_194;
  
  iVar1 = WSAStartup(0x101,&local_194);
  return iVar1 == 0;
}

