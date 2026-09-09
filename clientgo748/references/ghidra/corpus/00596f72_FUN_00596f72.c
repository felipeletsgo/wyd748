// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00596f72 | Name: FUN_00596f72


int FUN_00596f72(void)

{
  DWORD DVar1;
  int iVar2;
  CHAR local_108;
  char local_107;
  
  iVar2 = 0;
  DVar1 = GetCurrentDirectoryA(0x104,&local_108);
  if ((DVar1 != 0) && (local_107 == ':')) {
    iVar2 = FUN_0059b76a(local_108);
    iVar2 = iVar2 + -0x40;
  }
  return iVar2;
}

