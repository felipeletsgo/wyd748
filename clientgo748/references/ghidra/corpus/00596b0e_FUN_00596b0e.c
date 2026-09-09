// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00596b0e | Name: FUN_00596b0e


void FUN_00596b0e(int param_1)

{
  LeaveCriticalSection(*(LPCRITICAL_SECTION *)(&DAT_005cbf60 + param_1 * 4));
  return;
}

