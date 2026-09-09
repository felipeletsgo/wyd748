// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040c2cd | Name: FUN_0040c2cd


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall FUN_0040c2cd(undefined4 *param_1,undefined4 param_2)

{
  void *pvVar1;
  undefined4 local_3c;
  undefined4 local_38;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059edef;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0054a9e0(0);
  local_8 = 0;
  FUN_0040bad0();
  param_1[0xb] = param_2;
  param_1[0x4b] = 0;
  param_1[0x4c] = 0;
  *param_1 = &PTR_FUN_005a3f34;
  param_1[9] = &PTR_FUN_005a3f30;
  pvVar1 = operator_new(0x358);
  local_8._0_1_ = 1;
  if (pvVar1 == (void *)0x0) {
    local_38 = 0;
  }
  else {
    local_38 = FUN_00409cbc(0,(float)*(uint *)(DAT_005ccf9c + 0x2a504) / _DAT_005a365c,
                            (float)*(uint *)(DAT_005ccf9c + 0x2a508) / _DAT_005a365c,0x42000000,
                            0x42000000);
  }
  local_8._0_1_ = 0;
  param_1[0xc] = local_38;
  pvVar1 = operator_new(100);
  local_8._0_1_ = 2;
  if (pvVar1 == (void *)0x0) {
    local_3c = 0;
  }
  else {
    local_3c = FUN_0040104c(0,0,0,0);
  }
  local_8 = (uint)local_8._1_3_ << 8;
  param_1[10] = local_3c;
  param_1[0xe] = 0;
  param_1[0xd] = 0;
  _memset(param_1 + 0xf,0,0xf0);
  _memset(param_1 + 0x4d,0,0x20);
  ExceptionList = local_10;
  return param_1;
}

