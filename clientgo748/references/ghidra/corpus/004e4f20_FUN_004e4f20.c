// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e4f20 | Name: FUN_004e4f20


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_004e4f20(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6)

{
  undefined4 uVar1;
  void *pvVar2;
  undefined4 *puVar3;
  int iVar4;
  undefined4 uVar5;
  int *local_4c;
  int local_48;
  undefined4 local_18;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a106f;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8._0_1_ = 0;
  local_8._1_3_ = 0;
  *param_1 = &PTR_FUN_005a4f44;
  param_1[0x23] = param_5;
  FUN_004310a0(&param_2);
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x20] = uVar1;
  param_1[0x21] = 2000;
  param_1[0x22] = 0;
  param_1[7] = param_6;
  local_18 = 0xc;
  if (_DAT_005a3670 <= *(float *)(DAT_005ccf9c + 0x2b040)) {
    if (_DAT_005a4314 <= *(float *)(DAT_005ccf9c + 0x2b040)) {
      if (*(float *)(DAT_005ccf9c + 0x2b040) < _DAT_005a40fc) {
        local_18 = 8;
      }
    }
    else {
      local_18 = 5;
    }
  }
  else {
    local_18 = 2;
  }
  if (DAT_0092e660 == 0) {
    pvVar2 = operator_new(0x90);
    local_8._0_1_ = 1;
    if (pvVar2 == (void *)0x0) {
      local_48 = 0;
    }
    else {
      uVar1 = 1000;
      puVar3 = (undefined4 *)FUN_00430f50(0,0,0);
      local_48 = FUN_004d4de0(param_2,param_3,param_4,0,local_18,0x3dcccccd,0xff77aaff,0,0x38,
                              0x3f800000,1,*puVar3,puVar3[1],puVar3[2],uVar1);
    }
    local_8._0_1_ = 0;
    if (local_48 != 0) {
      FUN_0054ac09(local_48);
    }
  }
  pvVar2 = operator_new(0xc0);
  local_8._0_1_ = 2;
  if (pvVar2 == (void *)0x0) {
    local_4c = (int *)0x0;
  }
  else {
    local_4c = (int *)FUN_004df250(4,7,0x3f800000);
  }
  local_8 = (uint)local_8._1_3_ << 8;
  if (local_4c != (int *)0x0) {
    (**(code **)(*local_4c + 0x44))(0x115588);
    local_4c[0x1b] = 1;
    puVar3 = (undefined4 *)FUN_00430f20(param_1[0x1d],param_1[0x1f]);
    (**(code **)(*local_4c + 0x3c))(*puVar3,puVar3[1]);
    local_4c[0x28] = 3000;
    FUN_0054ac09(local_4c);
  }
  if (DAT_005ccf98 != 0) {
    iVar4 = FUN_00429a6d(0x9e);
    if (iVar4 != 0) {
      uVar5 = 0;
      uVar1 = 0;
      FUN_00429a6d(0x9e);
      FUN_0042ad2b(uVar1,uVar5);
    }
  }
  ExceptionList = local_10;
  return param_1;
}

