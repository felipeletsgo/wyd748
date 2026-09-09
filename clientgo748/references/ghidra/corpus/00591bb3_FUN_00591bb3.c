// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00591bb3 | Name: FUN_00591bb3


void FUN_00591bb3(uint param_1)

{
  if ((0x5c9977 < param_1) && (param_1 < 0x5c9bd9)) {
    FUN_00596aad(((int)(param_1 - 0x5c9978) >> 5) + 0x1c);
    return;
  }
  EnterCriticalSection((LPCRITICAL_SECTION)(param_1 + 0x20));
  return;
}

