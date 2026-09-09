// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b3f12 | Name: FUN_004b3f12


undefined4 FUN_004b3f12(void)

{
  undefined4 uVar1;
  DWORD DVar2;
  
  if (DAT_0092e5cc == 0) {
    DAT_0092e5d0 = LoadLibraryA(s_dbghelp_dll_005b9c6c);
    if (DAT_0092e5d0 == (HMODULE)0x0) {
      DVar2 = GetLastError();
      FUN_00591822(s_LoadLibrary___dbghelp_dll_____Ge_005b9c78,DVar2);
      DAT_0092e5cc = 0;
      uVar1 = 1;
    }
    else {
      DAT_0092e5f8 = GetProcAddress(DAT_0092e5d0,s_SymCleanup_005b9cac);
      DAT_0092e5fc = GetProcAddress(DAT_0092e5d0,s_SymFunctionTableAccess64_005b9cb8);
      DAT_0092e600 = GetProcAddress(DAT_0092e5d0,s_SymGetLineFromAddr64_005b9cd4);
      DAT_0092e604 = GetProcAddress(DAT_0092e5d0,s_SymGetModuleBase64_005b9cec);
      DAT_0092e608 = GetProcAddress(DAT_0092e5d0,s_SymGetModuleInfo64_005b9d00);
      DAT_0092e60c = GetProcAddress(DAT_0092e5d0,s_SymGetOptions_005b9d14);
      DAT_0092e610 = GetProcAddress(DAT_0092e5d0,s_SymGetSymFromAddr64_005b9d24);
      DAT_0092e614 = GetProcAddress(DAT_0092e5d0,s_SymInitialize_005b9d38);
      DAT_0092e61c = GetProcAddress(DAT_0092e5d0,s_SymSetOptions_005b9d48);
      DAT_0092e620 = GetProcAddress(DAT_0092e5d0,s_StackWalk64_005b9d58);
      DAT_0092e624 = GetProcAddress(DAT_0092e5d0,s_UnDecorateSymbolName_005b9d64);
      DAT_0092e618 = GetProcAddress(DAT_0092e5d0,s_SymLoadModule64_005b9d7c);
      if ((((((DAT_0092e5f8 == (FARPROC)0x0) || (DAT_0092e5fc == (FARPROC)0x0)) ||
            (DAT_0092e604 == (FARPROC)0x0)) ||
           ((DAT_0092e608 == (FARPROC)0x0 || (DAT_0092e60c == (FARPROC)0x0)))) ||
          ((DAT_0092e610 == (FARPROC)0x0 ||
           ((DAT_0092e614 == (FARPROC)0x0 || (DAT_0092e61c == (FARPROC)0x0)))))) ||
         ((DAT_0092e620 == (FARPROC)0x0 ||
          ((DAT_0092e624 == (FARPROC)0x0 || (DAT_0092e618 == (FARPROC)0x0)))))) {
        FUN_00591822(s_GetProcAddress____some_required_f_005b9d8c);
        FreeLibrary(DAT_0092e5d0);
        DAT_0092e5cc = 0;
        uVar1 = 1;
      }
      else {
        DAT_0092e5cc = 1;
        InitializeCriticalSection((LPCRITICAL_SECTION)&DAT_0092e5d8);
        uVar1 = 0;
      }
    }
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

