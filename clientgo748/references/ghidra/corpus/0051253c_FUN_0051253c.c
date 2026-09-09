// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0051253c | Name: FUN_0051253c


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0051253c(int param_1)

{
  float fVar1;
  void *pvVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  int iVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  int local_8c;
  int local_88;
  int local_84;
  int local_24;
  int local_1c;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a1fae;
  local_10 = ExceptionList;
  if (*(char *)(param_1 + 0x7e2) == '\x01') {
    ExceptionList = &local_10;
    for (local_14 = 0; local_14 < 2; local_14 = local_14 + 1) {
      pvVar2 = operator_new(0xcc);
      local_8 = 0;
      if (pvVar2 == (void *)0x0) {
        local_84 = 0;
      }
      else {
        uVar7 = 0;
        uVar6 = 1;
        uVar4 = 2;
        iVar5 = param_1;
        puVar3 = (undefined4 *)
                 FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                              (*(float *)(param_1 + 0x30) - _DAT_005a40f0) -
                              (float)local_14 * _DAT_005a430c,*(undefined4 *)(param_1 + 0x2c));
        local_84 = FUN_004d3d20(*puVar3,puVar3[1],puVar3[2],uVar4,iVar5,uVar6,uVar7);
      }
      local_8 = 0xffffffff;
      if (local_84 == 0) {
        ExceptionList = local_10;
        return;
      }
      if (*(char *)(param_1 + 0x79c) == '\x01') {
        uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x50),
                             *(float *)(param_1 + 0x30) - _DAT_005a40f0,
                             *(undefined4 *)(param_1 + 0x58));
        FUN_004310a0(uVar4);
      }
      else {
        uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                             *(float *)(param_1 + 0x30) - _DAT_005a40f0,
                             *(undefined4 *)(param_1 + 0x2c));
        FUN_004310a0(uVar4);
      }
      *(int *)(local_84 + 0x98) = local_14 * -100 + 1000;
      *(undefined4 *)(local_84 + 0x6c) = 1;
      *(undefined4 *)(local_84 + 0xb8) = 0x40d00000;
      FUN_004d40f3(0xff5555aa);
      FUN_0054ac09(local_84);
    }
  }
  else if (*(char *)(param_1 + 0x7e2) == '\x02') {
    ExceptionList = &local_10;
    for (local_1c = 0; local_1c < 3; local_1c = local_1c + 1) {
      pvVar2 = operator_new(0x9c);
      local_8 = 1;
      if (pvVar2 == (void *)0x0) {
        local_88 = 0;
      }
      else {
        uVar4 = 1;
        iVar5 = param_1;
        puVar3 = (undefined4 *)
                 FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                              (float)local_1c * _DAT_005a4160 + *(float *)(param_1 + 0x30),
                              *(undefined4 *)(param_1 + 0x2c));
        local_88 = FUN_004ed2a0(*puVar3,puVar3[1],puVar3[2],uVar4,iVar5);
      }
      local_8 = 0xffffffff;
      if (local_88 == 0) {
        ExceptionList = local_10;
        return;
      }
      *(int *)(local_88 + 0x84) = local_1c * 300 + 700;
      FUN_0054ac09(local_88);
    }
  }
  else if (*(char *)(param_1 + 0x7e2) == '\x03') {
    ExceptionList = &local_10;
    for (local_24 = 0; local_24 < 7; local_24 = local_24 + 1) {
      CSimpleArray<>();
      uVar4 = *(undefined4 *)(param_1 + 0x28);
      fVar1 = (float)local_24 * _DAT_005a40ec + *(float *)(param_1 + 0x30) + _DAT_005a34a0;
      uVar6 = *(undefined4 *)(param_1 + 0x2c);
      pvVar2 = operator_new(0x94);
      local_8 = 2;
      if (pvVar2 == (void *)0x0) {
        local_8c = 0;
      }
      else {
        local_8c = FUN_004c5cc0(uVar4,fVar1,uVar6,uVar4,fVar1,uVar6,(int)*(short *)(param_1 + 0x4d2)
                                ,0x2713,0,0,0);
      }
      local_8 = 0xffffffff;
      if (local_8c == 0) {
        ExceptionList = local_10;
        return;
      }
      *(int *)(local_8c + 0x24) = *(int *)(local_8c + 0x24) + local_24 * 100;
      FUN_0054ac09(local_8c);
    }
  }
  ExceptionList = local_10;
  return;
}

