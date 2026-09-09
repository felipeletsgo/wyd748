// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00593cec | Name: FUN_00593cec


undefined4 FUN_00593cec(void)

{
  DWORD *lpTlsValue;
  BOOL BVar1;
  DWORD DVar2;
  
  FUN_00596a84();
  DAT_005c9d60 = TlsAlloc();
  if (DAT_005c9d60 != 0xffffffff) {
    lpTlsValue = (DWORD *)FUN_005916ba(1,0x74);
    if (lpTlsValue != (DWORD *)0x0) {
      BVar1 = TlsSetValue(DAT_005c9d60,lpTlsValue);
      if (BVar1 != 0) {
        FUN_00593d40(lpTlsValue);
        DVar2 = GetCurrentThreadId();
        lpTlsValue[1] = 0xffffffff;
        *lpTlsValue = DVar2;
        return 1;
      }
    }
  }
  return 0;
}

