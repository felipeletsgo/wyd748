// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00590e7f | Name: FUN_00590e7f


HANDLE FUN_00590e7f(undefined4 param_1,SIZE_T param_2,undefined4 param_3)

{
  LPVOID lpThreadId;
  HANDLE hThread;
  DWORD DVar1;
  
  DVar1 = 0;
  lpThreadId = (LPVOID)FUN_005916ba(1,0x74);
  if (lpThreadId != (LPVOID)0x0) {
    FUN_00593d40(lpThreadId);
    *(undefined4 *)((int)lpThreadId + 0x48) = param_1;
    *(undefined4 *)((int)lpThreadId + 0x4c) = param_3;
    hThread = CreateThread((LPSECURITY_ATTRIBUTES)0x0,param_2,FUN_00590ef6,lpThreadId,4,lpThreadId);
    *(HANDLE *)((int)lpThreadId + 4) = hThread;
    if ((hThread != (HANDLE)0x0) && (DVar1 = ResumeThread(hThread), DVar1 != 0xffffffff)) {
      return hThread;
    }
    DVar1 = GetLastError();
  }
  FUN_0058ffd8(lpThreadId);
  if (DVar1 != 0) {
    FUN_00594baa(DVar1);
  }
  return (HANDLE)0xffffffff;
}

