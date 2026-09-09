// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d8020 | Name: FUN_004d8020


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall FUN_004d8020(undefined4 *param_1)

{
  undefined1 uVar1;
  undefined4 uVar2;
  void *pvVar3;
  undefined4 *puVar4;
  int iVar5;
  int in_stack_00000010;
  undefined4 in_stack_00000020;
  undefined4 in_stack_00000024;
  undefined4 in_stack_00000028;
  undefined4 in_stack_0000002c;
  float in_stack_00000030;
  undefined4 uVar6;
  undefined4 local_78;
  undefined4 local_70;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0b8f;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8._0_1_ = 0;
  local_8._1_3_ = 0;
  CSimpleArray<>();
  CSimpleArray<>();
  *param_1 = &PTR_FUN_005a4ae4;
  uVar2 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x2e] = uVar2;
  param_1[7] = in_stack_00000010;
  param_1[0x21] = 0x3f800000;
  param_1[0x20] = in_stack_00000028;
  param_1[0x2c] = in_stack_00000020;
  param_1[0x2d] = in_stack_00000024;
  if ((int)param_1[0x36] < 6) {
    param_1[0x36] = in_stack_0000002c;
  }
  else {
    param_1[0x36] = 5;
  }
  uVar2 = FUN_004310a0(&stack0x00000004);
  FUN_004310a0(uVar2);
  FUN_004310a0(&stack0x00000014);
  if (param_1[7] != 0) {
    uVar2 = FUN_00430f50(*(undefined4 *)(param_1[7] + 0x28),
                         *(float *)(param_1[7] + 0x30) + _DAT_005a3660,
                         *(undefined4 *)(param_1[7] + 0x2c));
    FUN_004310a0(uVar2);
  }
  param_1[0x2f] = 900;
  CSimpleArray<>();
  CSimpleArray<>();
  FUN_004310a0(param_1 + 0x30);
  uVar2 = FUN_00430f50(in_stack_00000030 * (float)param_1[0x33] +
                       (_DAT_005a3660 - in_stack_00000030) * (float)param_1[0x30],
                       in_stack_00000030 * (float)param_1[0x34] +
                       (_DAT_005a3660 - in_stack_00000030) * (float)param_1[0x31],
                       in_stack_00000030 * (float)param_1[0x35] +
                       (_DAT_005a3660 - in_stack_00000030) * (float)param_1[0x32]);
  FUN_004310a0(uVar2);
  for (local_14 = 0; local_14 < (int)param_1[0x36]; local_14 = local_14 + 1) {
    pvVar3 = operator_new(0x184);
    local_8._0_1_ = 1;
    if (pvVar3 == (void *)0x0) {
      local_70 = 0;
    }
    else {
      local_70 = FUN_004ccbe0(local_2c,local_28,local_24,local_20,local_1c,local_18,0x80,
                              param_1[0x2c],1000,_DAT_005a415c * (float)param_1[0x20],
                              _DAT_005a415c * (float)param_1[0x20]);
    }
    local_8._0_1_ = 0;
    uVar1 = (undefined1)local_8;
    local_8._0_1_ = 0;
    param_1[local_14 + 0x22] = local_70;
    if (param_1[local_14 + 0x22] != 0) {
      *(undefined4 *)(param_1[local_14 + 0x22] + 0x6c) = 1;
      *(undefined4 *)(param_1[local_14 + 0x22] + 0x17c) = 1;
      FUN_004310a0(&local_20);
      in_stack_00000030 = (float)(local_14 + 1) / (float)(int)param_1[0x36];
      uVar2 = FUN_00430f50(in_stack_00000030 * (float)param_1[0x33] +
                           (_DAT_005a3660 - in_stack_00000030) * (float)param_1[0x30],
                           in_stack_00000030 * (float)param_1[0x34] +
                           (_DAT_005a3660 - in_stack_00000030) * (float)param_1[0x31],
                           in_stack_00000030 * (float)param_1[0x35] +
                           (_DAT_005a3660 - in_stack_00000030) * (float)param_1[0x32]);
      FUN_004310a0(uVar2);
      FUN_0054ac09(param_1[local_14 + 0x22]);
      uVar1 = (undefined1)local_8;
      if (in_stack_00000010 != 0) {
        pvVar3 = operator_new(0xc0);
        local_8._0_1_ = 2;
        if (pvVar3 == (void *)0x0) {
          local_78 = 0;
        }
        else {
          local_78 = FUN_004df250(3,7,0x3f800000);
        }
        local_8._0_1_ = 0;
        param_1[local_14 + 0x27] = local_78;
        uVar1 = (undefined1)local_8;
        if (param_1[local_14 + 0x27] != 0) {
          *(undefined4 *)(param_1[local_14 + 0x27] + 0x6c) = 1;
          (**(code **)(*(int *)param_1[local_14 + 0x27] + 0x44))(param_1[0x2d]);
          puVar4 = (undefined4 *)FUN_00430f20(local_2c,local_24);
          (**(code **)(*(int *)param_1[local_14 + 0x27] + 0x3c))(*puVar4,puVar4[1]);
          *(undefined4 *)(param_1[local_14 + 0x27] + 0xa0) = 1000;
          FUN_0054ac09(param_1[local_14 + 0x27]);
          uVar1 = (undefined1)local_8;
        }
      }
    }
    local_8._0_1_ = uVar1;
  }
  if (((in_stack_00000010 != 0) && (DAT_005ccf98 != 0)) && (iVar5 = FUN_00429a6d(0xb2), iVar5 != 0))
  {
    FUN_00429a6d(0xb2);
    iVar5 = FUN_0042afd0();
    if (iVar5 == 0) {
      uVar6 = 0;
      uVar2 = 0;
      FUN_00429a6d(0xb2);
      FUN_0042ad2b(uVar2,uVar6);
    }
  }
  ExceptionList = local_10;
  return param_1;
}

