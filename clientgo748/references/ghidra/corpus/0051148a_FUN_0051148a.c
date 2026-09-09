// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0051148a | Name: FUN_0051148a


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0051148a(int param_1)

{
  void *pvVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  int *local_ec;
  int *local_e8;
  int *local_e4;
  int *local_e0;
  int local_dc;
  int *local_d8;
  int *local_d4;
  int local_44;
  undefined1 local_40 [12];
  int *local_34;
  int *local_30;
  int local_2c;
  int local_28;
  int *local_24;
  undefined4 local_20;
  int local_1c;
  float local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a1f4d;
  local_10 = ExceptionList;
  if (*(char *)(param_1 + 0x7e2) == '\x01') {
    ExceptionList = &local_10;
    pvVar1 = operator_new(0x22c);
    local_8 = 0;
    if (pvVar1 == (void *)0x0) {
      local_d4 = (int *)0x0;
    }
    else {
      uVar7 = 0;
      uVar6 = 0;
      puVar2 = (undefined4 *)FUN_00430f50(0,0,0);
      uVar3 = *puVar2;
      uVar4 = puVar2[1];
      uVar5 = puVar2[2];
      puVar2 = (undefined4 *)FUN_00430f50(0,0,0);
      local_d4 = (int *)FUN_004d5480(*(undefined4 *)(param_1 + 0x5c),*puVar2,puVar2[1],puVar2[2],
                                     uVar3,uVar4,uVar5,uVar6,uVar7);
    }
    local_8 = 0xffffffff;
    local_14 = local_d4;
    if (local_d4 != (int *)0x0) {
      local_20 = 0;
      if ((*(int *)(param_1 + 0x24c) == 4) || (*(int *)(param_1 + 0x24c) == 8)) {
        local_20 = 1;
      }
      FUN_0058f220(local_d4 + 0x58,param_1 + 0x1d2,0x20);
      local_1c = 0xe;
      local_14[0x69] = 0x3e99999a;
      local_14[0x6a] = 0x3e99999a;
      local_14[0x6b] = 0x3e99999a;
      (**(code **)(*local_14 + 0x78))(local_20);
      local_14[0x75] = 1;
      local_14[0x65] = 0x578;
      local_18 = *(float *)(param_1 + 0x30);
      if (('\0' < *(char *)(param_1 + 0x79c)) && (*(int *)(param_1 + 0x19c) != 0)) {
        local_18 = local_18 + _DAT_005a34a0;
      }
      (**(code **)(*local_14 + 0x3c))
                (*(undefined4 *)(param_1 + 0x28),local_18,*(undefined4 *)(param_1 + 0x2c));
      *(undefined4 *)(local_14[0x18] + 0x24) = 0x40000000;
      *(undefined4 *)(local_14[0x18] + 0x28) = 0x40000000;
      *(undefined4 *)(local_14[0x18] + 0x2c) = 0x40000000;
      local_14[0x7d] = *(int *)(param_1 + 0x34);
      *(undefined4 *)(local_14[0x18] + 0x2e4) = *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2e4);
      FUN_004bd906(*(undefined4 *)(&DAT_00672490 + local_1c * 4 + *(int *)(param_1 + 0x5c) * 0x2a0))
      ;
      local_14[0x68] = 0x3f800000;
      local_14[0x66] = 1;
      FUN_0054ac09(local_14);
      pvVar1 = operator_new(0x180);
      local_8 = 1;
      if (pvVar1 == (void *)0x0) {
        local_d8 = (int *)0x0;
      }
      else {
        local_d8 = (int *)FUN_004c8f70(0,0x514,0x40e00000,0x40e00000,0x40e00000,0x38d1b717,1,0x50);
      }
      local_8 = 0xffffffff;
      local_24 = local_d8;
      if (local_d8 != (int *)0x0) {
        uVar3 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                             *(float *)(param_1 + 0x30) + _DAT_005a3660,
                             *(undefined4 *)(param_1 + 0x2c));
        uVar3 = FUN_004310a0(uVar3);
        FUN_004310a0(uVar3);
        local_24[0x1b] = 1;
        local_24[0x40] = 1;
        local_24[0x52] = 2;
        local_24[0x50] = 0x3d4ccccd;
        local_24[0x4f] = 0x3d4ccccd;
        (**(code **)(*local_24 + 0x3c))(0xffffffff);
        FUN_0054ac09(local_24);
      }
    }
  }
  else if (*(char *)(param_1 + 0x7e2) == '\x02') {
    ExceptionList = &local_10;
    for (local_28 = 0; local_28 < 3; local_28 = local_28 + 1) {
      FUN_00430f50(*(undefined4 *)(param_1 + 0x4c0),*(float *)(param_1 + 0x4c4) + _DAT_005a4bd8,
                   *(undefined4 *)(param_1 + 0x4c8));
      local_2c = 0;
      pvVar1 = operator_new(0x180);
      local_8 = 2;
      if (pvVar1 == (void *)0x0) {
        local_dc = 0;
      }
      else {
        local_dc = FUN_004c8f70(0x21,1000,(float)local_28 * _DAT_005a430c + _DAT_005a365c,
                                (float)local_28 * _DAT_005a430c + _DAT_005a365c,
                                (float)local_28 * _DAT_005a430c + _DAT_005a365c,0,9,0x6e);
      }
      local_8 = 0xffffffff;
      local_2c = local_dc;
      if (local_dc == 0) {
        ExceptionList = local_10;
        return;
      }
      *(undefined4 *)(local_dc + 0x6c) = 1;
      *(undefined4 *)(local_dc + 0x104) = 0;
      FUN_004310a0(local_40);
      *(float *)(local_2c + 0x78) = *(float *)(local_2c + 0x78) - _DAT_005a34a0;
      FUN_0054ac09(local_2c);
      pvVar1 = operator_new(0x180);
      local_8 = 3;
      if (pvVar1 == (void *)0x0) {
        local_e0 = (int *)0x0;
      }
      else {
        local_e0 = (int *)FUN_004c8f70(0,0x5dc,(float)local_28 * _DAT_005a3660 + _DAT_005a3834,
                                       (float)local_28 + (float)local_28 + _DAT_005a3834,
                                       (float)local_28 * _DAT_005a3660 + _DAT_005a3834,0x38d1b717,1,
                                       0x50);
      }
      local_8 = 0xffffffff;
      local_34 = local_e0;
      if (local_e0 == (int *)0x0) {
        ExceptionList = local_10;
        return;
      }
      uVar3 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                           *(float *)(param_1 + 0x30) + _DAT_005a365c,
                           *(undefined4 *)(param_1 + 0x2c));
      uVar3 = FUN_004310a0(uVar3);
      FUN_004310a0(uVar3);
      local_34[0x1b] = 1;
      local_34[0x40] = 1;
      local_34[0x52] = 1;
      local_34[0x50] = 0x3da3d70a;
      local_34[0x4f] = 0x3da3d70a;
      (**(code **)(*local_34 + 0x3c))(0xffff5555);
      FUN_0054ac09(local_34);
      pvVar1 = operator_new(0x124);
      local_8 = 4;
      if (pvVar1 == (void *)0x0) {
        local_e4 = (int *)0x0;
      }
      else {
        local_e4 = (int *)FUN_004cb900(8,0x5dc,0x3a03126f,0x3a03126f,0x3a03126f,0x3ba3d70a,0);
      }
      local_8 = 0xffffffff;
      local_30 = local_e4;
      if (local_e4 != (int *)0x0) {
        local_e4[0x1b] = 1;
        FUN_004310a0(local_40);
        local_30[0x1e] =
             (int)((float)local_30[0x1e] - (_DAT_005a365c - (float)local_28 * _DAT_005a4214));
        (**(code **)(*local_30 + 0x3c))(0xffff5555);
        FUN_0054ac09(local_30);
      }
    }
  }
  else if (*(char *)(param_1 + 0x7e2) == '\x03') {
    ExceptionList = &local_10;
    for (local_44 = 0; local_44 < 8; local_44 = local_44 + 1) {
      pvVar1 = operator_new(0x180);
      local_8 = 5;
      if (pvVar1 == (void *)0x0) {
        local_e8 = (int *)0x0;
      }
      else {
        local_e8 = (int *)FUN_004c8f70(0x38,local_44 * 100 + 1000,
                                       (float)local_44 * _DAT_005a34a0 + _DAT_005a430c,
                                       (float)local_44 * _DAT_005a4154 + _DAT_005a430c,
                                       (float)local_44 * _DAT_005a34a0 + _DAT_005a430c,0,1,0x50);
      }
      local_8 = 0xffffffff;
      if (local_e8 == (int *)0x0) {
        ExceptionList = local_10;
        return;
      }
      uVar3 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                           (*(float *)(param_1 + 0x30) + _DAT_005a4104) -
                           (float)local_44 * _DAT_005a40f4,*(undefined4 *)(param_1 + 0x2c));
      uVar3 = FUN_004310a0(uVar3);
      FUN_004310a0(uVar3);
      local_e8[0x1b] = 1;
      local_e8[0x40] = 1;
      local_e8[0x52] = 9;
      local_e8[0x50] = (int)((float)local_44 + (float)local_44 + _DAT_005a3660);
      local_e8[0x4f] = (int)((float)local_44 + (float)local_44 + _DAT_005a3660);
      (**(code **)(*local_e8 + 0x3c))(0xffff3300);
      FUN_0054ac09(local_e8);
      pvVar1 = operator_new(0x180);
      local_8 = 6;
      if (pvVar1 == (void *)0x0) {
        local_ec = (int *)0x0;
      }
      else {
        local_ec = (int *)FUN_004c8f70(0,1000,0x40a00000,0x40a00000,0x40a00000,0x38d1b717,1,0x50);
      }
      local_8 = 0xffffffff;
      if (local_ec == (int *)0x0) {
        ExceptionList = local_10;
        return;
      }
      uVar3 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                           *(float *)(param_1 + 0x30) + _DAT_005a365c,
                           *(undefined4 *)(param_1 + 0x2c));
      uVar3 = FUN_004310a0(uVar3);
      FUN_004310a0(uVar3);
      local_ec[0x1b] = 1;
      local_ec[0x40] = 0;
      local_ec[0x52] = 1;
      local_ec[0x50] = 0x40a00000;
      local_ec[0x4f] = 0x40a00000;
      (**(code **)(*local_ec + 0x3c))(0xffff0000);
      FUN_0054ac09(local_ec);
    }
  }
  ExceptionList = local_10;
  return;
}

