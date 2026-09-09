// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ed2a0 | Name: FUN_004ed2a0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_004ed2a0(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            int param_5,int param_6)

{
  undefined4 uVar1;
  void *pvVar2;
  undefined4 *puVar3;
  undefined4 local_54;
  undefined4 local_50;
  int *local_4c;
  int local_18;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a14ba;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  *param_1 = &PTR_FUN_005a51b0;
  param_1[0x23] = param_5;
  param_1[7] = param_6;
  FUN_004310a0(&param_2);
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x20] = uVar1;
  param_1[0x21] = 2000;
  if (param_1[0x23] == 1) {
    param_1[0x21] = 0x5dc;
  }
  if (param_6 == 0) {
    param_1[0x22] = 0;
  }
  else {
    param_1[0x22] = *(undefined4 *)(param_6 + 0x34);
  }
  if (param_5 == 0) {
    for (local_18 = 0; local_18 < 3; local_18 = local_18 + 1) {
      pvVar2 = operator_new(0x22c);
      local_8._0_1_ = 1;
      if (pvVar2 == (void *)0x0) {
        local_4c = (int *)0x0;
      }
      else {
        local_4c = (int *)FUN_004d5480(0x56,param_2,param_3,param_4,param_2,param_3,param_4,3,
                                       param_6);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      if (local_4c != (int *)0x0) {
        *(undefined2 *)(local_4c + 0x59) = 0;
        *(undefined2 *)(local_4c + 0x58) = 0;
        *(undefined2 *)((int)local_4c + 0x166) = 0;
        *(undefined2 *)((int)local_4c + 0x162) = 0;
        local_4c[0x67] = 10;
        (**(code **)(*local_4c + 0x78))(0);
        local_4c[100] = param_1[0x20] + local_18 * 400;
        if (local_4c[0x18] != 0) {
          *(int *)(local_4c[0x18] + 0x2e8) = local_4c[100];
        }
        local_4c[0x65] = param_1[0x21] + local_18 * -200;
        *(undefined4 *)(local_4c[0x18] + 0x24) = 0x3f800000;
        *(undefined4 *)(local_4c[0x18] + 0x28) = 0x3f800000;
        *(undefined4 *)(local_4c[0x18] + 0x2c) = 0x3f800000;
        local_4c[0x66] = 2;
        local_4c[0x69] = (int)((float)local_18 * _DAT_005a4154 + _DAT_005a4158);
        local_4c[0x6a] = (int)((float)local_18 * _DAT_005a4154 + _DAT_005a4158);
        local_4c[0x6b] = (int)((float)local_18 * _DAT_005a4154 + _DAT_005a4158);
        local_4c[0x6d] = (int)((float)local_18 * _DAT_005a4154 + _DAT_005a4158);
        local_4c[0x6e] = (int)((float)local_18 * _DAT_005a4154 + _DAT_005a4158);
        local_4c[0x6f] = (int)((float)local_18 * _DAT_005a4154 + _DAT_005a4158);
        local_4c[0x75] = 2;
        local_4c[0x7d] = param_1[0x22];
        FUN_0054ac09(local_4c);
      }
    }
  }
  if (param_1[0x23] == 0) {
    param_1[0x25] = 0;
    pvVar2 = operator_new(0xc0);
    local_8._0_1_ = 2;
    if (pvVar2 == (void *)0x0) {
      local_50 = 0;
    }
    else {
      local_50 = FUN_004df250(6,7,0x3f800000);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x25] = local_50;
    if (param_1[0x25] != 0) {
      (**(code **)(*(int *)param_1[0x25] + 0x44))(0xffff0000);
      *(undefined4 *)(param_1[0x25] + 0x6c) = 1;
      puVar3 = (undefined4 *)FUN_00430f20(param_2,param_4);
      (**(code **)(*(int *)param_1[0x25] + 0x3c))(*puVar3,puVar3[1]);
      *(undefined4 *)(param_1[0x25] + 0xa0) = 0;
      FUN_0054ac09(param_1[0x25]);
    }
  }
  else if (param_1[0x23] == 1) {
    param_1[0x25] = 0;
    pvVar2 = operator_new(0xc0);
    local_8._0_1_ = 3;
    if (pvVar2 == (void *)0x0) {
      local_54 = 0;
    }
    else {
      local_54 = FUN_004df250(3,7,0x3f800000);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x25] = local_54;
    if (param_1[0x25] != 0) {
      (**(code **)(*(int *)param_1[0x25] + 0x44))(0x33555555);
      *(undefined4 *)(param_1[0x25] + 0x6c) = 1;
      puVar3 = (undefined4 *)FUN_00430f20(param_2,param_4);
      (**(code **)(*(int *)param_1[0x25] + 0x3c))(*puVar3,puVar3[1]);
      *(undefined4 *)(param_1[0x25] + 0xa0) = 0;
      FUN_0054ac09(param_1[0x25]);
    }
  }
  param_1[0x26] = 0;
  ExceptionList = local_10;
  return param_1;
}

