// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00573eb5 | Name: FUN_00573eb5


undefined4 FUN_00573eb5(int param_1)

{
  if (*(char *)(param_1 + 0x113) != '\0') {
    *(uint *)(param_1 + 0x60) = *(uint *)(param_1 + 0x60) | 2;
    return 7;
  }
  return 1;
}

