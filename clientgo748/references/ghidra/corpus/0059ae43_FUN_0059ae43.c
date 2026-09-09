// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059ae43 | Name: FUN_0059ae43


int FUN_0059ae43(LPSTR param_1,WCHAR param_2)

{
  LPSTR lpMultiByteStr;
  int iVar1;
  undefined4 *puVar2;
  
  lpMultiByteStr = param_1;
  if (param_1 == (LPSTR)0x0) {
    return 0;
  }
  if (DAT_013b82e4 == 0) {
    if ((ushort)param_2 < 0x100) {
      *param_1 = (CHAR)param_2;
      return 1;
    }
  }
  else {
    param_1 = (LPSTR)0x0;
    iVar1 = WideCharToMultiByte(DAT_013b82f4,0x220,&param_2,1,lpMultiByteStr,DAT_005cc31c,
                                (LPCSTR)0x0,(LPBOOL)&param_1);
    if ((iVar1 != 0) && (param_1 == (LPSTR)0x0)) {
      return iVar1;
    }
  }
  puVar2 = (undefined4 *)FUN_00594c1d();
  *puVar2 = 0x2a;
  return -1;
}

