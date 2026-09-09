// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058f7bc | Name: FUN_0058f7bc


undefined4 FUN_0058f7bc(uint param_1)

{
  int iVar1;
  int iVar2;
  HANDLE hObject;
  BOOL BVar3;
  DWORD DVar4;
  undefined4 uVar5;
  
  iVar1 = FUN_00594e4d(param_1);
  if (iVar1 != -1) {
    if ((param_1 == 1) || (param_1 == 2)) {
      iVar1 = FUN_00594e4d(2);
      iVar2 = FUN_00594e4d(1);
      if (iVar2 == iVar1) goto LAB_0058f80a;
    }
    hObject = (HANDLE)FUN_00594e4d(param_1);
    BVar3 = CloseHandle(hObject);
    if (BVar3 == 0) {
      DVar4 = GetLastError();
      goto LAB_0058f80c;
    }
  }
LAB_0058f80a:
  DVar4 = 0;
LAB_0058f80c:
  FUN_00594dce(param_1);
  *(undefined1 *)((&DAT_013b8620)[(int)param_1 >> 5] + 4 + (param_1 & 0x1f) * 0x24) = 0;
  if (DVar4 == 0) {
    uVar5 = 0;
  }
  else {
    FUN_00594baa(DVar4);
    uVar5 = 0xffffffff;
  }
  return uVar5;
}

