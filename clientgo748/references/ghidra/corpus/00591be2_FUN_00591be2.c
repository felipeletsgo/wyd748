// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00591be2 | Name: FUN_00591be2


void FUN_00591be2(int param_1,int param_2)

{
  if (param_1 < 0x14) {
    FUN_00596aad(param_1 + 0x1c);
    return;
  }
  EnterCriticalSection((LPCRITICAL_SECTION)(param_2 + 0x20));
  return;
}

