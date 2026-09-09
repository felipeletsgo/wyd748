// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00592473 | Name: FUN_00592473


HANDLE FUN_00592473(LPCSTR param_1,uint *param_2)

{
  HANDLE pvVar1;
  DWORD DVar2;
  undefined4 *puVar3;
  uint uVar4;
  _WIN32_FIND_DATAA local_144;
  
  pvVar1 = FindFirstFileA(param_1,&local_144);
  if (pvVar1 != (HANDLE)0xffffffff) {
    *param_2 = -(uint)(local_144.dwFileAttributes != 0x80) & local_144.dwFileAttributes;
    uVar4 = ___timet_from_ft(&local_144.ftCreationTime);
    param_2[1] = uVar4;
    uVar4 = ___timet_from_ft(&local_144.ftLastAccessTime);
    param_2[2] = uVar4;
    uVar4 = ___timet_from_ft(&local_144.ftLastWriteTime);
    param_2[3] = uVar4;
    param_2[4] = local_144.nFileSizeLow;
    FUN_0058ee20(param_2 + 5,local_144.cFileName);
    return pvVar1;
  }
  DVar2 = GetLastError();
  if (DVar2 < 2) {
LAB_005924b1:
    puVar3 = (undefined4 *)FUN_00594c1d();
    *puVar3 = 0x16;
  }
  else {
    if (3 < DVar2) {
      if (DVar2 == 8) {
        puVar3 = (undefined4 *)FUN_00594c1d();
        *puVar3 = 0xc;
        return (HANDLE)0xffffffff;
      }
      if (DVar2 != 0x12) goto LAB_005924b1;
    }
    puVar3 = (undefined4 *)FUN_00594c1d();
    *puVar3 = 2;
  }
  return (HANDLE)0xffffffff;
}

