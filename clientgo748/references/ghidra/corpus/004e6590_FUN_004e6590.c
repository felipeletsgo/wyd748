// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e6590 | Name: FUN_004e6590


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall FUN_004e6590(undefined4 *param_1)

{
  undefined4 uVar1;
  void *pvVar2;
  int iVar3;
  undefined4 in_stack_00000010;
  float in_stack_00000014;
  undefined4 uVar4;
  int local_f4;
  int local_f0;
  undefined4 local_ec;
  undefined4 local_e8;
  undefined4 local_e4;
  undefined4 local_e0;
  undefined4 local_dc;
  undefined4 local_d8;
  undefined4 local_d4;
  undefined4 local_28;
  float local_24;
  undefined4 local_20;
  undefined4 local_1c;
  int local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a11ae;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  *param_1 = &PTR_FUN_005a5040;
  param_1[0x20] = in_stack_00000010;
  FUN_004310a0(&stack0x00000004);
  param_1[0x21] = 0;
  param_1[0x22] = 0;
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x23] = uVar1;
  param_1[0x24] = 300;
  local_24 = 1.0;
  if (param_1[0x20] == 1) {
    local_24 = 0.5;
  }
  local_28 = 3000;
  local_14 = 0x88333355;
  local_1c = 0x88555555;
  local_20 = 0x7c;
  local_18 = 5000;
  if (param_1[0x20] == 1) {
    local_14 = 0x88335533;
    local_28 = 0;
  }
  else if (param_1[0x20] == 2) {
    local_14 = 0x883333ff;
    local_1c = 0x885555ff;
    local_28 = 0x5dc;
    local_20 = 0x1a0;
    local_24 = 0.7;
    local_18 = 0;
  }
  else if (param_1[0x20] == 3) {
    local_14 = 0x883333ff;
    local_1c = 0xffaaaaaa;
    local_28 = 3000;
    local_20 = 0x1a2;
    local_24 = 0.7;
    local_18 = 600;
  }
  else if (param_1[0x20] == 4) {
    local_14 = 0x88227777;
    local_1c = 0xff77ffff;
    local_28 = 3000;
    local_20 = 0x1a5;
    local_24 = 0.3;
    local_18 = 0;
  }
  else if (param_1[0x20] == 5) {
    local_14 = 0x66995533;
    local_28 = 10000;
    local_24 = 8.0;
    local_18 = 8000;
  }
  else {
    if (param_1[0x20] == 6) {
      pvVar2 = operator_new(0x124);
      local_8._0_1_ = 1;
      if (pvVar2 == (void *)0x0) {
        local_d4 = 0;
      }
      else {
        local_d4 = FUN_004cb900(0x1a2,3000,_DAT_005a366c * 0.5,_DAT_005a366c * 0.5,
                                _DAT_005a366c * 0.5,0,1000);
      }
      local_8._0_1_ = 0;
      param_1[0x21] = local_d4;
      if (param_1[0x21] != 0) {
        *(undefined4 *)(param_1[0x21] + 0x104) = 1;
        *(undefined4 *)(param_1[0x21] + 0x6c) = 1;
        uVar1 = FUN_00430f50(param_1[0x1d],(float)param_1[0x1e] + _DAT_005a503c,param_1[0x1f]);
        FUN_004310a0(uVar1);
        (**(code **)(*(int *)param_1[0x21] + 0x3c))(0x4433ff33);
        FUN_0054ac09(param_1[0x21]);
      }
      pvVar2 = operator_new(0x124);
      local_8._0_1_ = 2;
      if (pvVar2 == (void *)0x0) {
        local_d8 = 0;
      }
      else {
        local_d8 = FUN_004cb900(0x1a2,3000,_DAT_005a366c * 0.8,_DAT_005a366c * 0.8,
                                _DAT_005a366c * 0.8,0,0xfffff830);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      param_1[0x22] = local_d8;
      if (param_1[0x22] == 0) {
        ExceptionList = local_10;
        return param_1;
      }
      *(undefined4 *)(param_1[0x22] + 0x104) = 1;
      *(undefined4 *)(param_1[0x22] + 0x6c) = 1;
      uVar1 = FUN_00430f50(param_1[0x1d],(float)param_1[0x1e] + _DAT_005a5038,param_1[0x1f]);
      FUN_004310a0(uVar1);
      (**(code **)(*(int *)param_1[0x22] + 0x3c))(0xffaaffaa);
      FUN_0054ac09(param_1[0x22]);
      ExceptionList = local_10;
      return param_1;
    }
    if (param_1[0x20] == 7) {
      local_14 = 0xffaaaaaa;
      local_1c = 0xffaaaaaa;
      local_28 = 0x5dc;
      local_20 = 0x1a2;
      local_24 = _DAT_005a4160 * in_stack_00000014;
      local_18 = 600;
    }
    else if (param_1[0x20] == 8) {
      local_14 = 0x88227777;
      local_1c = 0xff77ffff;
      local_28 = 0x5dc;
      local_20 = 0x1a5;
      local_24 = _DAT_005a40ec * in_stack_00000014;
      local_18 = 500;
    }
  }
  pvVar2 = operator_new(0x124);
  local_8._0_1_ = 3;
  if (pvVar2 == (void *)0x0) {
    local_dc = 0;
  }
  else {
    local_dc = FUN_004cb900(local_20,local_28,_DAT_005a366c * local_24,_DAT_005a366c * local_24,
                            _DAT_005a366c * local_24,0,local_18);
  }
  local_8 = (uint)local_8._1_3_ << 8;
  param_1[0x21] = local_dc;
  if (param_1[0x21] != 0) {
    *(undefined4 *)(param_1[0x21] + 0x104) = 1;
    *(undefined4 *)(param_1[0x21] + 0x6c) = 1;
    uVar1 = FUN_00430f50(param_1[0x1d],(float)param_1[0x1e] + _DAT_005a5034,param_1[0x1f]);
    FUN_004310a0(uVar1);
    (**(code **)(*(int *)param_1[0x21] + 0x3c))(local_14);
    FUN_0054ac09(param_1[0x21]);
  }
  if (param_1[0x20] == 3) {
    local_18 = __ftol();
    local_18 = -local_18;
  }
  else if (param_1[0x20] == 7) {
    local_18 = __ftol();
    local_18 = -local_18;
  }
  else if (param_1[0x20] == 8) {
    local_18 = __ftol();
    local_18 = -local_18;
  }
  pvVar2 = operator_new(0x124);
  local_8._0_1_ = 4;
  if (pvVar2 == (void *)0x0) {
    local_e0 = 0;
  }
  else {
    local_e0 = FUN_004cb900(local_20,local_28,_DAT_005a366c * local_24,_DAT_005a366c * local_24,
                            _DAT_005a366c * local_24,0,-local_18);
  }
  local_8._0_1_ = 0;
  param_1[0x22] = local_e0;
  if (param_1[0x22] != 0) {
    *(undefined4 *)(param_1[0x22] + 0x104) = 1;
    *(undefined4 *)(param_1[0x22] + 0x6c) = 1;
    uVar1 = FUN_00430f50(param_1[0x1d],(float)param_1[0x1e] + _DAT_005a40ec,param_1[0x1f]);
    FUN_004310a0(uVar1);
    (**(code **)(*(int *)param_1[0x22] + 0x3c))(local_1c);
    FUN_0054ac09(param_1[0x22]);
  }
  if (param_1[0x20] == 3) {
    local_24 = local_24 * _DAT_005a34a0;
    iVar3 = -local_18;
    pvVar2 = operator_new(0x124);
    local_8._0_1_ = 5;
    if (pvVar2 == (void *)0x0) {
      local_e4 = 0;
    }
    else {
      local_e4 = FUN_004cb900(0x1a3,local_28,_DAT_005a366c * local_24,_DAT_005a366c * local_24,
                              _DAT_005a366c * local_24,0,local_18);
    }
    local_8._0_1_ = 0;
    param_1[0x22] = local_e4;
    local_18 = iVar3;
    if (param_1[0x22] != 0) {
      *(undefined4 *)(param_1[0x22] + 0x104) = 1;
      *(undefined4 *)(param_1[0x22] + 0x6c) = 1;
      uVar1 = FUN_00430f50(param_1[0x1d],(float)param_1[0x1e] + _DAT_005a40ec,param_1[0x1f]);
      FUN_004310a0(uVar1);
      (**(code **)(*(int *)param_1[0x22] + 0x3c))(local_1c);
      FUN_0054ac09(param_1[0x22]);
    }
  }
  if (param_1[0x20] == 7) {
    iVar3 = -local_18;
    pvVar2 = operator_new(0x124);
    local_8._0_1_ = 6;
    if (pvVar2 == (void *)0x0) {
      local_e8 = 0;
    }
    else {
      local_e8 = FUN_004cb900(0x1a3,local_28,_DAT_005a4104 * local_24,_DAT_005a4104 * local_24,
                              _DAT_005a4104 * local_24,0,local_18);
    }
    local_8._0_1_ = 0;
    param_1[0x22] = local_e8;
    local_18 = iVar3;
    if (param_1[0x22] != 0) {
      *(undefined4 *)(param_1[0x22] + 0x104) = 1;
      *(undefined4 *)(param_1[0x22] + 0x6c) = 1;
      uVar1 = FUN_00430f50(param_1[0x1d],(float)param_1[0x1e] + _DAT_005a40ec,param_1[0x1f]);
      FUN_004310a0(uVar1);
      (**(code **)(*(int *)param_1[0x22] + 0x3c))(local_14);
      FUN_0054ac09(param_1[0x22]);
    }
  }
  if (param_1[0x20] == 8) {
    pvVar2 = operator_new(0x124);
    local_8._0_1_ = 7;
    if (pvVar2 == (void *)0x0) {
      local_ec = 0;
    }
    else {
      local_ec = FUN_004cb900(0x1a3,local_28,_DAT_005a4104 * local_24,_DAT_005a4104 * local_24,
                              _DAT_005a4104 * local_24,0,local_18);
    }
    local_8._0_1_ = 0;
    param_1[0x22] = local_ec;
    if (param_1[0x22] != 0) {
      *(undefined4 *)(param_1[0x22] + 0x104) = 1;
      *(undefined4 *)(param_1[0x22] + 0x6c) = 1;
      uVar1 = FUN_00430f50(param_1[0x1d],(float)param_1[0x1e] + _DAT_005a40ec,param_1[0x1f]);
      FUN_004310a0(uVar1);
      (**(code **)(*(int *)param_1[0x22] + 0x3c))(local_14);
      FUN_0054ac09(param_1[0x22]);
    }
  }
  if ((param_1[0x20] == 2) || (param_1[0x20] == 4)) {
    pvVar2 = operator_new(0xcc);
    local_8._0_1_ = 8;
    if (pvVar2 == (void *)0x0) {
      local_f0 = 0;
    }
    else {
      local_f0 = FUN_004d2230(10,local_14,0,0);
    }
    local_8._0_1_ = 0;
    if (local_f0 != 0) {
      *(float *)(local_f0 + 0x98) = _DAT_005a4104 * local_24;
      *(float *)(local_f0 + 0x9c) = _DAT_005a4104 * local_24;
      FUN_004310a0(param_1 + 0x1d);
      *(undefined4 *)(local_f0 + 0xb0) = local_28;
      *(undefined4 *)(local_f0 + 0xb8) = local_28;
      FUN_0054ac09(local_f0);
    }
  }
  else if (((param_1[0x20] != 3) && (param_1[0x20] != 7)) && (param_1[0x20] != 8)) {
    pvVar2 = operator_new(0xcc);
    local_8._0_1_ = 9;
    if (pvVar2 == (void *)0x0) {
      local_f4 = 0;
    }
    else {
      local_f4 = FUN_004d2230(10,local_14,0,param_1[0x20] + 2);
    }
    local_8._0_1_ = 0;
    if (local_f4 != 0) {
      *(float *)(local_f4 + 0x98) = _DAT_005a3834 * local_24;
      *(float *)(local_f4 + 0x9c) = _DAT_005a3834 * local_24 + (float)(int)param_1[0x20];
      FUN_004310a0(param_1 + 0x1d);
      *(undefined4 *)(local_f4 + 0xb0) = local_28;
      *(undefined4 *)(local_f4 + 0xb8) = 3000;
      FUN_0054ac09(local_f4);
    }
  }
  if (((param_1[0x20] == 0) && (DAT_005ccf98 != 0)) && (iVar3 = FUN_00429a6d(0x26), iVar3 != 0)) {
    uVar4 = 0;
    uVar1 = 0;
    FUN_00429a6d(0x26);
    FUN_0042ad2b(uVar1,uVar4);
  }
  ExceptionList = local_10;
  return param_1;
}

