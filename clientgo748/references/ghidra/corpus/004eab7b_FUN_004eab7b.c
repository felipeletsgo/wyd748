// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004eab7b | Name: FUN_004eab7b


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004eab7b(undefined4 *param_1)

{
  void *pvVar1;
  undefined4 *puVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  int *local_110;
  int local_10c;
  int local_108;
  int *local_104;
  int *local_100;
  int *local_fc;
  int *local_f8;
  int *local_f4;
  int *local_f0;
  int *local_ec;
  int local_e8;
  int local_e4;
  int local_e0;
  int local_dc;
  int local_d8;
  int *local_1c;
  int local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_005a13fa;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a50f8;
  local_8 = 0;
  (**(code **)(*DAT_013b71e8 + 100))(param_1[0x2c]);
  (**(code **)(*DAT_013b71e8 + 100))(param_1[0x2d]);
  (**(code **)(*DAT_013b71e8 + 100))(param_1[0x2e]);
  (**(code **)(*DAT_013b71e8 + 100))(param_1[0x2b]);
  if (DAT_0092e660 != 1) {
    local_18 = 0;
    if (param_1[0x27] == 0) {
      pvVar1 = operator_new(0x180);
      local_8._0_1_ = 1;
      if (pvVar1 == (void *)0x0) {
        local_d8 = 0;
      }
      else {
        local_d8 = FUN_004c8f70(0x21,999,0x3fc00000,0x3fc00000,0x3fc00000,0,9,0x6f);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      local_18 = local_d8;
    }
    else if (param_1[0x27] == 1) {
      pvVar1 = operator_new(0x180);
      local_8._0_1_ = 2;
      if (pvVar1 == (void *)0x0) {
        local_dc = 0;
      }
      else {
        local_dc = FUN_004c8f70(0x47,0x378,0x3f000000,0x3f000000,0x3f000000,0,8,0x6f);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      local_18 = local_dc;
    }
    else if (param_1[0x27] == 4) {
      pvVar1 = operator_new(0x180);
      local_8._0_1_ = 3;
      if (pvVar1 == (void *)0x0) {
        local_e0 = 0;
      }
      else {
        local_e0 = FUN_004c8f70(0x21,999,0x3fc00000,0x3fc00000,0x3fc00000,0,9,0x6f);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      local_18 = local_e0;
    }
    else if (param_1[0x27] == 6) {
      pvVar1 = operator_new(0x180);
      local_8._0_1_ = 4;
      if (pvVar1 == (void *)0x0) {
        local_e4 = 0;
      }
      else {
        local_e4 = FUN_004c8f70(0x21,999,0x40a00000,0x40a00000,0x40a00000,0,9,0x5a);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      local_18 = local_e4;
    }
    if (local_18 != 0) {
      *(undefined4 *)(local_18 + 0x6c) = 1;
      *(undefined4 *)(local_18 + 0x104) = 0;
      FUN_004310a0(param_1 + 0x24);
      *(float *)(local_18 + 0x78) = *(float *)(local_18 + 0x78) - _DAT_005a34a0;
      FUN_0054ac09(local_18);
    }
    local_14 = 0x98;
    local_1c = (int *)0x0;
    if (param_1[0x27] == 0) {
      pvVar1 = operator_new(0x124);
      local_8._0_1_ = 5;
      if (pvVar1 == (void *)0x0) {
        local_e8 = 0;
      }
      else {
        local_e8 = FUN_004cb900(8,1000,0x3c23d70a,0x3c23d70a,0x3c23d70a,0x3b449ba6,0);
      }
      local_8._0_1_ = 0;
      if (local_e8 != 0) {
        *(undefined4 *)(local_e8 + 0x6c) = 1;
        FUN_004310a0(param_1 + 0x24);
        *(float *)(local_e8 + 0x78) = *(float *)(local_e8 + 0x78) - _DAT_005a414c;
        FUN_0054ac09(local_e8);
      }
      pvVar1 = operator_new(0xc0);
      local_8._0_1_ = 6;
      if (pvVar1 == (void *)0x0) {
        local_ec = (int *)0x0;
      }
      else {
        local_ec = (int *)FUN_004df250(4,7,0x3f800000);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      local_1c = local_ec;
      if (local_ec != (int *)0x0) {
        (**(code **)(*local_ec + 0x44))(0xffff7711);
      }
    }
    else if (param_1[0x27] == 1) {
      local_14 = 0x9a;
      pvVar1 = operator_new(0xc0);
      local_8._0_1_ = 7;
      if (pvVar1 == (void *)0x0) {
        local_f0 = (int *)0x0;
      }
      else {
        local_f0 = (int *)FUN_004df250(2,7,0x3f800000);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      local_1c = local_f0;
      if (local_f0 != (int *)0x0) {
        (**(code **)(*local_f0 + 0x44))(0xff55eeff);
      }
    }
    else if (param_1[0x27] == 3) {
      pvVar1 = operator_new(0xc0);
      local_8._0_1_ = 8;
      if (pvVar1 == (void *)0x0) {
        local_f4 = (int *)0x0;
      }
      else {
        local_f4 = (int *)FUN_004df250(2,7,0x3f800000);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      local_1c = local_f4;
      if (local_f4 != (int *)0x0) {
        (**(code **)(*local_f4 + 0x44))(0xffff7722);
      }
    }
    else if (param_1[0x27] == 4) {
      pvVar1 = operator_new(0x124);
      local_8._0_1_ = 9;
      if (pvVar1 == (void *)0x0) {
        local_f8 = (int *)0x0;
      }
      else {
        local_f8 = (int *)FUN_004cb900(8,1000,0x3c23d70a,0x3c23d70a,0x3c23d70a,0x3b449ba6,0);
      }
      local_8._0_1_ = 0;
      if (local_f8 != (int *)0x0) {
        local_f8[0x1b] = 1;
        FUN_004310a0(param_1 + 0x24);
        local_f8[0x1e] = (int)((float)local_f8[0x1e] - _DAT_005a414c);
        (**(code **)(*local_f8 + 0x3c))(0xff5555aa);
        FUN_0054ac09(local_f8);
      }
      pvVar1 = operator_new(0xc0);
      local_8._0_1_ = 10;
      if (pvVar1 == (void *)0x0) {
        local_fc = (int *)0x0;
      }
      else {
        local_fc = (int *)FUN_004df250(4,7,0x3f800000);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      local_1c = local_fc;
      if (local_fc != (int *)0x0) {
        (**(code **)(*local_fc + 0x44))(0xff5555aa);
      }
    }
    else if (param_1[0x27] == 6) {
      pvVar1 = operator_new(0x124);
      local_8._0_1_ = 0xb;
      if (pvVar1 == (void *)0x0) {
        local_100 = (int *)0x0;
      }
      else {
        local_100 = (int *)FUN_004cb900(8,1000,0x3c23d70a,0x3c23d70a,0x3c23d70a,0x3bc49ba6,0);
      }
      local_8._0_1_ = 0;
      if (local_100 != (int *)0x0) {
        local_100[0x1b] = 1;
        FUN_004310a0(param_1 + 0x24);
        local_100[0x1e] = (int)((float)local_100[0x1e] - _DAT_005a414c);
        (**(code **)(*local_100 + 0x3c))(0xff5555aa);
        FUN_0054ac09(local_100);
      }
      pvVar1 = operator_new(0xc0);
      local_8._0_1_ = 0xc;
      if (pvVar1 == (void *)0x0) {
        local_104 = (int *)0x0;
      }
      else {
        local_104 = (int *)FUN_004df250(4,7,0x3f800000);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      local_1c = local_104;
      if (local_104 != (int *)0x0) {
        (**(code **)(*local_104 + 0x44))(0xff5555aa);
      }
    }
    if (local_1c != (int *)0x0) {
      local_1c[0x1b] = 1;
      puVar2 = (undefined4 *)FUN_00430f20(param_1[0x24],param_1[0x26]);
      (**(code **)(*local_1c + 0x3c))(*puVar2,puVar2[1]);
      local_1c[0x28] = 0x5dc;
      FUN_0054ac09(local_1c);
    }
    if (param_1[0x27] == 2) {
      pvVar1 = operator_new(0x90);
      local_8._0_1_ = 0xd;
      if (pvVar1 == (void *)0x0) {
        local_108 = 0;
      }
      else {
        uVar4 = 1000;
        puVar2 = (undefined4 *)FUN_00430f50(0,0,0);
        local_108 = FUN_004d4de0(param_1[0x24],param_1[0x25],param_1[0x26],4,0xc,0x3d4ccccd,
                                 0xffffaa00,0,0x38,0x3f800000,1,*puVar2,puVar2[1],puVar2[2],uVar4);
      }
      local_8._0_1_ = 0;
      if (local_108 != 0) {
        FUN_0054ac09(local_108);
      }
      pvVar1 = operator_new(0x9c);
      local_8._0_1_ = 0xe;
      if (pvVar1 == (void *)0x0) {
        local_10c = 0;
      }
      else {
        local_10c = FUN_004e2360(param_1[0x24],param_1[0x25],param_1[0x26],0,0x3f800000,0xd2,
                                 0x44444444);
      }
      local_8._0_1_ = 0;
      if (local_10c != 0) {
        FUN_0054ac09(local_10c);
      }
      pvVar1 = operator_new(0x180);
      local_8._0_1_ = 0xf;
      if (pvVar1 == (void *)0x0) {
        local_110 = (int *)0x0;
      }
      else {
        local_110 = (int *)FUN_004c8f70(0x3b,0x9c4,0x3e4ccccd,0x3e4ccccd,0x3e4ccccd,0x3b449ba6,1,
                                        0x50);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      if (local_110 != (int *)0x0) {
        uVar4 = FUN_00430f50(param_1[0x24],(float)param_1[0x25] + _DAT_005a3660,param_1[0x1f]);
        uVar4 = FUN_004310a0(uVar4);
        FUN_004310a0(uVar4);
        local_110[0x1b] = 0;
        if ((*(int *)(DAT_005ccf9c + 0x2a39c) == 1) || (*(int *)(DAT_005ccf9c + 0x2a3a0) == 1)) {
          local_110[0x1b] = 1;
        }
        (**(code **)(*local_110 + 0x3c))(0xffffffff);
        FUN_0054ac09(local_110);
      }
      local_14 = 0x135;
    }
    if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(local_14), iVar3 != 0)) {
      uVar5 = 0;
      uVar4 = 0;
      FUN_00429a6d(local_14);
      FUN_0042ad2b(uVar4,uVar5);
    }
  }
  local_8 = 0xffffffff;
  FUN_004c8d6c();
  ExceptionList = local_10;
  return;
}

