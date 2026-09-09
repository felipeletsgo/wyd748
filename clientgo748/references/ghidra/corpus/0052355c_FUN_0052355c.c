// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052355c | Name: FUN_0052355c


/* WARNING: Removing unreachable block (ram,0x00523642) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0052355c(int param_1)

{
  void *pvVar1;
  undefined4 *puVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  int *local_84;
  int local_80;
  int local_7c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  int local_38;
  int local_34;
  undefined4 local_30;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a21a9;
  local_10 = ExceptionList;
  if (*(int *)(param_1 + 0x214) == 0) {
    ExceptionList = &local_10;
    if ((param_1 == *(int *)(DAT_0067cf38 + 0x4c)) &&
       (ExceptionList = &local_10, *(int *)(DAT_0067cf38 + 0x24) == 30000)) {
      local_1c = DAT_0067cf38;
      ExceptionList = &local_10;
      *(undefined4 *)(DAT_0067cf38 + 0x26e90) = 0;
    }
    if (((*(short *)(param_1 + 0x650) != 0) &&
        (local_20 = (**(code **)(*DAT_013b71e8 + 0x34))(*(undefined2 *)(param_1 + 0x650)),
        local_20 != 0)) && (*(char *)(local_20 + 0x78f) == '\x01')) {
      FUN_00430f50(*(undefined4 *)(param_1 + 0x28),*(float *)(param_1 + 0x30) + _DAT_005a3660,
                   *(undefined4 *)(param_1 + 0x2c));
      local_30 = 0;
    }
    if (*(char *)(param_1 + 0x245) != '\x01') {
      local_18 = *(int *)(param_1 + 0x414);
      local_14 = local_18;
      if ((float)_DAT_005a45c8 < *(float *)(param_1 + 0x230)) {
        local_18 = *(int *)(param_1 + 0x414) + 1;
        local_14 = local_18;
      }
      while (local_14 = local_14 + 1, local_14 < 0x30) {
        FUN_0040bd30(param_1 + 0x290 + local_18 * 8);
      }
      FUN_0051dba4(0xb,0);
      if (*(int *)(param_1 + 0x24c) == 0x2c) {
        FUN_00430f50(*(undefined4 *)(param_1 + 0x28),*(float *)(param_1 + 0x30) + _DAT_005a365c,
                     *(undefined4 *)(param_1 + 0x2c));
        pvVar1 = operator_new(0x90);
        local_8 = 0;
        if (pvVar1 == (void *)0x0) {
          local_7c = 0;
        }
        else {
          uVar4 = 1000;
          puVar2 = (undefined4 *)FUN_00430f50(0,0,0);
          local_7c = FUN_004d4de0(local_48,local_44,local_40,4,0xc,0x3d4ccccd,0xffffaa00,0,0x38,
                                  0x3f800000,1,*puVar2,puVar2[1],puVar2[2],uVar4);
        }
        local_8 = 0xffffffff;
        local_34 = local_7c;
        if (local_7c != 0) {
          FUN_0054ac09(local_7c);
        }
        local_3c = 0x44444444;
        pvVar1 = operator_new(0x9c);
        local_8 = 1;
        if (pvVar1 == (void *)0x0) {
          local_80 = 0;
        }
        else {
          local_80 = FUN_004e2360(local_48,local_44,local_40,0,0x3f800000,0xd2,local_3c);
        }
        local_8 = 0xffffffff;
        local_38 = local_80;
        if (local_80 != 0) {
          FUN_0054ac09(local_80);
        }
        pvVar1 = operator_new(0x180);
        local_8 = 2;
        if (pvVar1 == (void *)0x0) {
          local_84 = (int *)0x0;
        }
        else {
          local_84 = (int *)FUN_004c8f70(0x3b,0x9c4,0x3e4ccccd,0x3e4ccccd,0x3e4ccccd,0x3b449ba6,1,
                                         0x50);
        }
        local_8 = 0xffffffff;
        if (local_84 != (int *)0x0) {
          uVar4 = FUN_004310a0(&local_48);
          FUN_004310a0(uVar4);
          local_84[0x1b] = 0;
          if ((*(int *)(DAT_005ccf9c + 0x2a39c) == 1) || (*(int *)(DAT_005ccf9c + 0x2a3a0) == 1)) {
            local_84[0x1b] = 1;
          }
          (**(code **)(*local_84 + 0x3c))(0xffffffff);
          FUN_0054ac09(local_84);
        }
        if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x135), iVar3 != 0)) {
          uVar5 = 0;
          uVar4 = 0;
          FUN_00429a6d(0x135);
          FUN_0042ad2b(uVar4,uVar5);
        }
      }
      *(undefined1 *)(param_1 + 0x77c) = 0;
      *(undefined1 *)(param_1 + 0x77d) = 0;
      *(undefined1 *)(param_1 + 0x77e) = 0;
      *(undefined1 *)(param_1 + 0x77f) = 0;
      *(undefined1 *)(param_1 + 0x781) = 0;
      *(undefined1 *)(param_1 + 0x783) = 0;
      *(undefined1 *)(param_1 + 0x784) = 0;
      *(undefined1 *)(param_1 + 0x785) = 0;
      *(undefined1 *)(param_1 + 0x788) = 0;
      *(undefined1 *)(param_1 + 0x786) = 0;
      *(undefined1 *)(param_1 + 0x787) = 0;
      *(undefined1 *)(param_1 + 0x789) = 0;
      *(undefined1 *)(param_1 + 0x78a) = 0;
      *(undefined1 *)(param_1 + 0x78b) = 0;
      FUN_00524324(0);
      if (*(int *)(param_1 + 0x430) != 0) {
        (**(code **)(**(int **)(param_1 + 0x430) + 0x60))(0);
      }
      iVar3 = DAT_0067cf38;
      if ((param_1 == *(int *)(DAT_0067cf38 + 0x4c)) && (*(int *)(DAT_0067cf38 + 0x24) == 30000)) {
        uVar4 = (**(code **)(*DAT_0092e654 + 8))();
        *(undefined4 *)(iVar3 + 0x27454) = uVar4;
      }
      *(undefined1 *)(param_1 + 0x249) = 0;
      *(undefined1 *)(param_1 + 0x245) = 1;
      *(undefined4 *)(param_1 + 100) = *(undefined4 *)(param_1 + 0x28);
      *(float *)(param_1 + 0x68) = *(float *)(param_1 + 0x30) + _DAT_005a430c;
      *(undefined4 *)(param_1 + 0x6c) = *(undefined4 *)(param_1 + 0x2c);
      (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x530));
      *(undefined4 *)(param_1 + 0x530) = 0;
      if ((*(int *)(param_1 + 0x24c) == 0x38) && (*(short *)(param_1 + 0x1d2) == 0)) {
        FUN_00523b60();
      }
    }
  }
  ExceptionList = local_10;
  return;
}

