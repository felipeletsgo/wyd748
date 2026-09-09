// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ff400 | Name: FUN_004ff400


void __fastcall FUN_004ff400(int param_1)

{
  void *pvVar1;
  undefined4 local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined4 local_80;
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a19db;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004ffb1c();
  pvVar1 = operator_new(0xe4c);
  local_8 = 0;
  if (pvVar1 == (void *)0x0) {
    local_6c = 0;
  }
  else {
    local_6c = FUN_00401d03(0xffffffff,s_NoName_005bb730,0xffffffaa,0,0x44228000,0x43000000,
                            0x41800000,0,0x55aa0000,1,0);
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0x418) = local_6c;
  pvVar1 = operator_new(0xe4c);
  local_8 = 1;
  if (pvVar1 == (void *)0x0) {
    local_70 = 0;
  }
  else {
    local_70 = FUN_00401d03(0xffffffff,&DAT_0092ecb4,0xffff3300,0,0x44228000,0x43000000,0x41800000,0
                            ,0,1,0);
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0x41c) = local_70;
  if (DAT_005b892c == 2) {
    pvVar1 = operator_new(0xe4c);
    local_8 = 2;
    if (pvVar1 == (void *)0x0) {
      local_74 = 0;
    }
    else {
      local_74 = FUN_00401d03(0xffffffff,&DAT_0092ecb8,0xffffffff,0,0x44228000,0x430f0000,0x42480000
                              ,0,0xffffffff,1,0);
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x424) = local_74;
    pvVar1 = operator_new(0x1e4);
    local_8 = 3;
    if (pvVar1 == (void *)0x0) {
      local_78 = 0;
    }
    else {
      local_78 = FUN_00401466(0x1be,0xc1200000,0x441ec000,0x430f0000,0x42480000,0x77777777,4);
    }
    *(undefined4 *)(param_1 + 0x420) = local_78;
    *(undefined4 *)(*(int *)(param_1 + 0x420) + 0x3c) = 0;
  }
  else {
    pvVar1 = operator_new(0xe4c);
    local_8 = 4;
    if (pvVar1 == (void *)0x0) {
      local_7c = 0;
    }
    else {
      local_7c = FUN_00401d03(9,&DAT_0092ecbc,0xffffffff,0,0x44228000,0x430c0000,0x42200000,1,
                              0xffffffff,1,0);
    }
    *(undefined4 *)(param_1 + 0x424) = local_7c;
  }
  local_8 = 0xffffffff;
  pvVar1 = operator_new(0xe4c);
  local_8 = 5;
  if (pvVar1 == (void *)0x0) {
    local_80 = 0;
  }
  else {
    local_80 = FUN_00401d03(0xfffffffe,&DAT_0092ecc0,0xffffffff,0,0x44228000,0x43800000,0x42800000,1
                            ,0x77000000,1,0);
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0x42c) = local_80;
  *(undefined4 *)(*(int *)(param_1 + 0x42c) + 0x70) = 1;
  *(undefined4 *)(param_1 + 0x7e4) = 0;
  pvVar1 = operator_new(0x1e4);
  local_8 = 6;
  if (pvVar1 == (void *)0x0) {
    local_84 = 0;
  }
  else {
    local_84 = FUN_00401466(0xfffffffe,0,0,0x41200000,0x41200000,0x77777777,4);
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0x7e4) = local_84;
  pvVar1 = operator_new(0x374);
  local_8 = 7;
  if (pvVar1 == (void *)0x0) {
    local_88 = 0;
  }
  else {
    local_88 = FUN_00405065(0xfffffffe,0x1e,0x1e,0,0,0x42700000,0x40e00000,0xffff0000,0xff333333,1);
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0x434) = local_88;
  pvVar1 = operator_new(0x374);
  local_8 = 8;
  if (pvVar1 == (void *)0x0) {
    local_8c = 0;
  }
  else {
    local_8c = FUN_00405065(0xfffffffe,0x1e,0x1e,0,0,0x42700000,0x40e00000,0xffffaa00,0xff333333,1);
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0x438) = local_8c;
  (**(code **)(**(int **)(param_1 + 0x42c) + 0x60))(0);
  *(undefined4 *)(*(int *)(param_1 + 0x418) + 0xd1c) = 0xfffffffe;
  (**(code **)(**(int **)(param_1 + 0x418) + 0x60))(0);
  (**(code **)(**(int **)(param_1 + 0x41c) + 0x60))(0);
  if (*(int *)(param_1 + 0x7e4) != 0) {
    (**(code **)(**(int **)(param_1 + 0x7e4) + 0x60))(0);
  }
  (**(code **)(**(int **)(param_1 + 0x424) + 0x60))(0);
  pvVar1 = operator_new(0xe4c);
  local_8 = 9;
  if (pvVar1 == (void *)0x0) {
    local_90 = 0;
  }
  else {
    local_90 = FUN_00401d03(0xffffffff,&DAT_0092ecc4,0xffffffaa,0,0x44228000,0x43000000,0x41800000,0
                            ,0x55aa0000,1,0);
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0x428) = local_90;
  *(undefined4 *)(*(int *)(param_1 + 0x428) + 0xd1c) = 0xfffffffe;
  (**(code **)(**(int **)(param_1 + 0x428) + 0x60))(0);
  if (DAT_005b892c == 2) {
    (**(code **)(**(int **)(param_1 + 0x420) + 0x60))(0);
  }
  (**(code **)(**(int **)(param_1 + 0x434) + 0x60))(0);
  (**(code **)(**(int **)(param_1 + 0x438) + 0x60))(0);
  if (DAT_0067cf38 != 0) {
    (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x44))(*(undefined4 *)(param_1 + 0x418));
    (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x44))(*(undefined4 *)(param_1 + 0x41c));
    (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x44))(*(undefined4 *)(param_1 + 0x7e4));
    (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x44))(*(undefined4 *)(param_1 + 0x424));
    if (DAT_005b892c == 2) {
      (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x44))(*(undefined4 *)(param_1 + 0x420));
    }
    (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x44))(*(undefined4 *)(param_1 + 0x428));
    (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x44))(*(undefined4 *)(param_1 + 0x42c));
    (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x44))(*(undefined4 *)(param_1 + 0x434));
    (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x44))(*(undefined4 *)(param_1 + 0x438));
  }
  ExceptionList = local_10;
  return;
}

