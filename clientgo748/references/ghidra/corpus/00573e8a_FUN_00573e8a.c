// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00573e8a | Name: FUN_00573e8a


void FUN_00573e8a(int param_1)

{
  if (*(char *)(param_1 + 0x117) == '\x10') {
    *(uint *)(param_1 + 0x60) = *(uint *)(param_1 + 0x60) | 0x10;
  }
  return;
}

