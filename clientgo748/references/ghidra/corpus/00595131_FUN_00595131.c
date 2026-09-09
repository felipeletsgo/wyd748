// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00595131 | Name: FUN_00595131


DWORD FUN_00595131(uint param_1,LONG param_2,DWORD param_3)

{
  byte *pbVar1;
  HANDLE hFile;
  undefined4 *puVar2;
  DWORD DVar3;
  DWORD DVar4;
  
  hFile = (HANDLE)FUN_00594e4d(param_1);
  if (hFile == (HANDLE)0xffffffff) {
    puVar2 = (undefined4 *)FUN_00594c1d();
    *puVar2 = 9;
  }
  else {
    DVar3 = SetFilePointer(hFile,param_2,(PLONG)0x0,param_3);
    if (DVar3 == 0xffffffff) {
      DVar4 = GetLastError();
    }
    else {
      DVar4 = 0;
    }
    if (DVar4 == 0) {
      pbVar1 = (byte *)((&DAT_013b8620)[(int)param_1 >> 5] + 4 + (param_1 & 0x1f) * 0x24);
      *pbVar1 = *pbVar1 & 0xfd;
      return DVar3;
    }
    FUN_00594baa(DVar4);
  }
  return 0xffffffff;
}

