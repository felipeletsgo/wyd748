// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00591c05 | Name: FUN_00591c05


void FUN_00591c05(uint param_1)

{
  if ((0x5c9977 < param_1) && (param_1 < 0x5c9bd9)) {
    FUN_00596b0e(((int)(param_1 - 0x5c9978) >> 5) + 0x1c);
    return;
  }
  LeaveCriticalSection((LPCRITICAL_SECTION)(param_1 + 0x20));
  return;
}

