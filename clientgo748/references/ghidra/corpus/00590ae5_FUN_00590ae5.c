// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00590ae5 | Name: FUN_00590ae5


undefined4 FUN_00590ae5(uint param_1)

{
  HANDLE hFile;
  BOOL BVar1;
  DWORD DVar2;
  DWORD *pDVar3;
  undefined4 *puVar4;
  int iVar5;
  undefined4 uVar6;
  
  if (DAT_013b8720 <= param_1) {
LAB_00590b66:
    puVar4 = (undefined4 *)FUN_00594c1d();
    *puVar4 = 9;
    return 0xffffffff;
  }
  iVar5 = (param_1 & 0x1f) * 0x24;
  if ((*(byte *)((&DAT_013b8620)[(int)param_1 >> 5] + 4 + iVar5) & 1) == 0) goto LAB_00590b66;
  FUN_00594e8f(param_1);
  if ((*(byte *)((&DAT_013b8620)[(int)param_1 >> 5] + 4 + iVar5) & 1) != 0) {
    hFile = (HANDLE)FUN_00594e4d(param_1);
    BVar1 = FlushFileBuffers(hFile);
    if (BVar1 == 0) {
      DVar2 = GetLastError();
    }
    else {
      DVar2 = 0;
    }
    uVar6 = 0;
    if (DVar2 == 0) goto LAB_00590b5b;
    pDVar3 = (DWORD *)FUN_00594c26();
    *pDVar3 = DVar2;
  }
  puVar4 = (undefined4 *)FUN_00594c1d();
  *puVar4 = 9;
  uVar6 = 0xffffffff;
LAB_00590b5b:
  FUN_00594eee(param_1);
  return uVar6;
}

