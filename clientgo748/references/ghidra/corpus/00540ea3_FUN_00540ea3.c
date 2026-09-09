// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00540ea3 | Name: FUN_00540ea3


undefined4 * __thiscall FUN_00540ea3(undefined4 *param_1,undefined4 param_2)

{
  void *pvVar1;
  undefined4 local_24;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a26ea;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0054a9e0(0);
  local_8 = 0;
  *param_1 = &PTR_FUN_005a5a94;
  param_1[0x12] = 0;
  param_1[10] = 0;
  param_1[9] = param_2;
  pvVar1 = operator_new(0x24);
  local_8._0_1_ = 1;
  if (pvVar1 == (void *)0x0) {
    local_24 = 0;
  }
  else {
    local_24 = FUN_0054a9e0(0);
  }
  local_8 = (uint)local_8._1_3_ << 8;
  param_1[0x11] = local_24;
  for (local_14 = 0; local_14 < 0x1000; local_14 = local_14 + 1) {
    param_1[local_14 + 0x13] = 0;
  }
  for (local_14 = 0; local_14 < 6; local_14 = local_14 + 1) {
    param_1[local_14 + 0xb] = 0;
  }
  FUN_0054ac09(param_1[0x11]);
  ExceptionList = local_10;
  return param_1;
}

