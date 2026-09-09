// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00596aad | Name: FUN_00596aad


void FUN_00596aad(int param_1)

{
  int *piVar1;
  LPCRITICAL_SECTION lpCriticalSection;
  
  piVar1 = (int *)(&DAT_005cbf60 + param_1 * 4);
  if (*(int *)(&DAT_005cbf60 + param_1 * 4) == 0) {
    lpCriticalSection = _malloc(0x18);
    if (lpCriticalSection == (LPCRITICAL_SECTION)0x0) {
      __amsg_exit(0x11);
    }
    FUN_00596aad(0x11);
    if (*piVar1 == 0) {
      InitializeCriticalSection(lpCriticalSection);
      *piVar1 = (int)lpCriticalSection;
    }
    else {
      FUN_0058ffd8();
    }
    FUN_00596b0e(0x11);
  }
  EnterCriticalSection((LPCRITICAL_SECTION)*piVar1);
  return;
}

