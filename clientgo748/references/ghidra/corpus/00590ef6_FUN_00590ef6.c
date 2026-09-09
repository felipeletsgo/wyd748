// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00590ef6 | Name: FUN_00590ef6


undefined4 FUN_00590ef6(LPVOID param_1)

{
  LPVOID pvVar1;
  BOOL BVar2;
  void *local_14;
  undefined1 *puStack_10;
  undefined *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &DAT_005aa018;
  puStack_10 = &LAB_00596b2c;
  local_14 = ExceptionList;
  ExceptionList = &local_14;
  pvVar1 = TlsGetValue(DAT_005c9d60);
  if (pvVar1 != (LPVOID)0x0) {
    *(undefined4 *)((int)pvVar1 + 0x48) = *(undefined4 *)((int)param_1 + 0x48);
    *(undefined4 *)((int)pvVar1 + 0x4c) = *(undefined4 *)((int)param_1 + 0x4c);
    FUN_00593dba(param_1);
    param_1 = pvVar1;
  }
  BVar2 = TlsSetValue(DAT_005c9d60,param_1);
  if (BVar2 == 0) {
    __amsg_exit(0x10);
  }
  if (PTR_FUN_005c9930 != (undefined *)0x0) {
    (*(code *)PTR_FUN_005c9930)();
  }
  local_8 = 0;
  (**(code **)((int)param_1 + 0x48))(*(undefined4 *)((int)param_1 + 0x4c));
  FUN_00590fb3();
  ExceptionList = local_14;
  return 0;
}

