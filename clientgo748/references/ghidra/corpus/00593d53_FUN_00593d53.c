// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00593d53 | Name: FUN_00593d53


DWORD * FUN_00593d53(void)

{
  DWORD dwErrCode;
  DWORD *lpTlsValue;
  BOOL BVar1;
  DWORD DVar2;
  
  dwErrCode = GetLastError();
  lpTlsValue = TlsGetValue(DAT_005c9d60);
  if (lpTlsValue == (DWORD *)0x0) {
    lpTlsValue = (DWORD *)FUN_005916ba(1,0x74);
    if (lpTlsValue != (DWORD *)0x0) {
      BVar1 = TlsSetValue(DAT_005c9d60,lpTlsValue);
      if (BVar1 != 0) {
        FUN_00593d40(lpTlsValue);
        DVar2 = GetCurrentThreadId();
        lpTlsValue[1] = 0xffffffff;
        *lpTlsValue = DVar2;
        goto LAB_00593dae;
      }
    }
    __amsg_exit(0x10);
  }
LAB_00593dae:
  SetLastError(dwErrCode);
  return lpTlsValue;
}

