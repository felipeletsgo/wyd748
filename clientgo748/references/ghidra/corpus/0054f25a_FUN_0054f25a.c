// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054f25a | Name: FUN_0054f25a


void FUN_0054f25a(void)

{
  size_t sVar1;
  CHAR local_108 [256];
  int local_8;
  
  GetModuleFileNameA((HMODULE)0x0,local_108,0x100);
  sVar1 = _strlen(local_108);
  local_8 = sVar1 - 1;
  do {
    if (local_8 < 1) {
LAB_0054f2c8:
      SetCurrentDirectoryA(local_108);
      return;
    }
    if (local_108[local_8] == '\\') {
      local_108[local_8] = '\0';
      goto LAB_0054f2c8;
    }
    local_8 = local_8 + -1;
  } while( true );
}

