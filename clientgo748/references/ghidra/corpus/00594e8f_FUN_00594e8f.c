// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00594e8f | Name: FUN_00594e8f


void FUN_00594e8f(uint param_1)

{
  int iVar1;
  int iVar2;
  
  iVar2 = (param_1 & 0x1f) * 0x24;
  iVar1 = (&DAT_013b8620)[(int)param_1 >> 5] + iVar2;
  if (*(int *)(iVar1 + 8) == 0) {
    FUN_00596aad(0x11);
    if (*(int *)(iVar1 + 8) == 0) {
      InitializeCriticalSection((LPCRITICAL_SECTION)(iVar1 + 0xc));
      *(int *)(iVar1 + 8) = *(int *)(iVar1 + 8) + 1;
    }
    FUN_00596b0e(0x11);
  }
  EnterCriticalSection((LPCRITICAL_SECTION)((&DAT_013b8620)[(int)param_1 >> 5] + 0xc + iVar2));
  return;
}

