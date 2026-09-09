// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d4de0 | Name: FUN_004d4de0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_004d4de0(undefined4 *param_1,float param_2,float param_3,float param_4,int param_5,int param_6,
            float param_7,undefined4 param_8,int param_9,undefined4 param_10,float param_11,
            int param_12)

{
  undefined4 uVar1;
  void *pvVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  int in_stack_0000003c;
  float fVar7;
  float fVar8;
  int *local_bc;
  undefined4 local_88 [7];
  undefined1 local_6c [64];
  undefined1 local_2c [12];
  float local_20;
  int local_1c;
  int *local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0a4a;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  *param_1 = &PTR_FUN_005a4a24;
  param_1[0x21] = in_stack_0000003c;
  param_1[0x22] = 0;
  FUN_004310a0(&param_2);
  param_1[0x23] = param_5;
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x20] = uVar1;
  for (local_14 = 0; (int)local_14 < param_6; local_14 = local_14 + 1) {
    local_1c = local_14 * 300 + 2000;
    if (param_1[0x23] == 4) {
      local_1c = local_14 * 300 + 1000;
    }
    pvVar2 = operator_new(0x180);
    local_8._0_1_ = 1;
    if (pvVar2 == (void *)0x0) {
      local_bc = (int *)0x0;
    }
    else {
      uVar5 = local_14 & 0x80000001;
      if ((int)uVar5 < 0) {
        uVar5 = (uVar5 - 1 | 0xfffffffe) + 1;
      }
      uVar6 = local_14 & 0x80000001;
      if ((int)uVar6 < 0) {
        uVar6 = (uVar6 - 1 | 0xfffffffe) + 1;
      }
      uVar3 = local_14 & 0x80000001;
      if ((int)uVar3 < 0) {
        uVar3 = (uVar3 - 1 | 0xfffffffe) + 1;
      }
      local_bc = (int *)FUN_004c8f70(param_10,local_1c,(float)(int)uVar3 * param_7 + _DAT_005a4214,
                                     (float)(int)uVar6 * param_7 + _DAT_005a4214,
                                     (float)(int)uVar5 * param_7 + _DAT_005a4214,0,1,0x50);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    local_18 = local_bc;
    if (local_bc != (int *)0x0) {
      local_bc[0x4f] = (int)((float)((int)local_14 % 3) * _DAT_005a439c + _DAT_005a4214);
      if (param_12 == 1) {
        local_bc[0x1b] = 1;
      }
      else {
        local_bc[0x1b] = 0;
      }
      if ((int)param_1[0x23] < 2) {
        uVar5 = local_14 & 0x80000001;
        if ((int)uVar5 < 0) {
          uVar5 = (uVar5 - 1 | 0xfffffffe) + 1;
        }
        local_bc[0x52] = param_5 + 1 + uVar5;
      }
      else if (param_1[0x23] == 2) {
        local_bc[0x52] = 1;
        local_bc[0x50] = -0x3fc00000;
      }
      else if (param_1[0x23] == 3) {
        local_bc[0x52] = 1;
        local_bc[0x50] = 0x40400000;
      }
      else if (param_1[0x23] == 4) {
        local_bc[0x52] = 2;
        local_bc[0x50] = 0x40a00000;
      }
      else if (param_1[0x23] == 5) {
        local_bc[0x41] = 4;
        local_bc[0x52] = 0xd;
        iVar4 = _rand();
        local_20 = (float)(iVar4 % 300 + 700) * _DAT_005a41ec;
        FUN_00430f10();
        FUN_00430f10();
        thunk_FUN_005624c9(local_6c,local_20);
        thunk_FUN_00561462(local_2c,&stack0x00000030,local_6c);
        iVar4 = _rand();
        local_20 = ((float)(iVar4 % 0x16d) / _DAT_005a4a20) * _DAT_005a4148;
        local_20 = local_20 + local_20;
        thunk_FUN_00562604(local_6c,&stack0x00000030,local_20);
        thunk_FUN_00561462(local_18 + 0x58,local_2c,local_6c);
        thunk_FUN_005611dd(local_18 + 0x58,local_18 + 0x58);
        iVar4 = _rand();
        local_18[0x5b] = (int)((float)(iVar4 % 1000) * _DAT_005a41ec);
        local_18[0x3e] = in_stack_0000003c;
      }
      iVar4 = _rand();
      fVar8 = (float)(iVar4 % 5 + -3) * _DAT_005a40ec * param_11 + param_4;
      iVar4 = _rand();
      fVar7 = (float)(iVar4 % 5 + -3) * _DAT_005a4214 + param_3;
      iVar4 = _rand();
      uVar1 = FUN_00430f50((float)(iVar4 % 5 + -3) * _DAT_005a40ec * param_11 + param_2,fVar7,fVar8)
      ;
      uVar1 = FUN_004310a0(uVar1);
      FUN_004310a0(uVar1);
      if (param_1[0x23] == 5) {
        uVar1 = FUN_00430f50(param_2,param_3,param_4);
        uVar1 = FUN_004310a0(uVar1);
        FUN_004310a0(uVar1);
      }
      if (param_9 == 0) {
        (**(code **)(*local_18 + 0x3c))(param_8);
      }
      else {
        local_88[0] = 0xffffffff;
        local_88[1] = 0xffffaaaa;
        local_88[2] = 0xffffffaa;
        local_88[3] = 0xffaaffaa;
        local_88[4] = 0xffaaaaff;
        local_88[5] = 0xffaaffff;
        local_88[6] = 0xffffaaff;
        iVar4 = _rand();
        (**(code **)(*local_18 + 0x3c))(local_88[iVar4 % 7]);
      }
      FUN_0054ac09(local_18);
    }
  }
  ExceptionList = local_10;
  return param_1;
}

