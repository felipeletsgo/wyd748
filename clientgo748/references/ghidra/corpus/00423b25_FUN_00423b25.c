// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00423b25 | Name: FUN_00423b25


bool FUN_00423b25(void)

{
  _OSVERSIONINFOA local_98;
  
  _memset(&local_98,0,0x94);
  local_98.dwOSVersionInfoSize = 0x94;
  GetVersionExA(&local_98);
  return local_98.dwPlatformId == 1;
}

