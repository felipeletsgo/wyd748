// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c7755 | Name: FUN_004c7755


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004c7755(int param_1)

{
  float fVar1;
  int *piVar2;
  undefined4 uVar3;
  float local_a0;
  float local_9c;
  float local_98;
  float local_94;
  float local_90;
  float local_8c;
  float local_88;
  float local_84;
  float local_80;
  float local_78;
  float local_74;
  float local_70;
  float local_6c;
  float local_68;
  float local_64;
  float local_60;
  float local_5c;
  float local_58;
  int local_54;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  if ((*(int *)(param_1 + 0x58) == 1) && (*(int *)(param_1 + 0x5c) != 10000)) {
    piVar2 = (int *)FUN_004b14fd(*(undefined4 *)(param_1 + 0x60),*(int *)(param_1 + 0x5c) == 0x2711,
                                 180000);
    _memset(&local_48,0,0x44);
    if ((*(int *)(param_1 + 0x5c) == 0x2711) ||
       ((*(int *)(param_1 + 0x5c) == 0x2712 || (*(int *)(param_1 + 0x5c) == 0x2713)))) {
      local_48 = 0x3e99999a;
      local_44 = 0x3e99999a;
      local_40 = 0x3f800000;
      local_18 = 0x3e99999a;
      local_14 = 0x3e99999a;
      local_10 = 0x3f800000;
      local_c = local_3c;
      local_28 = 0x3e99999a;
      local_24 = 0x3e99999a;
      local_20 = 0x3f800000;
      local_8 = 0;
    }
    else if (*(int *)(param_1 + 0x5c) == 0x2713) {
      local_48 = 0x3f800000;
      local_44 = 0x3f800000;
      local_40 = 0x3f800000;
      local_18 = 0x3f800000;
      local_14 = 0x3f800000;
      local_10 = 0x3f800000;
      local_c = local_3c;
      local_28 = 0x3f800000;
      local_24 = 0x3f800000;
      local_20 = 0x3f800000;
      local_8 = 0x3f800000;
    }
    else {
      local_18 = 0x3e99999a;
      local_14 = 0x3e99999a;
      local_10 = 0x3e99999a;
      local_48 = 0x3e99999a;
      local_44 = 0x3e99999a;
      local_40 = 0x3e99999a;
      local_28 = 0x3e99999a;
      local_24 = 0x3e99999a;
      local_20 = 0x3e99999a;
      local_8 = 0;
    }
    local_3c = local_c;
    local_1c = local_c;
    (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
    if (*(int *)(param_1 + 0x70) == 0) {
      (**(code **)(*DAT_005ccf9c + 0x38))(1,0);
      (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
    }
    else {
      local_54 = 0x99;
      if (*(int *)(param_1 + 0x70) == 6) {
        local_54 = 0xa6;
      }
      if (*(int *)(param_1 + 0x70) == 7) {
        local_54 = 0xf6;
      }
      if (*(int *)(param_1 + 0x70) == 8) {
        local_54 = 0x104;
      }
      if (((DAT_005ccf9c[0xa8e4] == 0) && (DAT_005ccf9c[0xa8e8] == 0)) &&
         (DAT_005ccf9c[0xa8e6] == 0)) {
        uVar3 = FUN_004b925c(local_54 + 10,360000);
        (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar3);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
        (**(code **)(*DAT_005ccf9c + 0x34))(1,1,0xb);
      }
      else {
        uVar3 = FUN_004b925c(local_54 + 10,360000);
        (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar3);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
        (**(code **)(*DAT_005ccf9c + 0x34))(1,1,7);
      }
    }
    if (piVar2 != (int *)0x0) {
      if (*(int *)(param_1 + 0x5c) == 0x97) {
        (**(code **)(*piVar2 + 8))
                  (*(undefined4 *)(param_1 + 0x40),*(undefined4 *)(param_1 + 0x44),
                   *(undefined4 *)(param_1 + 0x48),*(undefined4 *)(param_1 + 0x50),
                   *(float *)(param_1 + 0x54) - _DAT_005a4380,0,0,0);
      }
      else if ((*(int *)(param_1 + 0x5c) == 0x68) || (*(int *)(param_1 + 0x5c) == 0x98)) {
        (**(code **)(*piVar2 + 8))
                  (*(undefined4 *)(param_1 + 0x40),*(undefined4 *)(param_1 + 0x44),
                   *(undefined4 *)(param_1 + 0x48),*(float *)(param_1 + 0x50) - _DAT_005a4380,
                   0x3fc90fdb,0,0,0);
      }
      else if (*(int *)(param_1 + 0x5c) == 0x2711) {
        (**(code **)(*piVar2 + 8))
                  (*(undefined4 *)(param_1 + 0x40),*(undefined4 *)(param_1 + 0x44),
                   *(undefined4 *)(param_1 + 0x48),*(float *)(param_1 + 0x50) - _DAT_005a4380,
                   0xbf32b8c3,0x3f490fdb,0,0);
      }
      else if (*(int *)(param_1 + 0x5c) == 0x2712) {
        piVar2[0xcb] = 0x3f0ccccd;
        piVar2[0xcc] = 0x3f0ccccd;
        CSimpleArray<>();
        CSimpleArray<>();
        CSimpleArray<>();
        uVar3 = FUN_004310a0(param_1 + 0x40);
        uVar3 = FUN_004310a0(uVar3);
        FUN_004310a0(uVar3);
        fVar1 = *(float *)(param_1 + 0x7c) / _DAT_005a40f0 + _DAT_005a40ec;
        (**(code **)(*piVar2 + 8))
                  (local_60 + *(float *)(param_1 + 0x7c),local_5c + fVar1,
                   local_58 + *(float *)(param_1 + 0x80),*(float *)(param_1 + 0x7c) * _DAT_005a3834,
                   0,0,0,0);
        (**(code **)(*piVar2 + 8))
                  (local_6c + *(float *)(param_1 + 0x84),local_68 + fVar1,
                   local_64 + *(float *)(param_1 + 0x88),*(float *)(param_1 + 0x84) * _DAT_005a3834,
                   0,0,0,0);
        (**(code **)(*piVar2 + 8))
                  (local_78 + *(float *)(param_1 + 0x8c),local_74 + fVar1,
                   local_70 + *(float *)(param_1 + 0x90),*(float *)(param_1 + 0x8c) * _DAT_005a3834,
                   0,0,0,0);
      }
      else if (*(int *)(param_1 + 0x5c) == 0x2713) {
        uVar3 = FUN_004b925c(6,360000);
        (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar3);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,1,0xb);
        (**(code **)(*DAT_005ccf9c + 0x34))(1,1,9);
        piVar2[0xcb] = 0x3fc00000;
        piVar2[0xcc] = 0x3fc00000;
        CSimpleArray<>();
        CSimpleArray<>();
        CSimpleArray<>();
        uVar3 = FUN_004310a0(param_1 + 0x40);
        uVar3 = FUN_004310a0(uVar3);
        FUN_004310a0(uVar3);
        fVar1 = *(float *)(param_1 + 0x7c) / _DAT_005a40f0 + _DAT_005a4214;
        (**(code **)(*piVar2 + 8))
                  (local_88 + *(float *)(param_1 + 0x7c),local_84 + fVar1,
                   local_80 + *(float *)(param_1 + 0x80),
                   (*(float *)(param_1 + 0x7c) + *(float *)(param_1 + 0x7c)) - _DAT_005a46d8,
                   *(float *)(param_1 + 0x7c) + *(float *)(param_1 + 0x7c),
                   *(float *)(param_1 + 0x7c) / _DAT_005a365c,0,0);
        (**(code **)(*piVar2 + 8))
                  (local_94 + *(float *)(param_1 + 0x84),local_90 + fVar1,
                   local_8c + *(float *)(param_1 + 0x88),
                   (*(float *)(param_1 + 0x84) + *(float *)(param_1 + 0x84)) - _DAT_005a3534,
                   *(float *)(param_1 + 0x84) + *(float *)(param_1 + 0x84),
                   *(float *)(param_1 + 0x84) / _DAT_005a365c,0,0);
        (**(code **)(*piVar2 + 8))
                  (local_a0 + *(float *)(param_1 + 0x8c),local_9c + fVar1,
                   local_98 + *(float *)(param_1 + 0x90),
                   *(float *)(param_1 + 0x84) + *(float *)(param_1 + 0x84) + _DAT_005a473c,
                   *(float *)(param_1 + 0x8c) + *(float *)(param_1 + 0x8c),
                   *(float *)(param_1 + 0x8c) / _DAT_005a365c,0,0);
      }
      else {
        (**(code **)(*piVar2 + 8))
                  (*(undefined4 *)(param_1 + 0x40),*(undefined4 *)(param_1 + 0x44),
                   *(undefined4 *)(param_1 + 0x48),*(undefined4 *)(param_1 + 0x50),0x3fc90fdb,0,0,0)
        ;
      }
    }
    if (*(int *)(param_1 + 0x70) != 0) {
      (**(code **)(*DAT_005ccf9c + 0x38))(1,0);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
      (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
    }
    (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
    (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
    if (*(int *)(param_1 + 0x6c) != 0) {
      (**(code **)(**(int **)(param_1 + 0x6c) + 0x24))();
    }
  }
  return 1;
}

