// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005704c4 | Name: FUN_005704c4


undefined4 __thiscall FUN_005704c4(undefined4 *param_1,LPCWSTR param_2,int param_3)

{
  int cbMultiByte;
  HANDLE pvVar1;
  undefined4 uVar2;
  LPCWSTR lpFileName;
  _OSVERSIONINFOA local_9c;
  undefined4 *local_8;
  
  if (param_2 == (LPCWSTR)0x0) {
    return 0x8876086c;
  }
  lpFileName = param_2;
  local_8 = param_1;
  if (param_3 != 0) {
    local_9c.dwOSVersionInfoSize = 0x94;
    GetVersionExA(&local_9c);
    if (local_9c.dwPlatformId != 2) {
      lpFileName = (LPCWSTR)&stack0xffffff58;
      cbMultiByte = WideCharToMultiByte(0,0,param_2,-1,(LPSTR)0x0,0,(LPCSTR)0x0,(LPBOOL)0x0);
      FUN_0058f730();
      WideCharToMultiByte(0,0,param_2,-1,&stack0xffffff58,cbMultiByte,(LPCSTR)0x0,(LPBOOL)0x0);
      param_3 = 0;
      param_1 = local_8;
    }
    if (param_3 != 0) {
      pvVar1 = CreateFileW(lpFileName,0x40000000,0,(LPSECURITY_ATTRIBUTES)0x0,2,0,(HANDLE)0x0);
      goto LAB_0057056b;
    }
  }
  pvVar1 = CreateFileA((LPCSTR)lpFileName,0x40000000,0,(LPSECURITY_ATTRIBUTES)0x0,2,0,(HANDLE)0x0);
LAB_0057056b:
  *param_1 = pvVar1;
  if (pvVar1 == (HANDLE)0xffffffff) {
    GetLastError();
    uVar2 = 0x88760b59;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}

