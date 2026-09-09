// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b1ea9 | Name: FUN_004b1ea9


undefined4 * __fastcall FUN_004b1ea9(undefined4 *param_1)

{
  void *pvVar1;
  undefined4 local_30;
  undefined4 local_2c;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0446;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  param_1[0x6c1f] = 0;
  param_1[0x6c21] = 0;
  param_1[0x6c22] = 0;
  *param_1 = &PTR_FUN_005a45fc;
  FUN_004b17dd();
  (**(code **)(*DAT_013b71e0 + 0x18))(param_1);
  FUN_004b222b();
  FUN_004b2340();
  FUN_004b229e();
  param_1[0x6bd2] = 0;
  pvVar1 = operator_new(0x24);
  local_8 = 0;
  if (pvVar1 == (void *)0x0) {
    local_2c = 0;
  }
  else {
    local_2c = FUN_0054a9e0(0);
  }
  local_8 = 0xffffffff;
  param_1[0x6c1f] = local_2c;
  pvVar1 = operator_new(0xc4);
  local_8 = 1;
  if (pvVar1 == (void *)0x0) {
    local_30 = 0;
  }
  else {
    local_30 = FUN_004ef840();
  }
  local_8 = 0xffffffff;
  param_1[0x6c21] = local_30;
  FUN_0054ac09(param_1[0x6c21]);
  DAT_0067cf38 = 0;
  param_1[0x6c20] = 0xffffffff;
  param_1[0x6bd3] = 0xffffffff;
  param_1[0x6bd4] = 0xffffffff;
  param_1[0x6c24] = 0;
  param_1[0x3c3] = 0;
  param_1[0x315] = 0;
  param_1[0x314] = 0;
  *(undefined1 *)(param_1 + 0x3bd) = 0xff;
  *(undefined1 *)((int)param_1 + 0xef5) = 0;
  _memset(param_1 + 1,0,0x2e8);
  _memset(param_1 + 0xbb,0,0x400);
  _memset(param_1 + 0x1bb,0,0x2f4);
  _memset(param_1 + 0x278,0,0x270);
  _memset(param_1 + 0x3c6,0,0x30);
  for (local_14 = 0; local_14 < 0x14; local_14 = local_14 + 1) {
    *(undefined1 *)((int)param_1 + local_14 + 0xef6) = 0xff;
  }
  param_1[0x6c23] = 0;
  param_1[0x7909] = 1;
  param_1[0x790a] = 0;
  for (local_14 = 0; local_14 < 4; local_14 = local_14 + 1) {
    *(undefined1 *)((int)param_1 + local_14 + 0xf14) = 0;
  }
  ExceptionList = local_10;
  return param_1;
}

