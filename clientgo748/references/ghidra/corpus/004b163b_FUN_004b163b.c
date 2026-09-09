// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b163b | Name: FUN_004b163b


int __fastcall FUN_004b163b(int param_1)

{
  int local_8;
  
  local_8 = 0;
  while( true ) {
    if (0x3ff < local_8) {
      return -1;
    }
    if (*(int *)(param_1 + 0x41808 + local_8 * 0x58) == 0) break;
    local_8 = local_8 + 1;
  }
  return local_8;
}

