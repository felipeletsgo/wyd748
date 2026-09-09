// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058ffd8 | Name: FUN_0058ffd8


void FUN_0058ffd8(LPVOID param_1)

{
  int iVar1;
  undefined4 local_2c;
  int local_28;
  undefined4 local_24;
  int local_20;
  void *local_14;
  undefined1 *puStack_10;
  undefined *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &DAT_005a9fa8;
  puStack_10 = &LAB_00596b2c;
  local_14 = ExceptionList;
  if (param_1 == (LPVOID)0x0) {
    return;
  }
  if (DAT_013b8608 == 3) {
    ExceptionList = &local_14;
    FUN_00596aad(9);
    local_8 = 0;
    local_20 = FUN_005958b8(param_1);
    if (local_20 != 0) {
      FUN_005958e3(local_20,param_1);
    }
    local_8 = 0xffffffff;
    FUN_00590042();
    iVar1 = local_20;
  }
  else {
    ExceptionList = &local_14;
    if (DAT_013b8608 != 2) goto LAB_005900a4;
    ExceptionList = &local_14;
    FUN_00596aad(9);
    local_8 = 1;
    local_28 = FUN_00596613(param_1,&local_2c,&local_24);
    if (local_28 != 0) {
      FUN_0059666a(local_2c,local_24,local_28);
    }
    local_8 = 0xffffffff;
    FUN_0059009a();
    iVar1 = local_28;
  }
  if (iVar1 != 0) {
    ExceptionList = local_14;
    return;
  }
LAB_005900a4:
  HeapFree(DAT_013b8604,0,param_1);
  ExceptionList = local_14;
  return;
}

