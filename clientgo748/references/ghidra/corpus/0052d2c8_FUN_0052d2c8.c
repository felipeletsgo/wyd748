// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052d2c8 | Name: FUN_0052d2c8


undefined4 __thiscall FUN_0052d2c8(int param_1,int param_2)

{
  int iVar1;
  void *pvVar2;
  undefined4 uVar3;
  int local_184;
  int local_180;
  int local_17c;
  int local_178;
  _SYSTEMTIME local_14c;
  undefined1 local_13c [128];
  int local_bc;
  int local_b8;
  int local_b4;
  int local_b0;
  int local_ac;
  int local_a8;
  int local_a4;
  int local_a0;
  int local_9c;
  undefined1 local_98 [128];
  undefined4 local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a2433;
  local_10 = ExceptionList;
  local_9c = param_2;
  local_a8 = DAT_0067cf38;
  local_b0 = DAT_013b71e8 + 0x6ec;
  local_a0 = *(int *)(DAT_0067cf38 + 0x27ae8);
  ExceptionList = &local_10;
  *(undefined1 *)(param_2 + 0x1b) = 0;
  *(undefined1 *)(param_2 + 0x7b) = 0;
  *(undefined1 *)(param_2 + 0x7a) = 0;
  local_ac = 0;
  local_18 = 0xffffff00;
  local_14 = 1;
  if ((*(char *)(param_2 + 0x1c) == '-') && (*(short *)(param_2 + 0x7c) == 3)) {
    iVar1 = FUN_00494dcf();
    if (((iVar1 == 30000) && (DAT_005b892c == 2)) &&
       (*(int *)(*(int *)(local_a8 + 0x27b00) + 0x1e8) == 0)) {
      local_14 = 0;
    }
    local_18 = 0xffaaffff;
    local_ac = 1;
    if (*(char *)(local_9c + 0x1d) == '-') {
      local_18 = 0xff00ffff;
      local_ac = 2;
    }
    FUN_0058f078(local_98,s___s_>__s_005bb85c,local_9c + 0xc,local_9c + 0x1c + local_ac);
  }
  else if (*(short *)(param_2 + 0x7c) == 7) {
    local_18 = 0xffbbbbbb;
    local_ac = 0;
    FUN_0058f078(local_98,s___s_>__s_005bb868,param_2 + 0xc,param_2 + 0x1c);
  }
  else if (*(char *)(param_2 + 0x1c) == '=') {
    iVar1 = FUN_00494dcf();
    if (((iVar1 == 30000) && (DAT_005b892c == 2)) &&
       (*(int *)(*(int *)(local_a8 + 0x27af8) + 0x1e8) == 0)) {
      local_14 = 0;
    }
    local_b4 = *(int *)(local_a8 + 0x2872c);
    if (1 < *(int *)(local_b4 + 0x210)) {
      local_18 = 0xffff99ff;
      local_ac = 1;
    }
    FUN_0058f078(local_98,s___s_>__s_005bb874,local_9c + 0xc,local_9c + 0x1c + local_ac);
  }
  else {
    if (*(char *)(param_2 + 0x1c) == '!') {
      GetLocalTime(&local_14c);
      pvVar2 = operator_new(0xe50);
      local_8 = 0;
      if (pvVar2 == (void *)0x0) {
        local_178 = 0;
      }
      else {
        local_178 = FUN_00407203(&DAT_005bb880,0xffffffff,0,0,0x43960000,0x41800000,0,0x77777777,1,0
                                );
      }
      local_8 = 0xffffffff;
      local_bc = local_178;
      if (local_178 != 0) {
        FUN_00408d33(local_178);
      }
      FUN_0058f078(local_98,&DAT_00a3b278,local_9c + 0xc);
      FUN_0058f078(local_13c,s__s___02d__02d__02d__005bb884,local_98,local_14c.wHour,
                   local_14c.wMinute,local_14c.wSecond);
      pvVar2 = operator_new(0xe50);
      local_8 = 1;
      if (pvVar2 == (void *)0x0) {
        local_17c = 0;
      }
      else {
        local_17c = FUN_00407203(local_13c,0xffffffff,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
      }
      local_8 = 0xffffffff;
      if (local_17c != 0) {
        FUN_00408d33(local_17c);
      }
      FUN_0058f078(local_98,&DAT_005bb898,local_9c + 0x1d + local_ac);
      pvVar2 = operator_new(0xe50);
      local_8 = 2;
      if (pvVar2 == (void *)0x0) {
        local_180 = 0;
      }
      else {
        local_180 = FUN_00407203(local_98,0xffffffcc,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
      }
      local_8 = 0xffffffff;
      local_b8 = local_180;
      if (local_180 != 0) {
        FUN_00408d33(local_180);
      }
      if (*(int *)(local_a8 + 0x27cf0) == 0) {
        ExceptionList = local_10;
        return 1;
      }
      (**(code **)(**(int **)(local_a8 + 0x27cf0) + 0x60))(1);
      ExceptionList = local_10;
      return 1;
    }
    iVar1 = FUN_00494dcf();
    if (((iVar1 == 30000) && (DAT_005b892c == 2)) &&
       (*(int *)(*(int *)(local_a8 + 0x27afc) + 0x1e8) == 0)) {
      local_14 = 0;
    }
    FUN_0058f078(local_98,s___s_____s>__s_005bb89c,local_9c + 0xc,param_1 + 0x168,
                 local_9c + 0x1c + local_ac);
  }
  if (local_14 != 0) {
    pvVar2 = operator_new(0xe50);
    local_8 = 3;
    if (pvVar2 == (void *)0x0) {
      local_184 = 0;
    }
    else {
      local_184 = FUN_00407203(local_98,local_18,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
    }
    local_8 = 0xffffffff;
    local_a4 = local_184;
    if ((local_a0 != 0) && (local_184 != 0)) {
      FUN_00408d33(local_184);
    }
    uVar3 = (**(code **)(*DAT_0092e654 + 8))();
    *(undefined4 *)(local_a8 + 0x27464) = uVar3;
  }
  ExceptionList = local_10;
  return 1;
}

