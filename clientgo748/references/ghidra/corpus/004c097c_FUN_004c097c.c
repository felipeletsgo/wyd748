// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c097c | Name: FUN_004c097c


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_004c097c(int param_1,undefined4 param_2)

{
  undefined4 uVar1;
  void *pvVar2;
  float local_70;
  float local_6c;
  float local_68;
  float local_64;
  float local_60;
  float local_5c;
  float local_58;
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  float local_44;
  undefined4 local_40;
  undefined1 local_30 [4];
  int local_2c;
  int local_28;
  undefined4 local_24;
  int local_20;
  int local_1c;
  undefined4 local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a06cf;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  uVar1 = FUN_004b15cc(param_2);
  *(undefined4 *)(param_1 + 0xd0) = uVar1;
  if (*(int *)(param_1 + 0xd0) == -1) {
    uVar1 = FUN_004b163b();
    *(undefined4 *)(param_1 + 0xd0) = uVar1;
    if ((-1 < *(int *)(param_1 + 0xd0)) && (*(int *)(param_1 + 0xd0) < 0x400)) {
      pvVar2 = operator_new(0x360);
      local_8 = 0;
      if (pvVar2 == (void *)0x0) {
        local_40 = 0;
      }
      else {
        local_40 = FUN_004baf10();
      }
      local_8 = 0xffffffff;
      *(undefined4 *)(DAT_006703a8 + 0x41808 + *(int *)(param_1 + 0xd0) * 0x58) = local_40;
      if (*(int *)(DAT_006703a8 + 0x41808 + *(int *)(param_1 + 0xd0) * 0x58) != 0) {
        *(undefined4 *)(param_1 + 4) =
             *(undefined4 *)(DAT_006703a8 + 0x41808 + *(int *)(param_1 + 0xd0) * 0x58);
      }
      FUN_0058f078(DAT_006703a8 + 0x417c8 + *(int *)(param_1 + 0xd0) * 0x58,&DAT_005bb314,param_2);
    }
    if (*(int *)(param_1 + 0xd0) < 0) {
      ExceptionList = local_10;
      return 0;
    }
    local_2c = FUN_0058fb07(param_2,0x8000,0);
    if (local_2c == -1) {
      *(undefined4 *)(*(int *)(DAT_006703a8 + 0x41808 + *(int *)(param_1 + 0xd0) * 0x58) + 8) = 1;
      ExceptionList = local_10;
      return 0;
    }
    FUN_0058f8c9(local_2c,local_30,4);
    FUN_0058f8c9(local_2c,param_1 + 8,4);
    FUN_0058f8c9(local_2c,*(int *)(param_1 + 4) + 0x10,4);
    FUN_0058f8c9(local_2c,*(int *)(param_1 + 4) + 0xc,4);
    FUN_0058f8c9(local_2c,param_1 + 0xb8,4);
    FUN_0058f8c9(local_2c,param_1 + 0xc,4);
    FUN_0058f8c9(local_2c,*(int *)(param_1 + 4) + 0x28,4);
    FUN_0058f8c9(local_2c,*(int *)(param_1 + 4) + 0x20,4);
    *(uint *)(param_1 + 0xcc) = *(uint *)(*(int *)(param_1 + 4) + 0x20) / 3;
    *(undefined4 *)(DAT_006703a8 + 0x41814 + *(int *)(param_1 + 0xd0) * 0x58) =
         *(undefined4 *)(param_1 + 8);
    *(undefined4 *)(DAT_006703a8 + 0x41818 + *(int *)(param_1 + 0xd0) * 0x58) =
         *(undefined4 *)(param_1 + 0xb8);
    *(undefined4 *)(DAT_006703a8 + 0x4181c + *(int *)(param_1 + 0xd0) * 0x58) =
         *(undefined4 *)(param_1 + 0xc);
    if (*(int *)(param_1 + 0xc) == 0) {
      *(undefined4 *)(param_1 + 0x10) = 0;
    }
    else {
      pvVar2 = _malloc(*(int *)(param_1 + 0xc) << 6);
      *(void **)(DAT_006703a8 + 0x4180c + *(int *)(param_1 + 0xd0) * 0x58) = pvVar2;
      pvVar2 = _malloc(0xa0);
      *(void **)(DAT_006703a8 + 0x41810 + *(int *)(param_1 + 0xd0) * 0x58) = pvVar2;
      FUN_0058f8c9(local_2c,*(undefined4 *)
                             (DAT_006703a8 + 0x4180c + *(int *)(param_1 + 0xd0) * 0x58),
                   *(int *)(param_1 + 0xc) << 6);
      FUN_0058f8c9(local_2c,*(undefined4 *)
                             (DAT_006703a8 + 0x41810 + *(int *)(param_1 + 0xd0) * 0x58),
                   *(int *)(param_1 + 0xc) << 2);
      *(undefined4 *)(param_1 + 0x10) =
           *(undefined4 *)(DAT_006703a8 + 0x4180c + *(int *)(param_1 + 0xd0) * 0x58);
      *(undefined4 *)(param_1 + 0xb4) =
           *(undefined4 *)(DAT_006703a8 + 0x41810 + *(int *)(param_1 + 0xd0) * 0x58);
    }
    local_24 = (**(code **)(**(int **)(DAT_005ccf9c + 0x2a3e4) + 0x5c))
                         (*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),
                          *(int *)(*(int *)(param_1 + 4) + 0x28) *
                          *(int *)(*(int *)(param_1 + 4) + 0xc),0,
                          *(undefined4 *)(*(int *)(param_1 + 4) + 0x10),1,
                          *(int *)(param_1 + 4) + 0x318);
    (**(code **)(**(int **)(*(int *)(param_1 + 4) + 0x318) + 0x2c))
              (*(undefined4 *)(*(int *)(param_1 + 4) + 0x318),0,0,&local_1c,0);
    FUN_0058f8c9(local_2c,local_1c,
                 *(int *)(*(int *)(param_1 + 4) + 0x28) * *(int *)(*(int *)(param_1 + 4) + 0xc));
    local_14 = *(uint *)(*(int *)(param_1 + 4) + 0xc) >> 2;
    local_20 = *(int *)(*(int *)(param_1 + 4) + 0x28);
    for (local_28 = 0; local_28 < local_20 + -1; local_28 = local_28 + 1) {
      if (*(float *)(*(int *)(param_1 + 4) + 0x344) < *(float *)(local_1c + local_28 * local_14 * 4)
         ) {
        *(undefined4 *)(*(int *)(param_1 + 4) + 0x344) =
             *(undefined4 *)(local_1c + local_28 * local_14 * 4);
      }
      if (*(float *)(local_1c + local_28 * local_14 * 4) < *(float *)(*(int *)(param_1 + 4) + 0x348)
         ) {
        *(undefined4 *)(*(int *)(param_1 + 4) + 0x348) =
             *(undefined4 *)(local_1c + local_28 * local_14 * 4);
      }
      if (*(float *)(*(int *)(param_1 + 4) + 0x34c) <
          *(float *)(local_1c + 4 + local_28 * local_14 * 4)) {
        *(undefined4 *)(*(int *)(param_1 + 4) + 0x34c) =
             *(undefined4 *)(local_1c + 4 + local_28 * local_14 * 4);
      }
      if (*(float *)(local_1c + 4 + local_28 * local_14 * 4) <
          *(float *)(*(int *)(param_1 + 4) + 0x350)) {
        *(undefined4 *)(*(int *)(param_1 + 4) + 0x350) =
             *(undefined4 *)(local_1c + 4 + local_28 * local_14 * 4);
      }
      if (*(float *)(*(int *)(param_1 + 4) + 0x354) <
          *(float *)(local_1c + 8 + local_28 * local_14 * 4)) {
        *(undefined4 *)(*(int *)(param_1 + 4) + 0x354) =
             *(undefined4 *)(local_1c + 8 + local_28 * local_14 * 4);
      }
      if (*(float *)(local_1c + 8 + local_28 * local_14 * 4) <
          *(float *)(*(int *)(param_1 + 4) + 0x358)) {
        *(undefined4 *)(*(int *)(param_1 + 4) + 0x358) =
             *(undefined4 *)(local_1c + 8 + local_28 * local_14 * 4);
      }
    }
    if (*(float *)(*(int *)(param_1 + 4) + 0x344) <= _DAT_005a3534) {
      local_44 = -*(float *)(*(int *)(param_1 + 4) + 0x344);
    }
    else {
      local_44 = *(float *)(*(int *)(param_1 + 4) + 0x344);
    }
    if (*(float *)(*(int *)(param_1 + 4) + 0x340) < local_44) {
      if (*(float *)(*(int *)(param_1 + 4) + 0x344) <= _DAT_005a3534) {
        local_48 = -*(float *)(*(int *)(param_1 + 4) + 0x344);
      }
      else {
        local_48 = *(float *)(*(int *)(param_1 + 4) + 0x344);
      }
      *(float *)(*(int *)(param_1 + 4) + 0x340) = local_48;
    }
    if (*(float *)(*(int *)(param_1 + 4) + 0x348) <= _DAT_005a3534) {
      local_4c = -*(float *)(*(int *)(param_1 + 4) + 0x348);
    }
    else {
      local_4c = *(float *)(*(int *)(param_1 + 4) + 0x348);
    }
    if (*(float *)(*(int *)(param_1 + 4) + 0x340) < local_4c) {
      if (*(float *)(*(int *)(param_1 + 4) + 0x348) <= _DAT_005a3534) {
        local_50 = -*(float *)(*(int *)(param_1 + 4) + 0x348);
      }
      else {
        local_50 = *(float *)(*(int *)(param_1 + 4) + 0x348);
      }
      *(float *)(*(int *)(param_1 + 4) + 0x340) = local_50;
    }
    if (*(float *)(*(int *)(param_1 + 4) + 0x34c) <= _DAT_005a3534) {
      local_54 = -*(float *)(*(int *)(param_1 + 4) + 0x34c);
    }
    else {
      local_54 = *(float *)(*(int *)(param_1 + 4) + 0x34c);
    }
    if (*(float *)(*(int *)(param_1 + 4) + 0x340) < local_54) {
      if (*(float *)(*(int *)(param_1 + 4) + 0x34c) <= _DAT_005a3534) {
        local_58 = -*(float *)(*(int *)(param_1 + 4) + 0x34c);
      }
      else {
        local_58 = *(float *)(*(int *)(param_1 + 4) + 0x34c);
      }
      *(float *)(*(int *)(param_1 + 4) + 0x340) = local_58;
    }
    if (*(float *)(*(int *)(param_1 + 4) + 0x350) <= _DAT_005a3534) {
      local_5c = -*(float *)(*(int *)(param_1 + 4) + 0x350);
    }
    else {
      local_5c = *(float *)(*(int *)(param_1 + 4) + 0x350);
    }
    if (*(float *)(*(int *)(param_1 + 4) + 0x340) < local_5c) {
      if (*(float *)(*(int *)(param_1 + 4) + 0x350) <= _DAT_005a3534) {
        local_60 = -*(float *)(*(int *)(param_1 + 4) + 0x350);
      }
      else {
        local_60 = *(float *)(*(int *)(param_1 + 4) + 0x350);
      }
      *(float *)(*(int *)(param_1 + 4) + 0x340) = local_60;
    }
    if (*(float *)(*(int *)(param_1 + 4) + 0x354) <= _DAT_005a3534) {
      local_64 = -*(float *)(*(int *)(param_1 + 4) + 0x354);
    }
    else {
      local_64 = *(float *)(*(int *)(param_1 + 4) + 0x354);
    }
    if (*(float *)(*(int *)(param_1 + 4) + 0x340) < local_64) {
      if (*(float *)(*(int *)(param_1 + 4) + 0x354) <= _DAT_005a3534) {
        local_68 = -*(float *)(*(int *)(param_1 + 4) + 0x354);
      }
      else {
        local_68 = *(float *)(*(int *)(param_1 + 4) + 0x354);
      }
      *(float *)(*(int *)(param_1 + 4) + 0x340) = local_68;
    }
    if (*(float *)(*(int *)(param_1 + 4) + 0x358) <= _DAT_005a3534) {
      local_6c = -*(float *)(*(int *)(param_1 + 4) + 0x358);
    }
    else {
      local_6c = *(float *)(*(int *)(param_1 + 4) + 0x358);
    }
    if (*(float *)(*(int *)(param_1 + 4) + 0x340) < local_6c) {
      if (*(float *)(*(int *)(param_1 + 4) + 0x358) <= _DAT_005a3534) {
        local_70 = -*(float *)(*(int *)(param_1 + 4) + 0x358);
      }
      else {
        local_70 = *(float *)(*(int *)(param_1 + 4) + 0x358);
      }
      *(float *)(*(int *)(param_1 + 4) + 0x340) = local_70;
    }
    (**(code **)(**(int **)(*(int *)(param_1 + 4) + 0x318) + 0x30))
              (*(undefined4 *)(*(int *)(param_1 + 4) + 0x318));
    (**(code **)(**(int **)(DAT_005ccf9c + 0x2a3e4) + 0x60))
              (*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),*(int *)(*(int *)(param_1 + 4) + 0x20) << 1,0
               ,0x65,1,*(int *)(param_1 + 4) + 0x31c);
    (**(code **)(**(int **)(*(int *)(param_1 + 4) + 0x31c) + 0x2c))
              (*(undefined4 *)(*(int *)(param_1 + 4) + 0x31c),0,0,&local_18,0);
    FUN_0058f8c9(local_2c,local_18,*(int *)(*(int *)(param_1 + 4) + 0x20) << 1);
    (**(code **)(**(int **)(*(int *)(param_1 + 4) + 0x31c) + 0x30))
              (*(undefined4 *)(*(int *)(param_1 + 4) + 0x31c));
    *(undefined4 *)(param_1 + 0xbc) = 1;
    FUN_0058f75f(local_2c);
  }
  else {
    *(undefined4 *)(param_1 + 4) =
         *(undefined4 *)(DAT_006703a8 + 0x41808 + *(int *)(param_1 + 0xd0) * 0x58);
    if (*(int *)(param_1 + 4) == 0) {
      ExceptionList = local_10;
      return 0;
    }
    if (*(int *)(*(int *)(param_1 + 4) + 8) == 1) {
      ExceptionList = local_10;
      return 0;
    }
    *(undefined4 *)(param_1 + 0x10) =
         *(undefined4 *)(DAT_006703a8 + 0x4180c + *(int *)(param_1 + 0xd0) * 0x58);
    *(undefined4 *)(param_1 + 8) =
         *(undefined4 *)(DAT_006703a8 + 0x41814 + *(int *)(param_1 + 0xd0) * 0x58);
    *(undefined4 *)(param_1 + 0xb8) =
         *(undefined4 *)(DAT_006703a8 + 0x41818 + *(int *)(param_1 + 0xd0) * 0x58);
    *(undefined4 *)(param_1 + 0xc) =
         *(undefined4 *)(DAT_006703a8 + 0x4181c + *(int *)(param_1 + 0xd0) * 0x58);
    *(undefined4 *)(param_1 + 0xb4) =
         *(undefined4 *)(DAT_006703a8 + 0x41810 + *(int *)(param_1 + 0xd0) * 0x58);
    *(uint *)(param_1 + 0xcc) = *(uint *)(*(int *)(param_1 + 4) + 0x20) / 3;
    *(undefined4 *)(param_1 + 0xbc) = 1;
  }
  ExceptionList = local_10;
  return 1;
}

