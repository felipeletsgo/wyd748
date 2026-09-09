// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059304f | Name: FUN_0059304f


void FUN_0059304f(void)

{
  HMODULE hModule;
  FARPROC pFVar1;
  
  hModule = GetModuleHandleA("KERNEL32");
  if (hModule != (HMODULE)0x0) {
    pFVar1 = GetProcAddress(hModule,"IsProcessorFeaturePresent");
    if (pFVar1 != (FARPROC)0x0) {
      (*pFVar1)(0);
      return;
    }
  }
  FUN_00593011();
  return;
}

