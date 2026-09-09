// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005927ac | Name: entry


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void entry(void)

{
  DWORD DVar1;
  int iVar2;
  undefined4 uVar3;
  uint uVar4;
  HMODULE pHVar5;
  undefined4 uVar6;
  _STARTUPINFOA local_60;
  undefined1 *local_1c;
  undefined4 *local_18;
  void *pvStack_14;
  undefined1 *puStack_10;
  undefined *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &DAT_005aa040;
  puStack_10 = &LAB_00596b2c;
  pvStack_14 = ExceptionList;
  local_1c = &stack0xffffff88;
  ExceptionList = &pvStack_14;
  DVar1 = GetVersion();
  _DAT_013b7ff4 = DVar1 >> 8 & 0xff;
  _DAT_013b7ff0 = DVar1 & 0xff;
  _DAT_013b7fec = _DAT_013b7ff0 * 0x100 + _DAT_013b7ff4;
  _DAT_013b7fe8 = DVar1 >> 0x10;
  iVar2 = FUN_00595813(1);
  if (iVar2 == 0) {
    FUN_005928d9(0x1c);
  }
  iVar2 = FUN_00593cec();
  if (iVar2 == 0) {
    FUN_005928d9(0x10);
  }
  local_8 = 0;
  FUN_00594f10();
  DAT_013b872c = GetCommandLineA();
  DAT_013b8028 = FUN_00599ee9();
  FUN_00599c9c();
  FUN_00599be3();
  FUN_0059268c();
  local_60.dwFlags = 0;
  GetStartupInfoA(&local_60);
  uVar3 = FUN_00599b8b();
  if ((local_60.dwFlags & 1) == 0) {
    uVar4 = 10;
  }
  else {
    uVar4 = (uint)local_60.wShowWindow;
  }
  uVar6 = 0;
  pHVar5 = GetModuleHandleA((LPCSTR)0x0);
  uVar3 = FUN_0055f7f9(pHVar5,uVar6,uVar3,uVar4);
  FUN_005926b9(uVar3);
  FUN_00597c52(*(undefined4 *)*local_18,local_18);
  return;
}

