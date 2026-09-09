// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c5cc0 | Name: FUN_004c5cc0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_004c5cc0(undefined4 *param_1,undefined4 param_2,float param_3,undefined4 param_4)

{
  undefined4 uVar1;
  void *pvVar2;
  int iVar3;
  undefined4 *puVar4;
  float10 fVar5;
  int in_stack_0000001c;
  undefined4 in_stack_00000020;
  undefined1 in_stack_00000024;
  int in_stack_00000028;
  undefined4 in_stack_0000002c;
  undefined *puVar6;
  undefined4 uVar7;
  undefined *puVar8;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined *local_34;
  undefined *local_30 [4];
  undefined1 local_20 [12];
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a071a;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0054a9e0(0);
  local_8 = 0;
  CSimpleArray<>();
  CSimpleArray<>();
  CSimpleArray<>();
  FUN_00430f10();
  FUN_00430f10();
  FUN_00430f10();
  *param_1 = &PTR_FUN_005a46fc;
  param_1[0x14] = 0;
  param_1[0x15] = 0;
  uVar1 = FUN_004310a0(&param_2);
  FUN_004310a0(uVar1);
  FUN_004310a0(&stack0x00000010);
  param_1[0x13] = in_stack_0000001c;
  param_1[0x17] = in_stack_00000020;
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[9] = uVar1;
  param_1[0x1b] = 0;
  param_1[0x1f] = 0;
  param_1[0x20] = 0;
  param_1[0x21] = 0;
  param_1[0x22] = 0;
  param_1[0x23] = 0;
  param_1[0x24] = 0;
  FUN_00493db0(local_20,&param_2);
  fVar5 = (float10)FUN_004b3aa0();
  local_14 = (float)fVar5;
  if (param_1[0x17] == 0x97) {
    param_1[0x18] = 800;
    param_1[0x15] = 0;
    if (0 < in_stack_0000001c) {
      local_30[1] = &DAT_00885522;
      local_30[2] = (undefined *)0xaa00;
      local_30[3] = &DAT_00aa00ff;
      pvVar2 = operator_new(0xcc);
      local_8._0_1_ = 1;
      if (pvVar2 == (void *)0x0) {
        local_60 = 0;
      }
      else {
        local_60 = FUN_004d2230(0x1c,local_30[in_stack_0000001c % 3],param_1[0x15],1);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      param_1[0x1b] = local_60;
      if (param_1[0x1b] != 0) {
        *(undefined4 *)(param_1[0x1b] + 0x98) = 0x3fa66666;
        *(undefined4 *)(param_1[0x1b] + 0x9c) = 0x3fa66666;
      }
    }
    if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x85), iVar3 != 0)) {
      uVar7 = 0;
      uVar1 = 0;
      FUN_00429a6d(0x85);
      FUN_0042ad2b(uVar1,uVar7);
    }
    iVar3 = __ftol();
    param_1[0x19] = iVar3 * 0x32;
  }
  else if (param_1[0x17] == 0x98) {
    iVar3 = __ftol();
    param_1[0x19] = iVar3 * 0x32;
    if (in_stack_0000001c == 0) {
      param_1[0x18] = 0x367;
    }
    if (in_stack_0000001c == 1) {
      param_1[0x18] = 0x368;
    }
    if (in_stack_0000001c == 2) {
      param_1[0x18] = 0x35f;
      pvVar2 = operator_new(0xcc);
      local_8._0_1_ = 2;
      if (pvVar2 == (void *)0x0) {
        local_64 = 0;
      }
      else {
        local_64 = FUN_004d2230(0x1c,0xff001020,param_1[0x15],1);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      param_1[0x1b] = local_64;
      if (param_1[0x1b] != 0) {
        *(undefined4 *)(param_1[0x1b] + 0x98) = 0x40000000;
        *(undefined4 *)(param_1[0x1b] + 0x9c) = 0x40000000;
      }
      iVar3 = __ftol();
      param_1[0x19] = iVar3 * 0x46;
    }
    if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x86), iVar3 != 0)) {
      uVar7 = 0;
      uVar1 = 0;
      FUN_00429a6d(0x86);
      FUN_0042ad2b(uVar1,uVar7);
    }
  }
  else if (param_1[0x17] == 0x99) {
    if (in_stack_0000001c == 0) {
      param_1[0x18] = 0x369;
    }
    if (in_stack_0000001c == 1) {
      param_1[0x18] = 0x36a;
    }
    if (in_stack_0000001c == 2) {
      param_1[0x18] = 0x36b;
    }
    if (in_stack_0000001c == 3) {
      param_1[0x18] = 0x36c;
    }
    if (in_stack_0000001c == 4) {
      param_1[0x18] = 0x36d;
    }
    if (in_stack_0000001c == 5) {
      param_1[0x18] = 0x37c;
    }
    if (in_stack_0000001c == 6) {
      param_1[0x18] = 0x38b;
    }
    if (in_stack_0000001c == 7) {
      param_1[0x18] = 0x38c;
    }
    if (in_stack_0000001c == 8) {
      param_1[0x18] = 0x38d;
    }
    if (in_stack_0000001c == 9) {
      param_1[0x18] = 0x25;
    }
    if (in_stack_0000001c == 10) {
      param_1[0x18] = 0x2ff;
    }
    if (in_stack_0000001c == 0xb) {
      param_1[0x18] = 0xafe;
    }
    param_1[0x15] = 0;
    iVar3 = __ftol();
    param_1[0x19] = iVar3 * 100;
  }
  else if (param_1[0x17] == 0x68) {
    param_1[0x18] = 0x36f;
    iVar3 = __ftol();
    param_1[0x19] = iVar3 * 0x96;
  }
  else if (param_1[0x17] == 0x69) {
    if (in_stack_0000001c == 0) {
      param_1[0x18] = 0x375;
    }
    if (in_stack_0000001c == 1) {
      param_1[0x18] = 0x397;
    }
    iVar3 = __ftol();
    param_1[0x19] = iVar3 * 100;
  }
  else if (param_1[0x17] == 10000) {
    iVar3 = __ftol();
    param_1[0x19] = iVar3 * 0x32;
  }
  else if (param_1[0x17] == 0x2711) {
    iVar3 = __ftol();
    param_1[0x19] = iVar3 * 100;
    param_1[0x18] = 0xb18;
    in_stack_00000028 = 5;
  }
  else if (param_1[0x17] == 0x2712) {
    param_1[0x19] = 1000;
    param_1[0x18] = 0xb18;
    in_stack_00000028 = 5;
  }
  else if (param_1[0x17] == 0x2713) {
    param_1[0x19] = 800;
    param_1[0x18] = 800;
    in_stack_00000028 = 5;
  }
  if (param_1[0x19] == 0) {
    param_1[0x19] = 1;
  }
  if (5000 < (uint)param_1[0x19]) {
    param_1[0x19] = 5000;
  }
  if ((((in_stack_00000028 == 8) || (in_stack_00000028 == 7)) || (in_stack_00000028 == 6)) ||
     (in_stack_00000028 == 5)) {
    param_1[0x1c] = in_stack_00000028;
  }
  else {
    param_1[0x1c] = 0;
  }
  param_1[0x1a] = 0;
  if (((param_1[0x17] == 0x97) || (param_1[0x17] == 0x98)) ||
     ((param_1[0x17] == 0x99 || ((param_1[0x17] == 10000 || (param_1[0x17] == 0x2711)))))) {
    local_30[0] = (undefined *)0x3da3d70a;
    local_34 = &DAT_00777777;
    if (param_1[0x17] == 0x98) {
      local_30[0] = (undefined *)0x3dcccccd;
    }
    if (param_1[0x17] == 0x99) {
      local_30[0] = (undefined *)0x3ecccccd;
    }
    if (in_stack_00000028 == 8) {
      local_34 = &DAT_00883333;
    }
    else if (in_stack_00000028 == 7) {
      local_34 = &DAT_00884388;
    }
    else if (in_stack_00000028 == 6) {
      local_34 = (undefined *)0x338843;
    }
    else if (in_stack_00000028 == 5) {
      local_34 = (undefined *)0x222288;
    }
    if (param_1[0x17] == 10000) {
      local_30[0] = (undefined *)0x3d4ccccd;
      local_34 = (undefined *)0xffaaaaff;
    }
    else if (param_1[0x17] == 0x2711) {
      local_30[0] = (undefined *)0x3f000000;
      local_34 = (undefined *)0xff7777ff;
    }
    pvVar2 = operator_new(0x184);
    local_8._0_1_ = 3;
    if (pvVar2 == (void *)0x0) {
      local_68 = 0;
    }
    else {
      uVar7 = 1000;
      uVar1 = 0x19a;
      puVar6 = local_30[0];
      puVar8 = local_30[0];
      puVar4 = (undefined4 *)FUN_00430f50(param_2,param_3 - _DAT_005a4214,param_4);
      local_68 = FUN_004ccbe0(param_2,param_3,param_4,*puVar4,puVar4[1],puVar4[2],uVar1,local_34,
                              uVar7,puVar6,puVar8);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x1a] = local_68;
  }
  if (param_1[0x1a] != 0) {
    *(undefined4 *)(param_1[0x1a] + 0x6c) = 1;
    *(undefined4 *)(param_1[0x1a] + 0x17c) = 1;
    *(int *)(param_1[0x1a] + 0x84) = param_1[0x19] + 2000;
    FUN_0054ac09(param_1[0x1a]);
  }
  param_1[0x1d] = in_stack_0000002c;
  *(undefined1 *)(param_1 + 0x1e) = in_stack_00000024;
  ExceptionList = local_10;
  return param_1;
}

