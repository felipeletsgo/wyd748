// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e52b0 | Name: FUN_004e52b0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall FUN_004e52b0(undefined4 *param_1)

{
  undefined4 uVar1;
  int iVar2;
  void *pvVar3;
  undefined4 in_stack_00000010;
  float fVar4;
  undefined4 uVar5;
  int local_60;
  int local_40;
  int local_3c;
  int aiStack_38 [10];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a1094;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  *param_1 = &PTR_FUN_005a4f80;
  param_1[0x23] = in_stack_00000010;
  FUN_004310a0(&stack0x00000004);
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x20] = uVar1;
  param_1[0x21] = 1000;
  local_40 = 10;
  if (_DAT_005a3670 <= *(float *)(DAT_005ccf9c + 0x2b040)) {
    if (_DAT_005a4314 <= *(float *)(DAT_005ccf9c + 0x2b040)) {
      if (*(float *)(DAT_005ccf9c + 0x2b040) < _DAT_005a40fc) {
        local_40 = 8;
      }
    }
    else {
      local_40 = 5;
    }
  }
  else {
    local_40 = 2;
  }
  for (local_3c = 0; local_3c < local_40; local_3c = local_3c + 1) {
    iVar2 = _rand();
    iVar2 = iVar2 % 5;
    pvVar3 = operator_new(0x180);
    local_8._0_1_ = 1;
    if (pvVar3 == (void *)0x0) {
      local_60 = 0;
    }
    else {
      local_60 = FUN_004c8f70(0,local_3c * 400 + 0x5dc,(float)iVar2 * _DAT_005a34a0 + _DAT_005a415c,
                              (float)iVar2 * _DAT_005a34a0 + _DAT_005a415c,
                              (float)iVar2 * _DAT_005a34a0 + _DAT_005a415c,0x3a83126f,1,0x50);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    aiStack_38[local_3c] = local_60;
    if (aiStack_38[local_3c] != 0) {
      iVar2 = _rand();
      fVar4 = (float)(iVar2 % 10 + -5) * _DAT_005a4158 + (float)param_1[0x1f];
      uVar1 = param_1[0x1e];
      iVar2 = _rand();
      uVar1 = FUN_00430f50((float)(iVar2 % 10 + -5) * _DAT_005a4158 + (float)param_1[0x1d],uVar1,
                           fVar4);
      FUN_004310a0(uVar1);
      *(undefined4 *)(aiStack_38[local_3c] + 0x6c) = 1;
      *(undefined4 *)(aiStack_38[local_3c] + 0x100) = 1;
      (**(code **)(*(int *)aiStack_38[local_3c] + 0x3c))(0xffaaeeff);
      FUN_0054ac09(aiStack_38[local_3c]);
    }
  }
  if ((DAT_005ccf98 != 0) && (iVar2 = FUN_00429a6d(0x99), iVar2 != 0)) {
    uVar5 = 0;
    uVar1 = 0;
    FUN_00429a6d(0x99);
    FUN_0042ad2b(uVar1,uVar5);
  }
  ExceptionList = local_10;
  return param_1;
}

