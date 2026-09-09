// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0051fcab | Name: FUN_0051fcab


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0051fcab(int param_1)

{
  void *pvVar1;
  undefined4 local_68;
  undefined4 local_64;
  undefined1 local_4c [2];
  undefined2 local_4a;
  undefined1 local_2c [8];
  undefined1 local_24;
  float local_1c;
  float local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a212e;
  local_10 = ExceptionList;
  if (*(int *)(param_1 + 0x214) == 0) {
    if (*(short *)(param_1 + 0x1d2) == 0x28) {
      ExceptionList = &local_10;
      if (*(short *)(param_1 + 0x1da) == 0) {
        ExceptionList = &local_10;
        *(undefined2 *)(param_1 + 0x1da) = 0x28;
      }
      if (*(short *)(param_1 + 0x1de) == 0) {
        *(undefined2 *)(param_1 + 0x1de) = 0x28;
      }
      if (*(short *)(param_1 + 0x1e2) == 0) {
        *(undefined2 *)(param_1 + 0x1e2) = 0x28;
      }
      if (*(short *)(param_1 + 0x1e6) == 0) {
        *(undefined2 *)(param_1 + 0x1e6) = 0x28;
      }
      *(undefined2 *)(param_1 + 0x1d6) = 0x28;
    }
    else if ((*(short *)(param_1 + 0x1d2) == 0x50) ||
            (ExceptionList = &local_10, *(short *)(param_1 + 0x1d2) == 0x40)) {
      ExceptionList = &local_10;
      if (*(short *)(param_1 + 0x1da) == 0) {
        ExceptionList = &local_10;
        *(undefined2 *)(param_1 + 0x1da) = 0x28;
      }
      if (*(short *)(param_1 + 0x1de) == 0) {
        *(undefined2 *)(param_1 + 0x1de) = 0x28;
      }
      if (*(short *)(param_1 + 0x1e2) == 0) {
        *(undefined2 *)(param_1 + 0x1e2) = 0x28;
      }
      if (*(short *)(param_1 + 0x1e6) == 0) {
        *(undefined2 *)(param_1 + 0x1e6) = 0x28;
      }
      *(undefined2 *)(param_1 + 0x1d6) = *(undefined2 *)(param_1 + 0x1d2);
    }
    if (*(int *)(param_1 + 0x60) == 0) {
      local_14 = 0;
      if ((((*(int *)(param_1 + 0x24c) == 4) || (*(int *)(param_1 + 0x24c) == 8)) ||
          (*(int *)(param_1 + 0x24c) == 0x24)) ||
         (((*(int *)(param_1 + 0x24c) == 0x27 || (*(int *)(param_1 + 0x24c) == 0x28)) ||
          (*(int *)(param_1 + 0x24c) == 0x3c)))) {
        local_14 = 1;
      }
      pvVar1 = operator_new(0x358);
      local_8 = 0;
      if (pvVar1 == (void *)0x0) {
        local_64 = 0;
      }
      else {
        local_64 = FUN_004bd420(param_1 + 0x1d2,param_1 + 0x1f2,*(undefined4 *)(param_1 + 0x5c),
                                local_14,param_1 + 0x202,1,0);
      }
      *(undefined4 *)(param_1 + 0x60) = local_64;
      if (*(int *)(param_1 + 0x60) == 0) {
        ExceptionList = local_10;
        return;
      }
    }
    local_8 = 0xffffffff;
    if (*(int *)(param_1 + 0x60) != 0) {
      *(int *)(*(int *)(param_1 + 0x60) + 0x2f4) = param_1;
      FUN_004be7b1();
      *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2e4) = 0x28;
      if ((*(int *)(param_1 + 0x5c) != 0x1f) || (_DAT_005a3660 <= *(float *)(param_1 + 0x38))) {
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x24) = *(undefined4 *)(param_1 + 0x38);
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x28) = *(undefined4 *)(param_1 + 0x38);
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2c) = *(undefined4 *)(param_1 + 0x38);
      }
      else {
        local_18 = _DAT_005a365c - *(float *)(param_1 + 0x38);
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x24) = *(undefined4 *)(param_1 + 0x38);
        *(float *)(*(int *)(param_1 + 0x60) + 0x28) = *(float *)(param_1 + 0x38) / local_18;
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2c) = *(undefined4 *)(param_1 + 0x38);
      }
    }
    if ((*(int *)(param_1 + 0x5c) != 0x1f) || (_DAT_005a3660 <= *(float *)(param_1 + 0x38))) {
      *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x24) = *(undefined4 *)(param_1 + 0x38);
      *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x28) = *(undefined4 *)(param_1 + 0x38);
      *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2c) = *(undefined4 *)(param_1 + 0x38);
    }
    else {
      local_1c = _DAT_005a365c - *(float *)(param_1 + 0x38);
      *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x24) = *(undefined4 *)(param_1 + 0x38);
      *(float *)(*(int *)(param_1 + 0x60) + 0x28) = *(float *)(param_1 + 0x38) / local_1c;
      *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2c) = *(undefined4 *)(param_1 + 0x38);
    }
    if (('\0' < *(char *)(param_1 + 0x79b)) &&
       ((((*(int *)(param_1 + 0x5c) == 0 || (*(int *)(param_1 + 0x5c) == 1)) ||
         (*(int *)(param_1 + 0x5c) == 8)) ||
        ((*(int *)(param_1 + 0x5c) == 3 || (*(int *)(param_1 + 0x5c) == 2)))))) {
      if (*(int *)(param_1 + 0x198) == 0) {
        _memset(local_4c,0,0x20);
        local_4a = *(undefined2 *)(param_1 + 0x7aa);
        _memset(local_2c,0,0x10);
        local_2c[0] = *(undefined1 *)(param_1 + 0x7ac);
        local_24 = *(undefined1 *)(param_1 + 0x7ad);
        pvVar1 = operator_new(0x358);
        local_8 = 1;
        if (pvVar1 == (void *)0x0) {
          local_68 = 0;
        }
        else {
          local_68 = FUN_004bd420(local_4c,local_2c,0x55,0,0,1,0);
        }
        local_8 = 0xffffffff;
        *(undefined4 *)(param_1 + 0x198) = local_68;
        if (*(int *)(param_1 + 0x198) == 0) {
          ExceptionList = local_10;
          return;
        }
        *(undefined4 *)(*(int *)(param_1 + 0x198) + 0x2f4) = 0;
        *(undefined4 *)(*(int *)(param_1 + 0x198) + 0x2e4) = 0x28;
        *(undefined4 *)(*(int *)(param_1 + 0x198) + 0x24) = *(undefined4 *)(param_1 + 0x38);
        *(undefined4 *)(*(int *)(param_1 + 0x198) + 0x28) = *(undefined4 *)(param_1 + 0x38);
        *(undefined4 *)(*(int *)(param_1 + 0x198) + 0x2c) = *(undefined4 *)(param_1 + 0x38);
        FUN_004c0746(1,*(undefined4 *)(param_1 + 0x7a0));
      }
      FUN_004be7b1();
    }
    FUN_00527848();
  }
  ExceptionList = local_10;
  return;
}

