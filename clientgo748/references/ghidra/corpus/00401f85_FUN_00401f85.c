// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00401f85 | Name: FUN_00401f85


void __fastcall FUN_00401f85(undefined4 *param_1)

{
  int iVar1;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_0059e95a;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a35c8;
  local_8 = 4;
  if ((DAT_0067cf38 != 0) && (iVar1 = *(int *)(DAT_0067cf38 + 0x28), iVar1 != 0)) {
    if (-1 < (int)param_1[0x38a]) {
      FUN_0040c1b0(iVar1 + 0x3c,param_1 + 0x338,param_1[0x38a]);
    }
    if (-1 < (int)param_1[0x21b]) {
      FUN_0040c1b0(iVar1 + 0x3c,param_1 + 0x1c9,param_1[0x21b]);
    }
    if (*(char *)((int)param_1 + 0xcdd) == '\x01') {
      if (-1 < (int)param_1[0x276]) {
        FUN_0040c1b0(iVar1 + 0x3c,param_1 + 0x224,param_1[0x276]);
      }
      if (-1 < (int)param_1[0x2d1]) {
        FUN_0040c1b0(iVar1 + 0x3c,param_1 + 0x27f,param_1[0x2d1]);
      }
      if (-1 < (int)param_1[0x32c]) {
        FUN_0040c1b0(iVar1 + 0x3c,param_1 + 0x2da,param_1[0x32c]);
      }
    }
  }
  local_8._0_1_ = 3;
  FUN_00421a4b();
  local_8._0_1_ = 2;
  FUN_00421a4b();
  local_8._0_1_ = 1;
  FUN_00421a4b();
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00421a4b();
  local_8 = 0xffffffff;
  FUN_00401152();
  ExceptionList = local_10;
  return;
}

