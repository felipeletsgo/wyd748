// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005906a7 | Name: FUN_005906a7


undefined4 FUN_005906a7(LPCSTR param_1,int *param_2)

{
  undefined2 uVar1;
  int iVar2;
  undefined4 *puVar3;
  char *_Str;
  size_t sVar4;
  UINT UVar5;
  BOOL BVar6;
  DWORD DVar7;
  undefined1 local_268 [260];
  _WIN32_FIND_DATAA local_164;
  int local_24;
  _FILETIME local_20;
  HANDLE local_18;
  _SYSTEMTIME local_14;
  
  iVar2 = FUN_00597030(param_1,&DAT_005aa010);
  if (iVar2 != 0) {
LAB_005906da:
    puVar3 = (undefined4 *)FUN_00594c1d();
    *puVar3 = 2;
    puVar3 = (undefined4 *)FUN_00594c26();
    *puVar3 = 2;
    return 0xffffffff;
  }
  if (param_1[1] == ':') {
    if ((*param_1 != '\0') && (param_1[2] == '\0')) goto LAB_005906da;
    local_24 = FUN_00596fb5((int)*param_1);
    local_24 = local_24 + -0x60;
  }
  else {
    local_24 = FUN_00596f72();
  }
  local_18 = FindFirstFileA(param_1,&local_164);
  if (local_18 == (HANDLE)0xffffffff) {
    iVar2 = FUN_00597030(param_1,&PTR_DAT_005aa00c);
    if ((((iVar2 == 0) ||
         (_Str = (char *)FUN_00596ecd(local_268,param_1,0x104), _Str == (char *)0x0)) ||
        ((sVar4 = _strlen(_Str), sVar4 != 3 && (iVar2 = FUN_00590962(_Str), iVar2 == 0)))) ||
       (UVar5 = GetDriveTypeA(_Str), UVar5 < 2)) {
      puVar3 = (undefined4 *)FUN_00594c1d();
      *puVar3 = 2;
      puVar3 = (undefined4 *)FUN_00594c26();
      *puVar3 = 2;
      return 0xffffffff;
    }
    local_164.dwFileAttributes = 0x10;
    local_164.nFileSizeHigh = 0;
    local_164.nFileSizeLow = 0;
    local_164.cFileName[0] = '\0';
    iVar2 = FUN_00596e0b(0x7bc,1,1,0,0,0,0xffffffff);
    param_2[7] = iVar2;
    param_2[6] = iVar2;
    param_2[8] = iVar2;
  }
  else {
    BVar6 = FileTimeToLocalFileTime(&local_164.ftLastWriteTime,&local_20);
    if ((BVar6 == 0) || (BVar6 = FileTimeToSystemTime(&local_20,&local_14), BVar6 == 0)) {
LAB_00590945:
      DVar7 = GetLastError();
      FUN_00594baa(DVar7);
      FindClose(local_18);
      return 0xffffffff;
    }
    iVar2 = FUN_00596e0b(local_14.wYear,local_14.wMonth,local_14.wDay,local_14.wHour,
                         local_14.wMinute,local_14.wSecond,0xffffffff);
    param_2[7] = iVar2;
    if ((local_164.ftLastAccessTime.dwLowDateTime != 0) ||
       (local_164.ftLastAccessTime.dwHighDateTime != 0)) {
      BVar6 = FileTimeToLocalFileTime(&local_164.ftLastAccessTime,&local_20);
      if ((BVar6 == 0) || (BVar6 = FileTimeToSystemTime(&local_20,&local_14), BVar6 == 0))
      goto LAB_00590945;
      iVar2 = FUN_00596e0b(local_14.wYear,local_14.wMonth,local_14.wDay,local_14.wHour,
                           local_14.wMinute,local_14.wSecond,0xffffffff);
    }
    param_2[6] = iVar2;
    if ((local_164.ftCreationTime.dwLowDateTime == 0) &&
       (local_164.ftCreationTime.dwHighDateTime == 0)) {
      iVar2 = param_2[7];
    }
    else {
      BVar6 = FileTimeToLocalFileTime(&local_164.ftCreationTime,&local_20);
      if ((BVar6 == 0) || (BVar6 = FileTimeToSystemTime(&local_20,&local_14), BVar6 == 0))
      goto LAB_00590945;
      iVar2 = FUN_00596e0b(local_14.wYear,local_14.wMonth,local_14.wDay,local_14.wHour,
                           local_14.wMinute,local_14.wSecond,0xffffffff);
    }
    param_2[8] = iVar2;
    FindClose(local_18);
  }
  uVar1 = FUN_005905ef(local_164.dwFileAttributes,param_1);
  *(undefined2 *)((int)param_2 + 6) = uVar1;
  param_2[5] = local_164.nFileSizeLow;
  *param_2 = local_24 + -1;
  param_2[4] = local_24 + -1;
  *(undefined2 *)(param_2 + 2) = 1;
  *(undefined2 *)(param_2 + 1) = 0;
  *(undefined2 *)(param_2 + 3) = 0;
  *(undefined2 *)((int)param_2 + 10) = 0;
  return 0;
}

