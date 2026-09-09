// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b428e | Name: FUN_004b428e


undefined4 FUN_004b428e(void)

{
  HANDLE pvVar1;
  
  if (DAT_0092e5cc != 0) {
    EnterCriticalSection((LPCRITICAL_SECTION)&DAT_0092e5d8);
    DAT_0092e5cc = 0;
    if (DAT_0092e5f8 != (code *)0x0) {
      pvVar1 = GetCurrentProcess();
      (*DAT_0092e5f8)(pvVar1);
    }
    FreeLibrary(DAT_0092e5d0);
    LeaveCriticalSection((LPCRITICAL_SECTION)&DAT_0092e5d8);
    if (DAT_0092e58c != 0) {
      FUN_0058ffd8(DAT_0092e58c);
      DAT_0092e58c = 0;
    }
    if (DAT_0092e590 != 0) {
      FUN_0058f62c(DAT_0092e590);
      DAT_0092e590 = 0;
    }
    DeleteCriticalSection((LPCRITICAL_SECTION)&DAT_0092e5d8);
    InterlockedDecrement((LONG *)&DAT_0092e5f0);
  }
  if (DAT_0092e628 != 1) {
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)0x0);
    DAT_0092e628 = 0;
  }
  return 0;
}

