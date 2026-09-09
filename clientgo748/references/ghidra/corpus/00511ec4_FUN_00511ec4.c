// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00511ec4 | Name: FUN_00511ec4


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00511ec4(int param_1)

{
  void *pvVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  int *local_a8;
  int *local_a4;
  int local_a0;
  int local_9c;
  undefined4 local_50;
  float local_48;
  int local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  int local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a1f83;
  local_10 = ExceptionList;
  if (*(char *)(param_1 + 0x7e2) == '\x01') {
    ExceptionList = &local_10;
    for (local_14 = 0; local_14 < 10; local_14 = local_14 + 1) {
      FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                   (float)local_14 * _DAT_005a34a0 + (*(float *)(param_1 + 0x30) - _DAT_005a365c),
                   *(undefined4 *)(param_1 + 0x2c));
      pvVar1 = operator_new(0x94);
      local_8 = 0;
      if (pvVar1 == (void *)0x0) {
        local_9c = 0;
      }
      else {
        local_9c = FUN_004e6590(local_24,local_20,local_1c,7,(float)local_14 * _DAT_005a4214);
      }
      local_8 = 0xffffffff;
      if (local_9c == 0) {
        ExceptionList = local_10;
        return;
      }
      FUN_0054ac09(local_9c);
    }
  }
  else if (*(char *)(param_1 + 0x7e2) == '\x02') {
    ExceptionList = &local_10;
    for (local_28 = 0; local_28 < 5; local_28 = local_28 + 1) {
      FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                   (*(float *)(param_1 + 0x30) + _DAT_005a34a0) - (float)local_28 * _DAT_005a4214,
                   *(undefined4 *)(param_1 + 0x2c));
      pvVar1 = operator_new(0x94);
      local_8 = 1;
      if (pvVar1 == (void *)0x0) {
        local_a0 = 0;
      }
      else {
        local_a0 = FUN_004e6590(local_3c,local_38,local_34,8,(float)local_28 * _DAT_005a4154);
      }
      local_8 = 0xffffffff;
      if (local_a0 == 0) {
        ExceptionList = local_10;
        return;
      }
      FUN_0054ac09(local_a0);
      pvVar1 = operator_new(0x180);
      local_8 = 2;
      if (pvVar1 == (void *)0x0) {
        local_a4 = (int *)0x0;
      }
      else {
        local_a4 = (int *)FUN_004c8f70(0x38,0x8fc,(float)local_28 * _DAT_005a55fc + _DAT_005a40ec,
                                       (float)local_28 * _DAT_005a34a0 + _DAT_005a40ec,
                                       (float)local_28 * _DAT_005a55fc + _DAT_005a40ec,0,1,0x50);
      }
      local_8 = 0xffffffff;
      if (local_a4 == (int *)0x0) {
        ExceptionList = local_10;
        return;
      }
      uVar2 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                           (float)local_28 * _DAT_005a4160 +
                           (*(float *)(param_1 + 0x30) - _DAT_005a34a0),
                           *(undefined4 *)(param_1 + 0x2c));
      uVar2 = FUN_004310a0(uVar2);
      FUN_004310a0(uVar2);
      local_a4[0x1b] = 1;
      local_a4[0x40] = 1;
      local_a4[0x52] = 9;
      local_a4[0x50] = (int)((float)local_28 * _DAT_005a3660 + _DAT_005a3660);
      local_a4[0x4f] = (int)((float)local_28 * _DAT_005a3660 + _DAT_005a3660);
      (**(code **)(*local_a4 + 0x3c))(0x33555555);
      FUN_0054ac09(local_a4);
    }
  }
  else if (*(char *)(param_1 + 0x7e2) == '\x03') {
    ExceptionList = &local_10;
    for (local_40 = 0; local_40 < 6; local_40 = local_40 + 1) {
      pvVar1 = operator_new(0x22c);
      local_8 = 3;
      if (pvVar1 == (void *)0x0) {
        local_a8 = (int *)0x0;
      }
      else {
        uVar7 = 0;
        uVar6 = 0;
        puVar3 = (undefined4 *)FUN_00430f50(0,0,0);
        uVar2 = *puVar3;
        uVar4 = puVar3[1];
        uVar5 = puVar3[2];
        puVar3 = (undefined4 *)FUN_00430f50(0,0,0);
        local_a8 = (int *)FUN_004d5480(*(undefined4 *)(param_1 + 0x5c),*puVar3,puVar3[1],puVar3[2],
                                       uVar2,uVar4,uVar5,uVar6,uVar7);
      }
      local_8 = 0xffffffff;
      if (local_a8 == (int *)0x0) {
        ExceptionList = local_10;
        return;
      }
      local_50 = 0;
      if ((*(int *)(param_1 + 0x24c) == 4) || (*(int *)(param_1 + 0x24c) == 8)) {
        local_50 = 1;
      }
      FUN_0058f220(local_a8 + 0x58,param_1 + 0x1d2,0x20);
      local_a8[0x69] = (int)((float)local_40 * _DAT_005a415c + _DAT_005a40ec);
      local_a8[0x6a] = (int)((float)local_40 * _DAT_005a415c + _DAT_005a40ec);
      local_a8[0x6b] = (int)((float)local_40 * _DAT_005a415c + _DAT_005a40ec);
      (**(code **)(*local_a8 + 0x78))(local_50);
      local_a8[0x75] = 1;
      local_a8[0x65] = local_40 * 300 + 100;
      local_48 = *(float *)(param_1 + 0x30);
      if (('\0' < *(char *)(param_1 + 0x79c)) && (*(int *)(param_1 + 0x19c) != 0)) {
        local_48 = local_48 + _DAT_005a34a0;
      }
      (**(code **)(*local_a8 + 0x3c))
                (*(undefined4 *)(param_1 + 0x28),local_48,*(undefined4 *)(param_1 + 0x2c));
      *(undefined4 *)(local_a8[0x18] + 0x24) = 0x3ff33333;
      *(undefined4 *)(local_a8[0x18] + 0x28) = 0x3ff33333;
      *(undefined4 *)(local_a8[0x18] + 0x2c) = 0x3ff33333;
      local_a8[0x67] = 0xc;
      local_a8[0x7d] = (int)((float)local_40 * _DAT_005a430c + *(float *)(param_1 + 0x34));
      *(int *)(local_a8[0x18] + 0x2e4) = *(int *)(*(int *)(param_1 + 0x60) + 0x2e4) + local_40 * -4;
      FUN_004bd906(*(undefined4 *)(&DAT_006724a0 + *(int *)(param_1 + 0x5c) * 0x2a0));
      local_a8[0x68] = 0x3f800000;
      local_a8[0x66] = 1;
      FUN_0054ac09(local_a8);
    }
  }
  ExceptionList = local_10;
  return;
}

