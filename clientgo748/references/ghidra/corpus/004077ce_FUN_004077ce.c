// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004077ce | Name: FUN_004077ce


undefined4 * __thiscall
FUN_004077ce(undefined4 *param_1,char *param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,
            undefined4 param_9,undefined4 param_10,undefined4 param_11,undefined4 param_12)

{
  void *pvVar1;
  int iVar2;
  undefined4 local_a0;
  undefined4 local_9c;
  undefined4 local_98;
  undefined1 local_78 [64];
  int local_38;
  undefined *local_34;
  undefined *local_30;
  char *local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined1 local_20 [16];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059ec38;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00407203(param_2,param_3,param_4,param_5,param_6,param_7,0,0x77777777,1,0);
  local_8 = 0;
  param_1[0x394] = param_8;
  param_1[0x395] = param_9;
  param_1[0x396] = param_10;
  param_1[0x397] = 0;
  param_1[0x398] = 0;
  param_1[0x39a] = 0;
  *param_1 = &PTR_FUN_005a3b38;
  local_34 = &DAT_005b10b8;
  local_30 = &DAT_005b10bc;
  local_28 = 0xffaaffff;
  local_24 = 0xffffaaff;
  FUN_0058f078(local_20,&DAT_005b10c0,param_1[0x396] + 1);
  pvVar1 = operator_new(0xe4c);
  local_8._0_1_ = 1;
  if (pvVar1 == (void *)0x0) {
    local_98 = 0;
  }
  else {
    local_98 = FUN_00401d03(0xffffffff,local_20,0xffffffaa,0x42940000,0x40400000,0x41000000,
                            0x41400000,0,0x77777777,1,0);
  }
  local_8._0_1_ = 0;
  param_1[0x398] = local_98;
  pvVar1 = operator_new(0x374);
  local_8._0_1_ = 2;
  if (pvVar1 == (void *)0x0) {
    local_9c = 0;
  }
  else {
    local_9c = FUN_00405065(7,param_11,param_12,0,0x41700000,0x42d00000,0x41000000,0xffffffff,
                            0xffffffff,1);
  }
  local_8._0_1_ = 0;
  param_1[0x39a] = local_9c;
  pvVar1 = operator_new(0x1e4);
  local_8._0_1_ = 3;
  if (pvVar1 == (void *)0x0) {
    local_a0 = 0;
  }
  else {
    local_a0 = FUN_00401466(3,0x42840000,0,0x41800000,0x41800000,0xffffffff,4);
  }
  local_8 = (uint)local_8._1_3_ << 8;
  param_1[0x399] = local_a0;
  local_2c = _strchr(param_2,0x5e);
  if (local_2c != (char *)0x0) {
    local_38 = (int)local_2c - (int)param_2;
    iVar2 = FUN_00421b0f(param_2,local_38);
    if (iVar2 == 0) {
      local_2c = (char *)0x0;
    }
  }
  if (local_2c == (char *)0x0) {
    FUN_00402309(param_2,0);
  }
  else {
    _memset(local_78,0,0x40);
    FUN_0058f220(local_78,param_2,(int)local_2c - (int)param_2);
    FUN_00402309(local_78,0);
  }
  ExceptionList = local_10;
  return param_1;
}

