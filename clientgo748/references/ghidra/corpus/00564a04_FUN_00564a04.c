// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00564a04 | Name: FUN_00564a04


void FUN_00564a04(undefined4 param_1)

{
  HMODULE pHVar1;
  
  if (DAT_013b7388 == (FARPROC)0x0) {
    if (DAT_013b7384 != (FARPROC)0x0) goto LAB_00564a8d;
    pHVar1 = GetModuleHandleA("d3d8.dll");
    if (pHVar1 != (HMODULE)0x0) {
      pHVar1 = LoadLibraryA("d3d8.dll");
      if (pHVar1 != (HMODULE)0x0) {
        DAT_013b7388 = GetProcAddress(pHVar1,"DebugSetMute");
      }
    }
    pHVar1 = GetModuleHandleA("d3d8d.dll");
    if (pHVar1 != (HMODULE)0x0) {
      pHVar1 = LoadLibraryA("d3d8d.dll");
      if (pHVar1 != (HMODULE)0x0) {
        DAT_013b7384 = GetProcAddress(pHVar1,"DebugSetMute");
      }
    }
    if (DAT_013b7388 != (FARPROC)0x0) goto LAB_00564a79;
  }
  else {
LAB_00564a79:
    (*DAT_013b7388)(param_1);
  }
  if (DAT_013b7384 == (FARPROC)0x0) {
    return;
  }
LAB_00564a8d:
                    /* WARNING: Could not recover jumptable at 0x00564a8d. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*DAT_013b7384)();
  return;
}

