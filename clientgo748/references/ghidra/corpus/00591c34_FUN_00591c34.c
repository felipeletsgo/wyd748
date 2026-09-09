// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00591c34 | Name: FUN_00591c34


void FUN_00591c34(int param_1,int param_2)

{
  if (param_1 < 0x14) {
    FUN_00596b0e(param_1 + 0x1c);
    return;
  }
  LeaveCriticalSection((LPCRITICAL_SECTION)(param_2 + 0x20));
  return;
}

