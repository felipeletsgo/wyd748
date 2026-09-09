// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059c9fd | Name: FUN_0059c9fd


int FUN_0059c9fd(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  HMODULE hModule;
  int iVar1;
  
  iVar1 = 0;
  if (DAT_013b8354 == (FARPROC)0x0) {
    hModule = LoadLibraryA("user32.dll");
    if (hModule != (HMODULE)0x0) {
      DAT_013b8354 = GetProcAddress(hModule,"MessageBoxA");
      if (DAT_013b8354 != (FARPROC)0x0) {
        DAT_013b8358 = GetProcAddress(hModule,"GetActiveWindow");
        DAT_013b835c = GetProcAddress(hModule,"GetLastActivePopup");
        goto LAB_0059ca4c;
      }
    }
    iVar1 = 0;
  }
  else {
LAB_0059ca4c:
    if (DAT_013b8358 != (FARPROC)0x0) {
      iVar1 = (*DAT_013b8358)();
      if ((iVar1 != 0) && (DAT_013b835c != (FARPROC)0x0)) {
        iVar1 = (*DAT_013b835c)(iVar1);
      }
    }
    iVar1 = (*DAT_013b8354)(iVar1,param_1,param_2,param_3);
  }
  return iVar1;
}

