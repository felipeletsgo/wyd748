// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0047e26a | Name: FUN_0047e26a


int FUN_0047e26a(void)

{
  int local_8;
  
  local_8 = 0;
  while( true ) {
    if (0x3f < local_8) {
      return -1;
    }
    if (*(int *)(DAT_0092e640 + 8 + local_8 * 0xc) == -1) break;
    local_8 = local_8 + 1;
  }
  return local_8;
}

