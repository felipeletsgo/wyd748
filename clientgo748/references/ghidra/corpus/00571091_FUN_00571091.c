// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00571091 | Name: FUN_00571091


undefined4 FUN_00571091(DWORD param_1,LPCSTR param_2,LPBYTE param_3)

{
  LSTATUS LVar1;
  DWORD local_c;
  HKEY local_8;
  
  local_8 = (HKEY)0x0;
  LVar1 = RegOpenKeyA((HKEY)0x80000002,"Software\\Microsoft\\Direct3D",&local_8);
  if (LVar1 == 0) {
    LVar1 = RegQueryValueExA(local_8,param_2,(LPDWORD)0x0,&local_c,param_3,(LPDWORD)&stack0x00000010
                            );
    RegCloseKey(local_8);
    if ((LVar1 == 0) && (local_c == param_1)) {
      return 1;
    }
  }
  return 0;
}

