// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00407d62 | Name: FUN_00407d62


undefined4 * __thiscall
FUN_00407d62(undefined4 *param_1,undefined4 param_2,char *param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,int param_9)

{
  int iVar1;
  void *pvVar2;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059ec95;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00407203(param_3,0xffffffff,0,0,0x43fa0000,0x41400000,0,0x77777777,1,0);
  local_8 = 0;
  *param_1 = &PTR_FUN_005a3bc8;
  local_14 = 0xffffffff;
  iVar1 = _strcmp(param_3,&DAT_00a39978);
  if ((iVar1 != 0) && ((*param_3 < '0' || ('9' < *param_3)))) {
    local_14 = 0xffaaffaa;
  }
  if (param_9 == 1) {
    pvVar2 = operator_new(0xe4c);
    local_8._0_1_ = 1;
    if (pvVar2 == (void *)0x0) {
      local_44 = 0;
    }
    else {
      local_44 = FUN_00401d03(0xfffffffe,param_4,local_14,0x42300000,0,0x43820000,0x41800000,1,
                              param_8,1,1);
    }
    param_1[0x394] = local_44;
  }
  else {
    pvVar2 = operator_new(0xe4c);
    local_8._0_1_ = 2;
    if (pvVar2 == (void *)0x0) {
      local_48 = 0;
    }
    else {
      local_48 = FUN_00401d03(0xfffffffe,param_4,local_14,0x42300000,0,0x43820000,0x41800000,1,
                              param_8,1,0);
    }
    param_1[0x394] = local_48;
  }
  local_8._0_1_ = 0;
  pvVar2 = operator_new(0xe4c);
  local_8._0_1_ = 3;
  if (pvVar2 == (void *)0x0) {
    local_4c = 0;
  }
  else {
    local_4c = FUN_00401d03(0xfffffffe,param_5,local_14,0x43988000,0,0x429e0000,0x41800000,1,param_8
                            ,1,1);
  }
  local_8._0_1_ = 0;
  param_1[0x395] = local_4c;
  pvVar2 = operator_new(0xe4c);
  local_8._0_1_ = 4;
  if (pvVar2 == (void *)0x0) {
    local_50 = 0;
  }
  else {
    local_50 = FUN_00401d03(0xfffffffe,param_6,local_14,0x43c08000,0,0x42080000,0x41800000,1,param_8
                            ,1,1);
  }
  local_8._0_1_ = 0;
  param_1[0x396] = local_50;
  pvVar2 = operator_new(0xe4c);
  local_8._0_1_ = 5;
  if (pvVar2 == (void *)0x0) {
    local_54 = 0;
  }
  else {
    local_54 = FUN_00401d03(0xfffffffe,param_7,local_14,0x43d20000,0,0x42a00000,0x41800000,1,param_8
                            ,1,1);
  }
  local_8 = (uint)local_8._1_3_ << 8;
  param_1[0x397] = local_54;
  FUN_0058f078(param_1 + 0x398,&DAT_005b10c4,param_2);
  ExceptionList = local_10;
  return param_1;
}

