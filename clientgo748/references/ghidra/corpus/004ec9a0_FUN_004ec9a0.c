// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ec9a0 | Name: FUN_004ec9a0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall FUN_004ec9a0(undefined4 *param_1)

{
  undefined4 uVar1;
  int iVar2;
  void *pvVar3;
  undefined4 in_stack_00000010;
  int in_stack_00000014;
  int in_stack_00000018;
  int in_stack_0000001c;
  float fVar4;
  undefined4 uVar5;
  int *local_48;
  int *local_44;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a144f;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  *param_1 = &PTR_FUN_005a5138;
  param_1[0x23] = 0;
  FUN_004310a0(&stack0x00000004);
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x20] = uVar1;
  param_1[0x21] = 3000;
  if (DAT_0092e660 == 0) {
    for (local_14 = 0; local_14 < in_stack_00000014; local_14 = local_14 + 1) {
      if (in_stack_0000001c == 0) {
        iVar2 = _rand();
        iVar2 = iVar2 % 5;
        pvVar3 = operator_new(0x180);
        local_8._0_1_ = 1;
        if (pvVar3 == (void *)0x0) {
          local_44 = (int *)0x0;
          local_48 = local_44;
        }
        else {
          local_48 = (int *)FUN_004c8f70(0,local_14 * 400 + 0x5dc,
                                         (float)iVar2 * _DAT_005a4214 + _DAT_005a4158,
                                         (float)iVar2 * _DAT_005a4214 + _DAT_005a4158,
                                         (float)iVar2 * _DAT_005a4214 + _DAT_005a4158,0x3a83126f,1,
                                         0x50);
        }
      }
      else {
        iVar2 = _rand();
        iVar2 = iVar2 % 5 + 5;
        pvVar3 = operator_new(0x180);
        local_8._0_1_ = 2;
        if (pvVar3 == (void *)0x0) {
          local_48 = (int *)0x0;
        }
        else {
          local_48 = (int *)FUN_004c8f70(0,local_14 * 400 + 0x5dc,
                                         (float)iVar2 * _DAT_005a3834 + _DAT_005a3660,
                                         (float)iVar2 * _DAT_005a3834 + _DAT_005a3660,
                                         (float)iVar2 * _DAT_005a3834 + _DAT_005a3660,0x3a83126f,1,
                                         0x50);
        }
      }
      local_8 = (uint)local_8._1_3_ << 8;
      if (local_48 != (int *)0x0) {
        iVar2 = _rand();
        fVar4 = (float)(iVar2 % 10 + -5) * _DAT_005a4214 + (float)param_1[0x1f];
        uVar1 = param_1[0x1e];
        iVar2 = _rand();
        uVar1 = FUN_00430f50((float)(iVar2 % 10 + -5) * _DAT_005a4214 + (float)param_1[0x1d],uVar1,
                             fVar4);
        FUN_004310a0(uVar1);
        local_48[0x1b] = 1;
        local_48[0x40] = 1;
        (**(code **)(*local_48 + 0x3c))(in_stack_00000010);
        FUN_0054ac09(local_48);
      }
    }
  }
  if (((in_stack_00000018 == 1) && (DAT_005ccf98 != 0)) && (iVar2 = FUN_00429a6d(0x9c), iVar2 != 0))
  {
    uVar5 = 0;
    uVar1 = 0;
    FUN_00429a6d(0x9c);
    FUN_0042ad2b(uVar1,uVar5);
  }
  ExceptionList = local_10;
  return param_1;
}

