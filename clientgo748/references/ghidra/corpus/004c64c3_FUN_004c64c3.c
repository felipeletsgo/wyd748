// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c64c3 | Name: FUN_004c64c3


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004c64c3(int param_1)

{
  undefined4 *puVar1;
  int iVar2;
  float10 fVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined *puVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  int *local_18c;
  int *local_188;
  int local_184;
  int *local_180;
  int local_17c;
  int *local_178;
  int local_174;
  int *local_170;
  int *local_16c;
  int *local_168;
  undefined1 local_15c [12];
  undefined1 local_150 [12];
  void *local_144;
  int *local_140;
  undefined1 local_13c [12];
  undefined1 local_130 [12];
  void *local_124;
  int *local_120;
  undefined1 local_11c [12];
  undefined1 local_110 [12];
  void *local_104;
  int local_100;
  void *local_f4;
  int *local_f0;
  void *local_ec;
  int local_e8;
  void *local_dc;
  int *local_d8;
  void *local_d4;
  int local_d0;
  void *local_cc;
  int *local_c8;
  void *local_bc;
  int *local_b8;
  void *local_ac;
  int *local_a8;
  undefined4 *local_a4;
  undefined4 *local_a0;
  int local_9c;
  int *local_98;
  undefined4 local_94 [2];
  undefined4 local_8c;
  int *local_88;
  undefined4 local_84;
  undefined *local_80;
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  int *local_70;
  undefined4 local_6c;
  undefined4 local_68;
  float local_64 [2];
  float local_5c;
  float local_58;
  undefined4 local_54;
  float local_50;
  int local_4c;
  undefined *local_48;
  int local_44;
  float local_40;
  undefined4 local_3c;
  float local_38;
  int *local_34;
  int local_30;
  int *local_2c;
  int local_28;
  int *local_24;
  int *local_20 [3];
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a07b0;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x6c) != 0) {
    local_a4 = *(undefined4 **)(param_1 + 0x6c);
    ExceptionList = &local_10;
    local_a0 = local_a4;
    if (local_a4 != (undefined4 *)0x0) {
      ExceptionList = &local_10;
      (**(code **)*local_a4)(1);
    }
    *(undefined4 *)(param_1 + 0x6c) = 0;
  }
  if ((*(int *)(param_1 + 0x68) != 0) &&
     (local_14 = (**(code **)(*DAT_0092e654 + 8))(),
     local_14 < (uint)(*(int *)(param_1 + 0x24) + 2000 + *(int *)(param_1 + 100)))) {
    *(uint *)(*(int *)(param_1 + 0x68) + 0x84) =
         (local_14 - *(int *)(*(int *)(param_1 + 0x68) + 0x80)) + 300;
    FUN_004c8cc0(local_14);
  }
  if ((*(int *)(param_1 + 0x5c) != 10000) && (*(int *)(param_1 + 0x5c) != 0x2712)) {
    if (*(int *)(param_1 + 0x5c) == 0x2713) {
      local_ac = operator_new(0xc0);
      local_8 = 0;
      if (local_ac == (void *)0x0) {
        local_168 = (int *)0x0;
      }
      else {
        local_168 = (int *)FUN_004df250(3,7,0x3f800000);
      }
      local_a8 = local_168;
      local_8 = 0xffffffff;
      local_20[2] = local_168;
      if (local_168 != (int *)0x0) {
        (**(code **)(*local_168 + 0x44))(0x80ffffff);
        local_20[2][0x1b] = 1;
        puVar1 = (undefined4 *)
                 FUN_00430f20(*(undefined4 *)(param_1 + 0x40),*(undefined4 *)(param_1 + 0x48));
        (**(code **)(*local_20[2] + 0x3c))(*puVar1,puVar1[1]);
        local_20[2][0x28] = 400;
        FUN_0054ac09(local_20[2]);
      }
    }
    else {
      if ((*(int *)(param_1 + 0x5c) == 0x98) && (*(int *)(param_1 + 0x4c) == 2)) {
        local_20[0] = (int *)0xffffffff;
        local_20[1] = (int *)0xff003377;
        local_bc = operator_new(0xc0);
        local_8 = 1;
        if (local_bc == (void *)0x0) {
          local_16c = (int *)0x0;
        }
        else {
          local_16c = (int *)FUN_004df250(1,7,0x3f800000);
        }
        local_b8 = local_16c;
        local_8 = 0xffffffff;
        local_24 = local_16c;
        if (local_16c != (int *)0x0) {
          (**(code **)(*local_16c + 0x44))(local_20[1]);
          local_24[0x1b] = 1;
          puVar1 = (undefined4 *)
                   FUN_00430f20(*(undefined4 *)(param_1 + 0x40),*(undefined4 *)(param_1 + 0x48));
          (**(code **)(*local_24 + 0x3c))(*puVar1,puVar1[1]);
          local_24[0x28] = 800;
          FUN_0054ac09(local_24);
        }
        for (local_28 = 0; local_28 < 2; local_28 = local_28 + 1) {
          local_cc = operator_new(0x180);
          local_8 = 2;
          if (local_cc == (void *)0x0) {
            local_170 = (int *)0x0;
          }
          else {
            local_170 = (int *)FUN_004c8f70(0x38,700,(float)local_28 * _DAT_005a4158 + _DAT_005a34a0
                                            ,(float)local_28 * _DAT_005a4158 + _DAT_005a34a0,
                                            (float)local_28 * _DAT_005a4158 + _DAT_005a34a0,
                                            0x3a83126f,1,0x50);
          }
          local_c8 = local_170;
          local_8 = 0xffffffff;
          local_2c = local_170;
          if (local_170 != (int *)0x0) {
            (**(code **)(*local_170 + 0x3c))(local_20[local_28]);
            local_2c[0x1b] = 1;
            local_2c[0x41] = 1;
            FUN_004310a0(param_1 + 0x40);
            FUN_0054ac09(local_2c);
          }
        }
      }
      else if (*(int *)(param_1 + 0x5c) == 0x2711) {
        local_d4 = operator_new(0x124);
        local_8 = 3;
        if (local_d4 == (void *)0x0) {
          local_174 = 0;
        }
        else {
          local_174 = FUN_004cb900(8,1000,0x3c23d70a,0x3c23d70a,0x3c23d70a,0x3b449ba6,0);
        }
        local_d0 = local_174;
        local_8 = 0xffffffff;
        local_30 = local_174;
        if (local_174 != 0) {
          *(undefined4 *)(local_174 + 0x6c) = 1;
          FUN_004310a0(param_1 + 0x34);
          *(float *)(local_30 + 0x78) = *(float *)(local_30 + 0x78) - _DAT_005a414c;
          FUN_0054ac09(local_30);
        }
        local_dc = operator_new(0xc0);
        local_8 = 4;
        if (local_dc == (void *)0x0) {
          local_178 = (int *)0x0;
        }
        else {
          local_178 = (int *)FUN_004df250(4,7,0x3f800000);
        }
        local_d8 = local_178;
        local_8 = 0xffffffff;
        local_34 = local_178;
        if (local_178 != (int *)0x0) {
          (**(code **)(*local_178 + 0x44))(0xff7777ff);
          local_34[0x1b] = 1;
          puVar1 = (undefined4 *)
                   FUN_00430f20(*(undefined4 *)(param_1 + 0x34),*(undefined4 *)(param_1 + 0x3c));
          (**(code **)(*local_34 + 0x3c))(*puVar1,puVar1[1]);
          local_34[0x28] = 800;
          FUN_0054ac09(local_34);
        }
      }
      if ((*(int *)(param_1 + 0x5c) == 0x69) || (*(int *)(param_1 + 0x5c) == 0x2711)) {
        if ((DAT_005ccf98 != 0) && (iVar2 = FUN_00429a6d(0x1a), iVar2 != 0)) {
          uVar13 = 0;
          uVar12 = 0;
          FUN_00429a6d(0x1a);
          FUN_0042ad2b(uVar12,uVar13);
        }
      }
      else {
        if ((DAT_005ccf98 != 0) && (iVar2 = FUN_00429a6d(0x18), iVar2 != 0)) {
          uVar13 = 0;
          uVar12 = 0;
          FUN_00429a6d(0x18);
          FUN_0042ad2b(uVar12,uVar13);
        }
        local_84 = 0x3f800000;
        local_68 = 0x3f000000;
        local_58 = *(float *)(param_1 + 0x40);
        local_54 = *(undefined4 *)(param_1 + 0x44);
        local_50 = *(float *)(param_1 + 0x48);
        FUN_00430f50(*(float *)(param_1 + 0x34) - *(float *)(param_1 + 0x40),0,
                     *(float *)(param_1 + 0x3c) - *(float *)(param_1 + 0x48));
        thunk_FUN_005611dd(local_64,local_64);
        local_4c = 0;
        local_6c = 0x80ffffff;
        local_80 = (undefined *)0x52a9e5;
        local_48 = (undefined *)0x334388;
        if (*(int *)(param_1 + 0x70) == 8) {
          local_6c = 0x80ffcccc;
          local_80 = &DAT_00e57777;
          local_48 = &DAT_00883333;
        }
        else if (*(int *)(param_1 + 0x70) == 7) {
          local_6c = 0x80ffccff;
          local_80 = &DAT_00cc88cc;
          local_48 = &DAT_00884388;
        }
        else if (*(int *)(param_1 + 0x70) == 6) {
          local_6c = 0x80ccffcc;
          local_80 = &DAT_0088e588;
          local_48 = (undefined *)0x338843;
        }
        else if (*(int *)(param_1 + 0x70) == 5) {
          local_6c = 0x80ccccff;
          local_80 = (undefined *)0x5253e5;
          local_48 = (undefined *)0x222288;
        }
        if (*(int *)(param_1 + 0x5c) == 0x99) {
          FUN_00493db0(local_94,param_1 + 0x28);
          fVar3 = (float10)FUN_00493c30(local_94[0],local_8c);
          *(float *)(param_1 + 0x50) = (float)(fVar3 + (float10)_DAT_005a4380);
        }
        if (*(char *)(param_1 + 0x78) == '\x01') {
          local_6c = 0xffffffff;
          local_80 = (undefined *)0xffffffff;
          local_48 = (undefined *)0xffffffff;
        }
        local_ec = operator_new(0xcc);
        local_8 = 5;
        if (local_ec == (void *)0x0) {
          local_17c = 0;
        }
        else {
          local_17c = FUN_004d2230(0x213,local_6c,*(undefined4 *)(param_1 + 0x50),4);
        }
        local_e8 = local_17c;
        local_8 = 0xffffffff;
        local_4c = local_17c;
        if (local_17c != 0) {
          *(undefined4 *)(local_17c + 0xa4) = 0xe5;
          *(undefined4 *)(local_17c + 0xb0) = 500;
          *(undefined4 *)(local_17c + 0xb8) = 500;
          FUN_004310a0(&local_58);
          *(undefined4 *)(local_4c + 0x98) = 0x3f800000;
          *(undefined4 *)(local_4c + 0x9c) = 0x3f800000;
          *(undefined4 *)(local_4c + 0x6c) = 1;
          *(undefined1 *)(local_4c + 0xac) = 0;
          FUN_0054ac09(local_4c);
          local_98 = (int *)0x0;
          local_f4 = operator_new(0xc0);
          local_8 = 6;
          if (local_f4 == (void *)0x0) {
            local_180 = (int *)0x0;
          }
          else {
            local_180 = (int *)FUN_004df250(7,0x76,0x3f800000);
          }
          local_f0 = local_180;
          local_8 = 0xffffffff;
          local_98 = local_180;
          if (local_180 != (int *)0x0) {
            (**(code **)(*local_180 + 0x44))(local_48);
            local_98[0x1b] = 1;
            puVar1 = (undefined4 *)
                     FUN_00430f20(local_64[0] * _DAT_005a34a0 + local_58,
                                  local_5c * _DAT_005a34a0 + local_50);
            (**(code **)(*local_98 + 0x3c))(*puVar1,puVar1[1]);
            local_98[0x28] = 300;
            FUN_0054ac09(local_98);
          }
        }
        if ((*(int *)(param_1 + 0x74) != 0) &&
           (local_9c = (**(code **)(*DAT_013b71e8 + 0x34))(*(undefined4 *)(param_1 + 0x74)),
           local_9c != 0)) {
          *(undefined1 *)(local_9c + 0x7cc) = 1;
          uVar12 = (**(code **)(*DAT_0092e654 + 8))();
          *(undefined4 *)(local_9c + 2000) = uVar12;
        }
        local_44 = 0;
        CSimpleArray<>();
        FUN_0058f220(&local_7c,local_64,0xc);
        local_104 = operator_new(0x90);
        local_8 = 7;
        if (local_104 == (void *)0x0) {
          local_184 = 0;
        }
        else {
          uVar11 = 800;
          uVar10 = 1;
          uVar9 = 0x3f800000;
          uVar8 = 0xe7;
          uVar7 = 0;
          puVar6 = &DAT_00ffeeaa;
          uVar5 = 0x3dcccccd;
          uVar4 = 0x14;
          uVar13 = 5;
          uVar12 = FUN_004c8c60(local_110,0x3e99999a);
          puVar1 = (undefined4 *)FUN_00493d50(local_11c,uVar12);
          local_184 = FUN_004d4de0(*puVar1,puVar1[1],puVar1[2],uVar13,uVar4,uVar5,puVar6,uVar7,uVar8
                                   ,uVar9,uVar10,local_7c,local_78,local_74,uVar11);
        }
        local_100 = local_184;
        local_8 = 0xffffffff;
        local_44 = local_184;
        if (local_184 != 0) {
          FUN_0054ac09(local_184);
        }
        local_88 = (int *)0x0;
        local_40 = local_58;
        local_3c = local_54;
        local_38 = local_50;
        local_124 = operator_new(0x180);
        local_8 = 8;
        if (local_124 == (void *)0x0) {
          local_188 = (int *)0x0;
        }
        else {
          local_188 = (int *)FUN_004c8f70(0xe6,600,0x40400000,0x40400000,0x40400000,0x3a03126f,1,
                                          0x50);
        }
        local_120 = local_188;
        local_8 = 0xffffffff;
        local_88 = local_188;
        if (local_188 != (int *)0x0) {
          local_188[0x42] = 0;
          uVar12 = FUN_004c8c60(local_130,0x3f000000);
          uVar12 = FUN_00493d50(local_13c,uVar12);
          FUN_004310a0(uVar12);
          FUN_004310a0(&local_7c);
          local_88[0x45] = (int)(*(float *)(param_1 + 0x50) + _DAT_005a4380 + _DAT_005a439c);
          local_88[0x1b] = 1;
          local_88[0x52] = 0xe;
          local_88[0x3b] = 0x3a83126f;
          local_88[0x3c] = 0x3a83126f;
          local_88[0x3d] = 0x3a83126f;
          (**(code **)(*local_88 + 0x3c))(local_80);
          FUN_0054ac09(local_88);
        }
        local_70 = (int *)0x0;
        local_144 = operator_new(0x180);
        local_8 = 9;
        if (local_144 == (void *)0x0) {
          local_18c = (int *)0x0;
        }
        else {
          local_18c = (int *)FUN_004c8f70(0xe6,600,0x40400000,0x40400000,0x40400000,0x3a03126f,1,
                                          0x50);
        }
        local_140 = local_18c;
        local_8 = 0xffffffff;
        local_70 = local_18c;
        if (local_18c != (int *)0x0) {
          local_18c[0x42] = 0;
          uVar12 = FUN_004c8c60(local_150,0x3f000000);
          uVar12 = FUN_00493d50(local_15c,uVar12);
          FUN_004310a0(uVar12);
          FUN_004310a0(&local_7c);
          local_70[0x45] = (int)((*(float *)(param_1 + 0x50) + _DAT_005a4380) - _DAT_005a439c);
          local_70[0x1b] = 1;
          local_70[0x52] = 0xf;
          local_70[0x3b] = 0x3a83126f;
          local_70[0x3c] = 0x3a83126f;
          local_70[0x3d] = 0x3a83126f;
          (**(code **)(*local_70 + 0x3c))(local_80);
          FUN_0054ac09(local_70);
        }
      }
    }
  }
  ExceptionList = local_10;
  return;
}

