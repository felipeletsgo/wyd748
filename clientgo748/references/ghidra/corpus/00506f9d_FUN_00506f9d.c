// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00506f9d | Name: FUN_00506f9d


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00506f9d(int param_1,uint param_2)

{
  float fVar1;
  void *pvVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 *puVar5;
  uint uVar6;
  float10 fVar7;
  undefined4 uVar8;
  float fVar9;
  undefined4 uVar10;
  float fVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  int *local_a20;
  int *local_a1c;
  undefined4 local_a18;
  undefined4 local_a14;
  undefined4 local_a10;
  undefined4 local_a0c;
  undefined4 local_a00;
  undefined4 local_9fc;
  undefined4 local_9f8;
  int *local_9ec;
  int *local_9e0;
  undefined4 local_9dc;
  undefined4 local_9d8;
  undefined4 local_9d4;
  int *local_9d0;
  undefined4 local_9cc;
  int local_9c8;
  undefined4 local_9b8;
  undefined4 local_9b4;
  int local_9b0;
  undefined4 local_9a0;
  int local_998;
  int local_994;
  int local_990;
  int local_98c;
  int *local_988;
  int *local_974;
  int *local_970;
  int local_96c;
  int local_968;
  int *local_964;
  int *local_960;
  int *local_95c;
  int local_954;
  int *local_950;
  int *local_94c;
  int *local_948;
  int *local_93c;
  int local_938;
  int *local_934;
  int *local_930;
  int *local_92c;
  int local_928;
  int local_924;
  int local_920;
  int local_91c;
  int *local_918;
  int *local_914;
  int local_90c;
  int local_908;
  int local_900;
  int local_8fc;
  int *local_8f0;
  int local_8e4;
  int local_8dc;
  int local_8d8;
  int *local_8d4;
  int local_8d0;
  int local_8cc;
  int local_8c4;
  int local_8c0;
  int local_8bc;
  int local_8b8;
  int local_8ac;
  int *local_8a8;
  int local_8a4;
  int local_8a0;
  int local_89c;
  int local_898;
  int *local_894;
  int *local_890;
  int local_888;
  undefined4 local_378;
  float local_374;
  undefined4 local_370;
  int local_36c;
  undefined4 local_368;
  int local_364;
  undefined4 local_360;
  undefined4 local_35c;
  int *local_358;
  undefined4 local_354;
  undefined4 local_350;
  undefined4 local_34c;
  undefined4 local_348;
  undefined4 local_344;
  undefined4 local_340;
  undefined4 local_33c;
  undefined4 local_338;
  undefined4 local_334;
  undefined4 local_330;
  undefined4 local_32c;
  undefined4 local_328;
  undefined4 local_324;
  undefined4 local_320;
  undefined4 local_31c;
  undefined4 local_318;
  int local_314;
  int local_310;
  undefined4 local_30c;
  undefined4 local_308;
  undefined4 local_304;
  undefined4 local_300;
  int local_2fc;
  int *local_2f8;
  int *local_2f4;
  int local_2f0;
  undefined4 local_2ec;
  int *local_2e8;
  undefined4 local_2e4;
  undefined4 local_2e0;
  undefined4 local_2dc;
  int local_2d8;
  int local_2d4;
  float local_2d0;
  uint local_2cc;
  undefined4 local_2c8 [6];
  undefined4 local_2b0;
  undefined4 local_2ac;
  int local_2a8;
  uint local_2a4;
  int local_2a0;
  float local_29c;
  float local_298;
  int local_294;
  int local_290;
  int local_28c;
  int local_288;
  int local_284;
  float local_280;
  float local_27c;
  float local_278;
  undefined *local_274;
  int local_270;
  int *local_26c;
  int local_268;
  int local_264;
  int local_260;
  int local_25c;
  float local_258;
  float local_254;
  float local_250;
  float local_24c;
  float local_248;
  float local_244;
  undefined4 local_240;
  int *local_23c;
  int local_238;
  undefined4 local_234;
  int *local_230;
  undefined4 local_22c;
  undefined4 local_228;
  undefined4 local_224;
  int local_220;
  undefined4 local_21c;
  undefined4 local_218;
  undefined4 local_214;
  int local_210;
  int local_20c;
  int *local_208;
  int local_204;
  float local_200;
  int *local_1fc;
  int *local_1f8;
  int local_1f4;
  int local_1f0;
  int local_1ec [10];
  int *local_1c4;
  int local_1c0;
  int *local_1bc;
  int local_1b8;
  int *local_1b4;
  uint local_1b0;
  int local_1ac [5];
  float local_198;
  int *local_194;
  int local_190;
  undefined4 local_18c;
  undefined4 local_188;
  undefined4 local_184;
  int local_180;
  int *local_17c;
  int local_178;
  int local_174;
  int local_170;
  int *local_16c;
  int local_168;
  int local_164;
  int local_160;
  int *local_15c;
  int local_158;
  float local_154;
  int *local_150;
  int *local_14c;
  int local_148;
  int local_144 [4];
  float local_134;
  float local_130;
  float local_12c;
  float local_128;
  float local_124;
  float local_120;
  int local_11c;
  uint local_118;
  int local_114;
  undefined4 local_110;
  int local_10c;
  undefined4 local_108;
  float local_104;
  int *local_100;
  float local_fc;
  int local_f8;
  int local_f4 [4];
  float local_e4;
  float local_e0;
  float local_dc;
  int local_d8;
  undefined4 local_d4;
  int local_d0;
  int *local_cc;
  int local_c8;
  float local_c4;
  float local_c0;
  float local_bc;
  float local_b8;
  float local_b4;
  float local_b0;
  int local_ac;
  int local_a8;
  int local_a4;
  uint local_a0;
  float local_9c;
  float local_98;
  float local_94;
  float local_90;
  float local_8c;
  float local_88;
  int local_84;
  undefined4 local_80;
  int *local_7c;
  int local_78;
  int local_74;
  uint local_70;
  int local_6c;
  float local_68;
  float local_64;
  float local_60;
  int local_5c;
  float local_58;
  float local_54;
  float local_50;
  float local_4c;
  int *local_48;
  int *local_44;
  undefined4 local_40;
  int local_3c;
  float local_38;
  float local_34;
  float local_30;
  float local_2c;
  float local_28;
  float local_24;
  float local_20;
  int local_1c;
  int local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a1eb1;
  local_10 = ExceptionList;
  if (*(int *)(param_1 + 0x214) == 0) {
    local_14 = DAT_0067cf38;
    if (*(char *)(param_1 + 0x795) == '\0') {
      ExceptionList = &local_10;
      if (DAT_0092e664 == 1) {
        ExceptionList = &local_10;
        if (*(int *)(param_1 + 0x538) != 0) {
          ExceptionList = &local_10;
          *(undefined4 *)(*(int *)(param_1 + 0x538) + 0x24) = 0;
        }
        if (*(int *)(param_1 + 0x514) != 0) {
          *(undefined4 *)(*(int *)(param_1 + 0x514) + 0x24) = 0;
        }
        if (*(int *)(param_1 + 0x518) != 0) {
          *(undefined4 *)(*(int *)(param_1 + 0x518) + 0x24) = 0;
        }
        if (*(int *)(param_1 + 0x520) != 0) {
          *(undefined4 *)(*(int *)(param_1 + 0x520) + 0x24) = 0;
        }
        if (*(int *)(param_1 + 0x550) != 0) {
          *(undefined4 *)(*(int *)(param_1 + 0x550) + 0x24) = 0;
        }
        if (*(int *)(param_1 + 0x528) != 0) {
          *(undefined4 *)(*(int *)(param_1 + 0x528) + 0x24) = 0;
        }
        if (*(int *)(param_1 + 0x52c) != 0) {
          *(undefined4 *)(*(int *)(param_1 + 0x52c) + 0x24) = 0;
        }
        if (*(int *)(param_1 + 0x53c) != 0) {
          *(undefined4 *)(*(int *)(param_1 + 0x53c) + 0x24) = 0;
        }
        if (*(int *)(param_1 + 0x54c) != 0) {
          *(undefined4 *)(*(int *)(param_1 + 0x54c) + 0x24) = 0;
        }
        if (*(int *)(param_1 + 0x55c) != 0) {
          *(undefined4 *)(*(int *)(param_1 + 0x55c) + 0x48) = 0;
        }
        if (*(int *)(param_1 + 0x554) != 0) {
          *(undefined4 *)(*(int *)(param_1 + 0x554) + 0x24) = 0;
        }
        if (*(int *)(param_1 + 0x558) != 0) {
          *(undefined4 *)(*(int *)(param_1 + 0x558) + 0x24) = 0;
        }
        for (local_1c = 0; local_1c < 4; local_1c = local_1c + 1) {
          if (*(int *)(param_1 + 0x5c0 + local_1c * 4) != 0) {
            *(undefined4 *)(*(int *)(param_1 + 0x5c0 + local_1c * 4) + 0x48) = 0;
          }
        }
        for (local_1c = 0; local_1c < 5; local_1c = local_1c + 1) {
          if (*(int *)(param_1 + 0x5d0 + local_1c * 4) != 0) {
            *(undefined4 *)(*(int *)(param_1 + 0x5d0 + local_1c * 4) + 0x24) = 0;
          }
        }
        if (*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) {
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f8) = 0;
          *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f3) = 0;
          *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f1) = 0;
          *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f5) = 0;
          *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f2) = 0;
          *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f4) = 0;
        }
        if (*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0) {
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f8) = 0;
          *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f3) = 0;
          *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f1) = 0;
          *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f5) = 0;
          *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f2) = 0;
          *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f4) = 0;
        }
      }
      if ((*(int *)(param_1 + 0x4bc) != 0) && (*(uint *)(param_1 + 0x4bc) < param_2)) {
        fVar7 = (float10)FUN_00493c30(*(float *)(param_1 + 0x4c0) - *(float *)(param_1 + 0x28),
                                      *(float *)(param_1 + 0x4c8) - *(float *)(param_1 + 0x2c));
        local_20 = (float)(fVar7 + (float10)_DAT_005a4380);
        if (((-1 < *(short *)(param_1 + 0x4d0)) && (*(short *)(param_1 + 0x4d0) < 0x60)) ||
           ((((0x96 < *(short *)(param_1 + 0x4d0) && (*(short *)(param_1 + 0x4d0) < 0x9a)) ||
             (*(short *)(param_1 + 0x4d0) == 0x68)) || (*(short *)(param_1 + 0x4d0) == 0x69)))) {
          FUN_00430f50(*(undefined4 *)(param_1 + 0x28),*(float *)(param_1 + 0x30) + _DAT_005a3660,
                       *(undefined4 *)(param_1 + 0x2c));
          FUN_00430f50(*(undefined4 *)(param_1 + 0x4c0),*(undefined4 *)(param_1 + 0x4c4),
                       *(undefined4 *)(param_1 + 0x4c8));
          if (*(short *)(param_1 + 0x4d0) == 2) {
            pvVar2 = operator_new(0xa8);
            local_8 = 0;
            if (pvVar2 == (void *)0x0) {
              local_888 = 0;
            }
            else {
              local_888 = FUN_004ecd90(local_2c,local_28,local_24,local_2c,local_28,local_24,0,
                                       *(undefined4 *)(param_1 + 0x4cc));
            }
            local_8 = 0xffffffff;
            local_3c = local_888;
            if (local_888 != 0) {
              FUN_0054ac09(local_888);
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 3) {
            if ((*(int *)(param_1 + 0x4cc) != 0) &&
               (*(int *)(*(int *)(param_1 + 0x4cc) + 0x51c) != 0)) {
              FUN_004e8a95(param_2);
            }
            if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x9e), iVar3 != 0)) {
              uVar10 = 0;
              uVar4 = 0;
              FUN_00429a6d(0x9e);
              FUN_0042ad2b(uVar4,uVar10);
            }
          }
          else if ((*(short *)(param_1 + 0x4d0) == 8) || (*(short *)(param_1 + 0x4d0) == 10)) {
            iVar3 = (**(code **)(*DAT_013b71e8 + 0x78))();
            if (param_1 == *(int *)(iVar3 + 0x54)) {
              uVar4 = 2;
              (**(code **)(*DAT_013b71e8 + 0x78))(2);
              FUN_004efb89(uVar4);
            }
            if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0xa0), iVar3 != 0)) {
              uVar10 = 0;
              uVar4 = 0;
              FUN_00429a6d(0xa0);
              FUN_0042ad2b(uVar4,uVar10);
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0xb) {
            local_58 = local_2c;
            local_54 = local_28;
            local_50 = local_24;
            iVar3 = _rand();
            local_4c = (float)(iVar3 % 5);
            pvVar2 = operator_new(0x180);
            local_8 = 1;
            if (pvVar2 == (void *)0x0) {
              local_890 = (int *)0x0;
            }
            else {
              local_890 = (int *)FUN_004c8f70(0x38,700,0x3fcccccd,0x3fcccccd,0x3fcccccd,0x3b03126f,1
                                              ,0x50);
            }
            local_8 = 0xffffffff;
            local_44 = local_890;
            if (local_890 != (int *)0x0) {
              (**(code **)(*local_890 + 0x3c))(0xff990000);
              local_44[0x1b] = 1;
              local_44[0x41] = 1;
              uVar4 = FUN_00430f50(local_58,local_54 + _DAT_005a4158,local_50);
              FUN_004310a0(uVar4);
              FUN_0054ac09(local_44);
            }
            pvVar2 = operator_new(0x180);
            local_8 = 2;
            if (pvVar2 == (void *)0x0) {
              local_894 = (int *)0x0;
            }
            else {
              local_894 = (int *)FUN_004c8f70(0x3c,0x4b0,local_4c * _DAT_005a40ec + _DAT_005a564c,
                                              local_4c * _DAT_005a40ec + _DAT_005a5648,
                                              local_4c * _DAT_005a40ec + _DAT_005a564c,0x3a03126f,1,
                                              0x50);
            }
            local_8 = 0xffffffff;
            local_48 = local_894;
            if (local_894 != (int *)0x0) {
              (**(code **)(*local_894 + 0x3c))(0xff994444);
              local_48[0x45] = (int)((_DAT_005a4148 * local_4c) / _DAT_005a40f0);
              local_48[0x1b] = 1;
              local_48[0x41] = 1;
              uVar4 = FUN_00430f50(local_58,local_54 + _DAT_005a4158,local_50);
              FUN_004310a0(uVar4);
              FUN_0054ac09(local_48);
            }
            local_40 = 0xa8;
            if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0xa8), iVar3 != 0)) {
              uVar10 = 0;
              uVar4 = 0;
              FUN_00429a6d(local_40);
              FUN_0042ad2b(uVar4,uVar10);
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 9) {
            local_68 = local_2c;
            local_64 = local_28;
            local_60 = local_24;
            pvVar2 = operator_new(0x90);
            local_8 = 3;
            if (pvVar2 == (void *)0x0) {
              local_898 = 0;
            }
            else {
              local_898 = FUN_004edd60(local_68,local_64,local_60,0);
            }
            local_8 = 0xffffffff;
            local_5c = local_898;
            if (local_898 != 0) {
              FUN_0054ac09(local_898);
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0xf) {
            pvVar2 = operator_new(0xcc);
            local_8 = 4;
            if (pvVar2 == (void *)0x0) {
              local_89c = 0;
            }
            else {
              local_89c = FUN_004d2230(0xb16,0xff999999,*(undefined4 *)(param_1 + 0x34),4);
            }
            local_8 = 0xffffffff;
            local_6c = local_89c;
            if (local_89c != 0) {
              *(undefined4 *)(local_89c + 0xa4) = 0x19d;
              *(undefined4 *)(local_89c + 0xb0) = 500;
              *(undefined4 *)(local_89c + 0xb8) = 500;
              if (*(char *)(param_1 + 0x79c) == '\x01') {
                uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x50),
                                     ((*(float *)(&DAT_0092e7d4 + *(int *)(param_1 + 0x5c) * 8) *
                                      *(float *)(param_1 + 0x38)) / _DAT_005a365c +
                                     *(float *)(param_1 + 0x54)) - _DAT_005a40ec,
                                     *(undefined4 *)(param_1 + 0x58));
                FUN_004310a0(uVar4);
              }
              else {
                uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                                     (*(float *)(&DAT_0092e7d4 + *(int *)(param_1 + 0x5c) * 8) *
                                     *(float *)(param_1 + 0x38)) / _DAT_005a365c +
                                     *(float *)(param_1 + 0x30) + _DAT_005a40ec,
                                     *(undefined4 *)(param_1 + 0x2c));
                FUN_004310a0(uVar4);
              }
              *(undefined4 *)(local_6c + 0x98) = 0x40200000;
              *(undefined4 *)(local_6c + 0x9c) = 0x40200000;
              *(undefined4 *)(local_6c + 0x6c) = 1;
              *(undefined1 *)(local_6c + 0xac) = 0;
              FUN_0054ac09(local_6c);
            }
          }
          else if (((*(short *)(param_1 + 0x4d0) == 0x10) || (*(short *)(param_1 + 0x4d0) == 0xc))
                  || (*(short *)(param_1 + 0x4d0) == 0x1c)) {
            local_34 = local_34 + _DAT_005a3660;
            local_70 = (uint)(*(short *)(param_1 + 0x4d0) == 0xc);
            if (*(short *)(param_1 + 0x4d0) == 0x1c) {
              local_70 = 2;
            }
            pvVar2 = operator_new(200);
            local_8 = 5;
            if (pvVar2 == (void *)0x0) {
              local_8a0 = 0;
            }
            else {
              local_8a0 = FUN_004e1290(local_2c,local_28,local_24,local_38,local_34,local_30,
                                       local_70,0);
            }
            local_8 = 0xffffffff;
            local_74 = local_8a0;
            if (local_8a0 != 0) {
              FUN_0054ac09(local_8a0);
            }
          }
          else if ((*(short *)(param_1 + 0x4d0) == 0x11) && (*(int *)(param_1 + 0x60) != 0)) {
            if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x9b), iVar3 != 0)) {
              uVar10 = 0;
              uVar4 = 0;
              FUN_00429a6d(0x9b);
              FUN_0042ad2b(uVar4,uVar10);
            }
            if ((*(int *)(param_1 + 0x670) == 1) &&
               (*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0)) {
              *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f0) = 1;
            }
            if ((*(int *)(param_1 + 0x674) == 1) &&
               (*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0)) {
              *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f0) = 1;
            }
            if ((*(int *)(param_1 + 0x670) == 0) && (*(int *)(param_1 + 0x674) == 0)) {
              if (*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) {
                *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f0) = 1;
              }
              if (*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0) {
                *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f0) = 1;
              }
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x12) {
            iVar3 = (**(code **)(*DAT_013b71e8 + 0x78))();
            if (param_1 == *(int *)(iVar3 + 0x54)) {
              uVar4 = 2;
              (**(code **)(*DAT_013b71e8 + 0x78))(2);
              FUN_004efb89(uVar4);
            }
            if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0xa0), iVar3 != 0)) {
              uVar10 = 0;
              uVar4 = 0;
              FUN_00429a6d(0xa0);
              FUN_0042ad2b(uVar4,uVar10);
            }
          }
          else if ((*(short *)(param_1 + 0x4d0) == 0x13) && (*(int *)(param_1 + 0x60) != 0)) {
            if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0xa0), iVar3 != 0)) {
              uVar10 = 0;
              uVar4 = 0;
              FUN_00429a6d(0xa0);
              FUN_0042ad2b(uVar4,uVar10);
            }
            if (*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) {
              *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f8) = 4;
            }
            if (*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0) {
              *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f8) = 4;
            }
            pvVar2 = operator_new(0xb0);
            local_8 = 6;
            if (pvVar2 == (void *)0x0) {
              local_8a4 = 0;
            }
            else {
              local_8a4 = FUN_004e3ba0(local_38,local_34,local_30,0,0,0);
            }
            local_8 = 0xffffffff;
            local_78 = local_8a4;
            if (local_8a4 != 0) {
              FUN_0054ac09(local_8a4);
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x14) {
            if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x99), iVar3 != 0)) {
              uVar10 = 0;
              uVar4 = 0;
              FUN_00429a6d(0x99);
              FUN_0042ad2b(uVar4,uVar10);
            }
            if (*(int *)(param_1 + 0x4cc) != 0) {
              local_80 = 0;
              if ((*(int *)(*(int *)(param_1 + 0x4cc) + 0x24c) == 4) ||
                 (*(int *)(*(int *)(param_1 + 0x4cc) + 0x24c) == 8)) {
                local_80 = 1;
              }
              pvVar2 = operator_new(0x22c);
              local_8 = 7;
              if (pvVar2 == (void *)0x0) {
                local_8a8 = (int *)0x0;
              }
              else {
                uVar12 = 0;
                uVar8 = 0;
                puVar5 = (undefined4 *)FUN_00430f50(0,0,0);
                uVar4 = *puVar5;
                uVar10 = puVar5[1];
                uVar13 = puVar5[2];
                puVar5 = (undefined4 *)FUN_00430f50(0,0,0);
                local_8a8 = (int *)FUN_004d5480(*(undefined4 *)(*(int *)(param_1 + 0x4cc) + 0x5c),
                                                *puVar5,puVar5[1],puVar5[2],uVar4,uVar10,uVar13,
                                                uVar8,uVar12);
              }
              local_8 = 0xffffffff;
              local_7c = local_8a8;
              if (local_8a8 != (int *)0x0) {
                if (((*(int *)(param_1 + 0x4cc) == 0) ||
                    (*(char *)(*(int *)(param_1 + 0x4cc) + 0x79c) < '\x01')) ||
                   (*(int *)(*(int *)(param_1 + 0x4cc) + 0x19c) == 0)) {
                  FUN_0058f220(local_8a8 + 0x58,*(int *)(param_1 + 0x4cc) + 0x1d2,0x20);
                }
                else {
                  local_8a8[0x17] = *(int *)(*(int *)(param_1 + 0x4cc) + 0x7a0);
                  FUN_0058f220(local_8a8 + 0x58,*(int *)(param_1 + 0x4cc) + 0x1a2,0x20);
                  local_7c[0x88] = *(int *)(*(int *)(param_1 + 0x4cc) + 0x5c);
                  FUN_0058f220(local_7c + 0x7f,*(int *)(param_1 + 0x4cc) + 0x1d2,0x20);
                }
                local_7c[0x69] = 0x3f000000;
                local_7c[0x6a] = 0x3f000000;
                local_7c[0x6b] = 0x3f000000;
                (**(code **)(*local_7c + 0x78))(local_80);
                local_7c[0x75] = 1;
                local_7c[0x65] = 3000;
                (**(code **)(*local_7c + 0x3c))
                          (*(undefined4 *)(*(int *)(param_1 + 0x4cc) + 0x28),
                           *(float *)(*(int *)(param_1 + 0x4cc) + 0x30) + _DAT_005a4214,
                           *(undefined4 *)(*(int *)(param_1 + 0x4cc) + 0x2c));
                if ((*(char *)(param_1 + 0x79c) < '\x01') || (*(int *)(param_1 + 0x19c) == 0)) {
                  FUN_004bd906(*(undefined4 *)(&DAT_00672490 + local_7c[0x17] * 0x2a0));
                }
                else {
                  if (local_7c[0x18] != 0) {
                    FUN_004bd906(*(undefined4 *)(&DAT_00672490 + local_7c[0x17] * 0x2a0));
                  }
                  if (local_7c[0x87] != 0) {
                    FUN_004bd906(*(undefined4 *)(&DAT_006724f0 + local_7c[0x88] * 0x2a0));
                  }
                }
                local_7c[0x68] = *(int *)(*(int *)(param_1 + 0x4cc) + 0x34);
                local_7c[0x66] = 1;
                local_7c[0x67] = 1;
                FUN_0054ac09(local_7c);
              }
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x16) {
            local_90 = local_38;
            local_88 = local_30;
            local_8c = local_34 + _DAT_005a3660;
            pvVar2 = operator_new(0x90);
            local_8 = 8;
            if (pvVar2 == (void *)0x0) {
              local_8ac = 0;
            }
            else {
              local_8ac = FUN_004e0990(local_90,local_8c,local_88,1);
            }
            local_8 = 0xffffffff;
            local_84 = local_8ac;
            if (local_8ac != 0) {
              FUN_0054ac09(local_8ac);
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x17) {
            local_9c = local_38;
            local_98 = local_34;
            local_94 = local_30;
            for (local_a0 = 0; (int)local_a0 < 4; local_a0 = local_a0 + 1) {
              uVar6 = local_a0 & 0x80000001;
              if ((int)uVar6 < 0) {
                uVar6 = (uVar6 - 1 | 0xfffffffe) + 1;
              }
              uVar4 = FUN_00430f50(((float)(int)(uVar6 << 1) + local_38) - _DAT_005a3660,local_34,
                                   ((float)((int)local_a0 / 2 << 1) + local_30) - _DAT_005a3660);
              FUN_004310a0(uVar4);
              pvVar2 = operator_new(0xb0);
              local_8 = 9;
              if (pvVar2 == (void *)0x0) {
                local_8b8 = 0;
              }
              else {
                local_8b8 = FUN_004e3ba0(local_9c,local_98,local_94,1,0,0);
              }
              local_8 = 0xffffffff;
              local_a4 = local_8b8;
              if (local_8b8 != 0) {
                FUN_0054ac09(local_8b8);
              }
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x18) {
            local_34 = local_34 + _DAT_005a3660;
            pvVar2 = operator_new(0xc0);
            local_8 = 10;
            if (pvVar2 == (void *)0x0) {
              local_8bc = 0;
            }
            else {
              local_8bc = FUN_004e7610(local_2c,local_28,local_24,local_38,local_34,local_30,0,0);
            }
            local_8 = 0xffffffff;
            local_a8 = local_8bc;
            if (local_8bc != 0) {
              FUN_0054ac09(local_8bc);
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x19) {
            local_b8 = local_38;
            local_b4 = local_34;
            local_b0 = local_30;
            pvVar2 = operator_new(0x90);
            local_8 = 0xb;
            if (pvVar2 == (void *)0x0) {
              local_8c0 = 0;
            }
            else {
              local_8c0 = FUN_004e0d00(local_b8,local_b4,local_b0,0);
            }
            local_8 = 0xffffffff;
            local_ac = local_8c0;
            if (local_8c0 != 0) {
              FUN_0054ac09(local_8c0);
            }
          }
          else if ((*(short *)(param_1 + 0x4d0) == 0x1b) || (*(short *)(param_1 + 0x4d0) == 0x1d)) {
            local_c4 = local_38;
            local_c0 = local_34;
            local_bc = local_30;
            pvVar2 = operator_new(0x90);
            local_8 = 0xc;
            if (pvVar2 == (void *)0x0) {
              local_8c4 = 0;
            }
            else {
              local_8c4 = FUN_004e4f20(local_c4,local_c0,local_bc,0,0);
            }
            local_8 = 0xffffffff;
            local_c8 = local_8c4;
            if (local_8c4 != 0) {
              FUN_0054ac09(local_8c4);
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x1f) {
            if (*(int *)(param_1 + 0x4cc) != 0) {
              local_e4 = local_38;
              local_e0 = local_34;
              local_dc = local_30;
              puVar5 = (undefined4 *)FUN_00430f20(local_38,local_30);
              iVar3 = FUN_0049c70f(*puVar5,puVar5[1]);
              local_e0 = (float)iVar3 * _DAT_005a4214;
              pvVar2 = operator_new(0x90);
              local_8 = 0xd;
              if (pvVar2 == (void *)0x0) {
                local_8cc = 0;
              }
              else {
                local_8cc = FUN_004e4a10(local_e4,local_e0,local_dc,4);
              }
              local_8 = 0xffffffff;
              local_d8 = local_8cc;
              if (local_8cc != 0) {
                FUN_0054ac09(local_8cc);
              }
              pvVar2 = operator_new(0x90);
              local_8 = 0xe;
              if (pvVar2 == (void *)0x0) {
                local_8d0 = 0;
              }
              else {
                local_8d0 = FUN_004e4a10(local_2c,local_28,local_24,4);
              }
              local_8 = 0xffffffff;
              local_d0 = local_8d0;
              if (local_8d0 != 0) {
                FUN_0054ac09(local_8d0);
              }
              local_d4 = 0;
              if ((*(int *)(*(int *)(param_1 + 0x4cc) + 0x24c) == 4) ||
                 (*(int *)(*(int *)(param_1 + 0x4cc) + 0x24c) == 8)) {
                local_d4 = 1;
              }
              pvVar2 = operator_new(0x22c);
              local_8 = 0xf;
              if (pvVar2 == (void *)0x0) {
                local_8d4 = (int *)0x0;
              }
              else {
                uVar12 = 0;
                uVar8 = 0;
                puVar5 = (undefined4 *)FUN_00430f50(0,0,0);
                uVar4 = *puVar5;
                uVar10 = puVar5[1];
                uVar13 = puVar5[2];
                puVar5 = (undefined4 *)FUN_00430f50(0,0,0);
                local_8d4 = (int *)FUN_004d5480(*(undefined4 *)(*(int *)(param_1 + 0x4cc) + 0x5c),
                                                *puVar5,puVar5[1],puVar5[2],uVar4,uVar10,uVar13,
                                                uVar8,uVar12);
              }
              local_8 = 0xffffffff;
              local_cc = local_8d4;
              if (local_8d4 != (int *)0x0) {
                if (((*(int *)(param_1 + 0x4cc) == 0) ||
                    (*(char *)(*(int *)(param_1 + 0x4cc) + 0x79c) < '\x01')) ||
                   (*(int *)(*(int *)(param_1 + 0x4cc) + 0x19c) == 0)) {
                  FUN_0058f220(local_8d4 + 0x58,*(int *)(param_1 + 0x4cc) + 0x1d2,0x20);
                }
                else {
                  local_8d4[0x17] = *(int *)(*(int *)(param_1 + 0x4cc) + 0x7a0);
                  FUN_0058f220(local_8d4 + 0x58,*(int *)(param_1 + 0x4cc) + 0x1a2,0x20);
                  local_cc[0x88] = *(int *)(*(int *)(param_1 + 0x4cc) + 0x5c);
                  FUN_0058f220(local_cc + 0x7f,*(int *)(param_1 + 0x4cc) + 0x1d2,0x20);
                }
                local_cc[0x69] = 0x3f000000;
                local_cc[0x6a] = 0x3f000000;
                local_cc[0x6b] = 0x3f000000;
                (**(code **)(*local_cc + 0x78))(local_d4);
                local_cc[0x75] = 1;
                local_cc[0x65] = 0x5dc;
                (**(code **)(*local_cc + 0x3c))
                          (*(undefined4 *)(*(int *)(param_1 + 0x4cc) + 0x28),
                           *(float *)(*(int *)(param_1 + 0x4cc) + 0x30) + _DAT_005a4214,
                           *(undefined4 *)(*(int *)(param_1 + 0x4cc) + 0x2c));
                if ((*(char *)(param_1 + 0x79c) < '\x01') || (*(int *)(param_1 + 0x19c) == 0)) {
                  FUN_004bd906(*(undefined4 *)(&DAT_00672490 + local_cc[0x17] * 0x2a0));
                }
                else {
                  if (local_cc[0x18] != 0) {
                    FUN_004bd906(*(undefined4 *)(&DAT_00672490 + local_cc[0x17] * 0x2a0));
                  }
                  if (local_cc[0x87] != 0) {
                    FUN_004bd906(*(undefined4 *)(&DAT_006724f0 + local_cc[0x88] * 0x2a0));
                  }
                }
                local_cc[0x68] = *(int *)(*(int *)(param_1 + 0x4cc) + 0x34);
                local_cc[0x66] = 1;
                local_cc[0x67] = 9;
                FUN_0054ac09(local_cc);
              }
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x20) {
            pvVar2 = operator_new(0xa0);
            local_8 = 0x10;
            if (pvVar2 == (void *)0x0) {
              local_8d8 = 0;
            }
            else {
              local_8d8 = FUN_004e2a30(local_38,local_34,local_30,0,*(undefined4 *)(param_1 + 0x4cc)
                                       ,0xffffffff,0x22331100);
            }
            local_8 = 0xffffffff;
            local_f4[3] = local_8d8;
            if (local_8d8 != 0) {
              FUN_0054ac09(local_8d8);
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x22) {
            local_34 = local_34 + _DAT_005a3660;
            pvVar2 = operator_new(0xb0);
            local_8 = 0x11;
            if (pvVar2 == (void *)0x0) {
              local_8dc = 0;
            }
            else {
              local_8dc = FUN_004e5d10(local_2c,local_28,local_24,local_38,local_34,local_30,0,0);
            }
            local_8 = 0xffffffff;
            local_f4[2] = local_8dc;
            if (local_8dc != 0) {
              FUN_0054ac09(local_8dc);
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x25) {
            local_f4[0] = 0;
            local_f4[1] = 0;
            local_fc = 1.0;
            if (*(char *)(param_1 + 0x79c) == '\x01') {
              local_fc = 1.3;
            }
            for (local_f8 = 0; local_f8 < 2; local_f8 = local_f8 + 1) {
              iVar3 = _rand();
              local_104 = (float)(iVar3 % 5);
              if (local_f4[local_f8] == 0) {
                pvVar2 = operator_new(0x180);
                local_8 = 0x12;
                if (pvVar2 == (void *)0x0) {
                  local_8e4 = 0;
                }
                else {
                  local_8e4 = FUN_004c8f70(0x6d,0,(_DAT_005a4158 * local_104 + _DAT_005a3660) *
                                                  local_fc - (float)local_f8 * _DAT_005a4154,
                                           (_DAT_005a4158 * local_104 + _DAT_005a3660) * local_fc -
                                           (float)local_f8 * _DAT_005a4154,
                                           (_DAT_005a4158 * local_104 + _DAT_005a3660) * local_fc -
                                           (float)local_f8 * _DAT_005a4154,0,8,0x50);
                }
                local_8 = 0xffffffff;
                local_f4[local_f8] = local_8e4;
                (**(code **)(*(int *)local_f4[local_f8] + 0x3c))(0xffffdd00);
                *(undefined4 *)(local_f4[local_f8] + 0x6c) = 1;
                *(undefined4 *)(local_f4[local_f8] + 0x104) = 0;
                *(undefined4 *)(local_f4[local_f8] + 0xf8) = 2000;
                FUN_0054ac09(local_f4[local_f8]);
              }
              if (local_f4[local_f8] != 0) {
                *(float *)(local_f4[local_f8] + 0x114) =
                     ((float)((param_2 - *(int *)(local_f4[local_f8] + 0xfc)) % 1000) *
                     _DAT_005a4290) / _DAT_005a43c0 + (float)local_f8;
                uVar4 = FUN_00430f50(*(float *)(param_1 + 0x28) + _DAT_005a34a0,
                                     *(float *)(param_1 + 0x30) + _DAT_005a4418,
                                     *(float *)(param_1 + 0x2c) + _DAT_005a34a0);
                FUN_004310a0(uVar4);
              }
              pvVar2 = operator_new(0x180);
              local_8 = 0x13;
              if (pvVar2 == (void *)0x0) {
                local_8f0 = (int *)0x0;
              }
              else {
                local_8f0 = (int *)FUN_004c8f70(0x6d,500,(_DAT_005a4158 * local_104 + _DAT_005a3660)
                                                         * local_fc,
                                                (_DAT_005a4158 * local_104 + _DAT_005a3660) *
                                                local_fc,(_DAT_005a4158 * local_104 + _DAT_005a3660)
                                                         * local_fc,0,8,0x50);
              }
              local_8 = 0xffffffff;
              local_100 = local_8f0;
              if (local_8f0 != (int *)0x0) {
                (**(code **)(*local_8f0 + 0x3c))(0xffffdd00);
                local_100[0x1b] = 1;
                local_100[0x41] = 1;
                local_100[0x3e] = 2000;
                local_100[0x45] =
                     (int)(((float)((param_2 - local_100[0x3f]) % 1000) * _DAT_005a4290) /
                          _DAT_005a43c0);
                uVar4 = FUN_00430f50(*(float *)(param_1 + 0x28) + _DAT_005a34a0,
                                     *(float *)(param_1 + 0x30) + _DAT_005a4418,
                                     *(float *)(param_1 + 0x2c) + _DAT_005a34a0);
                FUN_004310a0(uVar4);
                local_100[7] = param_1;
                FUN_0054ac09(local_100);
              }
            }
            if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x69), iVar3 != 0)) {
              uVar10 = 0;
              uVar4 = 0;
              FUN_00429a6d(0x69);
              FUN_0042ad2b(uVar4,uVar10);
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x26) {
            local_34 = local_34 + _DAT_005a3660;
            local_28 = local_28 + _DAT_005a3660;
            local_108 = 1;
            if (*(int *)(param_1 + 0x24c) == 0x26) {
              local_108 = 2;
            }
            pvVar2 = operator_new(0xc0);
            local_8 = 0x14;
            if (pvVar2 == (void *)0x0) {
              local_8fc = 0;
            }
            else {
              local_8fc = FUN_004e7610(local_2c,local_28,local_24,local_38,local_34,local_30,
                                       local_108,0);
            }
            local_8 = 0xffffffff;
            local_10c = local_8fc;
            if (local_8fc != 0) {
              FUN_0054ac09(local_8fc);
            }
          }
          else if ((*(short *)(param_1 + 0x4d0) == 0x28) || (*(short *)(param_1 + 0x4d0) == 0x15)) {
            local_110 = 0xff33ff66;
            if (*(short *)(param_1 + 0x4d2) == 1) {
              local_110 = 0xff66ffaa;
            }
            if (*(short *)(param_1 + 0x4d2) == 2) {
              local_110 = 0xff113388;
            }
            if (*(short *)(param_1 + 0x4d2) == 3) {
              local_110 = 0xffff8800;
            }
            pvVar2 = operator_new(0x90);
            local_8 = 0x15;
            if (pvVar2 == (void *)0x0) {
              local_900 = 0;
            }
            else {
              local_900 = FUN_004ec9a0(local_38,local_34,local_30,local_110,10,1,0);
            }
            local_8 = 0xffffffff;
            local_114 = local_900;
            if (local_900 != 0) {
              FUN_0054ac09(local_900);
            }
          }
          else if ((((*(short *)(param_1 + 0x4d0) == 0x29) || (*(short *)(param_1 + 0x4d0) == 0x2b))
                   || (*(short *)(param_1 + 0x4d0) == 0x5a)) ||
                  (*(short *)(param_1 + 0x4d0) == 0x36)) {
            local_128 = local_38;
            local_124 = local_34;
            local_120 = local_30;
            puVar5 = (undefined4 *)FUN_00430f20(local_38,local_30);
            iVar3 = FUN_0049c70f(*puVar5,puVar5[1]);
            local_124 = (float)iVar3 * _DAT_005a4214;
            local_118 = (uint)(*(short *)(param_1 + 0x4d0) == 0x2b);
            if (*(short *)(param_1 + 0x4d0) == 0x5a) {
              local_118 = 2;
            }
            if (*(short *)(param_1 + 0x4d0) == 0x36) {
              local_118 = 3;
            }
            pvVar2 = operator_new(0x90);
            local_8 = 0x16;
            if (pvVar2 == (void *)0x0) {
              local_908 = 0;
            }
            else {
              local_908 = FUN_004e4a10(local_128,local_124,local_120,local_118);
            }
            local_8 = 0xffffffff;
            local_11c = local_908;
            if (local_908 != 0) {
              FUN_0054ac09(local_908);
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x2a) {
            local_134 = local_2c;
            local_12c = local_24;
            local_130 = local_28 - _DAT_005a3660;
            pvVar2 = operator_new(0xac);
            local_8 = 0x17;
            if (pvVar2 == (void *)0x0) {
              local_90c = 0;
            }
            else {
              local_90c = FUN_004ee730(local_134,local_130,local_12c,0);
            }
            local_8 = 0xffffffff;
            local_144[3] = local_90c;
            if (local_90c != 0) {
              FUN_0054ac09(local_90c);
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x2c) {
            local_144[0] = -0xaaff78;
            local_144[1] = 0xff555500;
            local_144[2] = 0xff005500;
            for (local_148 = 0; local_148 < 3; local_148 = local_148 + 1) {
              iVar3 = _rand();
              local_154 = (float)(iVar3 % 5) + _DAT_005a4104;
              pvVar2 = operator_new(0x180);
              local_8 = 0x18;
              if (pvVar2 == (void *)0x0) {
                local_914 = (int *)0x0;
              }
              else {
                local_914 = (int *)FUN_004c8f70(0x38,800,_DAT_005a4154 -
                                                         (float)local_148 * _DAT_005a45d4,
                                                _DAT_005a4154 - (float)local_148 * _DAT_005a45d4,
                                                _DAT_005a4154 - (float)local_148 * _DAT_005a45d4,
                                                0x3b03126f,1,0x50);
              }
              local_8 = 0xffffffff;
              local_14c = local_914;
              if (local_914 != (int *)0x0) {
                (**(code **)(*local_914 + 0x3c))(local_144[local_148]);
                local_14c[0x1b] = 1;
                local_14c[0x41] = 1;
                uVar4 = FUN_00430f50(local_2c,local_28 + _DAT_005a3f2c,local_24);
                FUN_004310a0(uVar4);
                FUN_0054ac09(local_14c);
              }
              pvVar2 = operator_new(0x180);
              local_8 = 0x19;
              if (pvVar2 == (void *)0x0) {
                local_918 = (int *)0x0;
              }
              else {
                local_918 = (int *)FUN_004c8f70(0x3c,500,local_154 * _DAT_005a45d4 + _DAT_005a4154,
                                                local_154 * _DAT_005a45d4 + _DAT_005a4154,
                                                local_154 * _DAT_005a45d4 + _DAT_005a4154,0x3a03126f
                                                ,1,0x50);
              }
              local_8 = 0xffffffff;
              local_150 = local_918;
              if (local_918 != (int *)0x0) {
                (**(code **)(*local_918 + 0x3c))(0xff333355);
                local_150[0x45] = (int)((_DAT_005a4148 * local_154) / _DAT_005a40f0);
                local_150[0x1b] = 1;
                local_150[0x41] = 1;
                uVar4 = FUN_00430f50(local_2c,local_28 + _DAT_005a3f2c,local_24);
                FUN_004310a0(uVar4);
                FUN_0054ac09(local_150);
              }
            }
            if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x9f), iVar3 != 0)) {
              uVar10 = 0;
              uVar4 = 0;
              FUN_00429a6d(0x9f);
              FUN_0042ad2b(uVar4,uVar10);
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x2e) {
            pvVar2 = operator_new(0x90);
            local_8 = 0x1a;
            if (pvVar2 == (void *)0x0) {
              local_91c = 0;
            }
            else {
              uVar4 = 1000;
              puVar5 = (undefined4 *)FUN_00430f50(0,0,0);
              local_91c = FUN_004d4de0(local_2c,local_28,local_24,3,0xf,0x3f800000,0xffff3300,0,0x7a
                                       ,0x3f800000,1,*puVar5,puVar5[1],puVar5[2],uVar4);
            }
            local_8 = 0xffffffff;
            local_168 = local_91c;
            if (local_91c != 0) {
              FUN_0054ac09(local_91c);
            }
            pvVar2 = operator_new(0x90);
            local_8 = 0x1b;
            if (pvVar2 == (void *)0x0) {
              local_920 = 0;
            }
            else {
              uVar4 = 1000;
              puVar5 = (undefined4 *)FUN_00430f50(0,0,0);
              local_920 = FUN_004d4de0(local_2c,local_28,local_24,1,0xf,0x3e99999a,0xffff3300,0,0x38
                                       ,0x3f800000,1,*puVar5,puVar5[1],puVar5[2],uVar4);
            }
            local_8 = 0xffffffff;
            local_160 = local_920;
            if (local_920 != 0) {
              FUN_0054ac09(local_920);
            }
            local_28 = local_28 + _DAT_005a40f0;
            pvVar2 = operator_new(0x90);
            local_8 = 0x1c;
            if (pvVar2 == (void *)0x0) {
              local_924 = 0;
            }
            else {
              uVar4 = 1000;
              puVar5 = (undefined4 *)FUN_00430f50(0,0,0);
              local_924 = FUN_004d4de0(local_2c,local_28,local_24,2,0xf,0x3f800000,0xff0033ff,0,0x7a
                                       ,0x3f800000,1,*puVar5,puVar5[1],puVar5[2],uVar4);
            }
            local_8 = 0xffffffff;
            local_164 = local_924;
            if (local_924 != 0) {
              FUN_0054ac09(local_924);
            }
            pvVar2 = operator_new(0x90);
            local_8 = 0x1d;
            if (pvVar2 == (void *)0x0) {
              local_928 = 0;
            }
            else {
              uVar4 = 1000;
              puVar5 = (undefined4 *)FUN_00430f50(0,0,0);
              local_928 = FUN_004d4de0(local_2c,local_28,local_24,2,0xf,0x3e99999a,0xff0033ff,0,0x38
                                       ,0x3f800000,1,*puVar5,puVar5[1],puVar5[2],uVar4);
            }
            local_8 = 0xffffffff;
            local_158 = local_928;
            if (local_928 != 0) {
              FUN_0054ac09(local_928);
            }
            pvVar2 = operator_new(0xc0);
            local_8 = 0x1e;
            if (pvVar2 == (void *)0x0) {
              local_92c = (int *)0x0;
            }
            else {
              local_92c = (int *)FUN_004df250(2,7,0x3f800000);
            }
            local_8 = 0xffffffff;
            local_15c = local_92c;
            if (local_92c != (int *)0x0) {
              local_92c[0x1b] = 1;
              (**(code **)(*local_92c + 0x44))(0xffaa3300);
              puVar5 = (undefined4 *)FUN_00430f20(local_2c,local_24);
              (**(code **)(*local_15c + 0x3c))(*puVar5,puVar5[1]);
              local_15c[0x28] = 0x5dc;
              FUN_0054ac09(local_15c);
            }
            if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x24), iVar3 != 0)) {
              uVar10 = 0;
              uVar4 = 0;
              FUN_00429a6d(0x24);
              FUN_0042ad2b(uVar4,uVar10);
            }
          }
          else if ((*(short *)(param_1 + 0x4d0) == 0x30) || (*(short *)(param_1 + 0x4d0) == 0x31)) {
            local_28 = local_28 - _DAT_005a3660;
            local_170 = *(short *)(param_1 + 0x4d0) + -0x30;
            pvVar2 = operator_new(0x22c);
            local_8 = 0x1f;
            if (pvVar2 == (void *)0x0) {
              local_930 = (int *)0x0;
            }
            else {
              local_930 = (int *)FUN_004d5480(0x14,local_2c,local_28,local_24,local_38,local_34,
                                              local_30,local_170,*(undefined4 *)(param_1 + 0x4cc));
            }
            local_8 = 0xffffffff;
            local_16c = local_930;
            if (local_930 != (int *)0x0) {
              if (*(short *)(param_1 + 0x4d0) == 0x30) {
                *(undefined2 *)(local_930 + 0x59) = 4;
                *(undefined2 *)(local_930 + 0x58) = 4;
                *(undefined2 *)((int)local_930 + 0x166) = 1;
                *(undefined2 *)((int)local_930 + 0x162) = 1;
                (**(code **)(*local_930 + 0x78))(0);
                *(undefined4 *)(local_16c[0x18] + 0x24) = 0x3ecccccd;
                *(undefined4 *)(local_16c[0x18] + 0x28) = 0x3ecccccd;
                *(undefined4 *)(local_16c[0x18] + 0x2c) = 0x3ecccccd;
                local_16c[0x66] = 1;
              }
              else if (*(short *)(param_1 + 0x4d0) == 0x31) {
                *(undefined2 *)(local_930 + 0x59) = 2;
                *(undefined2 *)(local_930 + 0x58) = 2;
                *(undefined2 *)((int)local_930 + 0x166) = 0;
                *(undefined2 *)((int)local_930 + 0x162) = 0;
                (**(code **)(*local_930 + 0x78))(0);
                *(undefined4 *)(local_16c[0x18] + 0x24) = 0x3e4ccccd;
                *(undefined4 *)(local_16c[0x18] + 0x28) = 0x3e4ccccd;
                *(undefined4 *)(local_16c[0x18] + 0x2c) = 0x3e4ccccd;
                local_16c[0x66] = 0;
              }
              local_16c[0x69] = 0x3f800000;
              local_16c[0x6a] = 0x3f800000;
              local_16c[0x6b] = 0x3f800000;
              local_16c[0x75] = 1;
              local_16c[0x68] = *(int *)(param_1 + 0x34);
              local_16c[0x67] = 2;
              FUN_0054ac09(local_16c);
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x32) {
            local_2c = local_2c - _DAT_005a34a0;
            local_28 = local_28 - _DAT_005a3660;
            local_24 = local_24 + _DAT_005a34a0;
            local_174 = *(short *)(param_1 + 0x4d0) + -0x30;
            for (local_178 = 0; local_178 < 3; local_178 = local_178 + 1) {
              local_2c = (float)local_178 * _DAT_005a414c + local_2c;
              local_24 = local_24 - (float)local_178 * _DAT_005a414c;
              pvVar2 = operator_new(0x22c);
              local_8 = 0x20;
              if (pvVar2 == (void *)0x0) {
                local_934 = (int *)0x0;
              }
              else {
                local_934 = (int *)FUN_004d5480(0x20,local_2c,local_28,local_24,local_38,local_34,
                                                local_30,local_174,*(undefined4 *)(param_1 + 0x4cc))
                ;
              }
              local_8 = 0xffffffff;
              local_17c = local_934;
              if (local_934 != (int *)0x0) {
                *(undefined2 *)(local_934 + 0x59) = 0;
                *(undefined2 *)(local_934 + 0x58) = 0;
                *(undefined2 *)((int)local_934 + 0x166) = 0;
                *(undefined2 *)((int)local_934 + 0x162) = 0;
                (**(code **)(*local_934 + 0x78))(0);
                *(undefined4 *)(local_17c[0x18] + 0x24) = 0x40000000;
                *(undefined4 *)(local_17c[0x18] + 0x28) = 0x40000000;
                *(undefined4 *)(local_17c[0x18] + 0x2c) = 0x40000000;
                local_17c[0x66] = 1;
                local_17c[0x69] = 0x3f800000;
                local_17c[0x6a] = 0x3f800000;
                local_17c[0x6b] = 0x3f800000;
                local_17c[0x75] = 1;
                local_17c[0x68] = *(int *)(param_1 + 0x34);
                local_17c[0x67] = 4;
                FUN_0054ac09(local_17c);
              }
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x33) {
            FUN_00430f50(*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x30),
                         *(undefined4 *)(param_1 + 0x2c));
            pvVar2 = operator_new(0xa8);
            local_8 = 0x21;
            if (pvVar2 == (void *)0x0) {
              local_938 = 0;
            }
            else {
              local_938 = FUN_004ecd90(local_18c,local_188,local_184,local_18c,local_188,local_184,1
                                       ,*(undefined4 *)(param_1 + 0x4cc));
            }
            local_8 = 0xffffffff;
            local_180 = local_938;
            if (local_938 != 0) {
              FUN_0054ac09(local_938);
            }
            if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x9d), iVar3 != 0)) {
              uVar10 = 0;
              uVar4 = 0;
              FUN_00429a6d(0x9d);
              FUN_0042ad2b(uVar4,uVar10);
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x35) {
            for (local_190 = 0; local_190 < 5; local_190 = local_190 + 1) {
              local_194 = (int *)0x0;
              iVar3 = _rand();
              local_1ac[4] = iVar3 % 10 + 10;
              local_198 = (float)local_190 * _DAT_005a40ec;
              pvVar2 = operator_new(0x180);
              local_8 = 0x22;
              if (pvVar2 == (void *)0x0) {
                local_93c = (int *)0x0;
              }
              else {
                local_93c = (int *)FUN_004c8f70(0,local_190 * 200 + 0x4b0,
                                                (float)local_1ac[4] * _DAT_005a4214 + _DAT_005a414c
                                                + local_198,
                                                (float)local_1ac[4] * _DAT_005a40ec + _DAT_005a414c
                                                + local_198,
                                                (float)local_1ac[4] * _DAT_005a4214 + _DAT_005a414c
                                                + local_198,0x38d1b717,1,0x50);
              }
              local_8 = 0xffffffff;
              local_194 = local_93c;
              if (local_93c != (int *)0x0) {
                iVar3 = _rand();
                fVar11 = (float)(iVar3 % 10 + -5) * _DAT_005a41e8 + *(float *)(param_1 + 0x2c);
                uVar4 = *(undefined4 *)(param_1 + 0x30);
                iVar3 = _rand();
                uVar4 = FUN_00430f50((float)(iVar3 % 10 + -5) * _DAT_005a41e8 +
                                     *(float *)(param_1 + 0x28),uVar4,fVar11);
                uVar4 = FUN_004310a0(uVar4);
                FUN_004310a0(uVar4);
                local_194[0x1b] = 1;
                local_194[0x40] = 0;
                local_194[0x52] = 1;
                (**(code **)(*local_194 + 0x3c))(0xffffffff);
                FUN_0054ac09(local_194);
              }
            }
          }
          else if ((*(short *)(param_1 + 0x4d0) == 0x48) || (*(short *)(param_1 + 0x4d0) == 0x50)) {
            if (*(int *)(param_1 + 0x4cc) != 0) {
              local_1b0 = (uint)(*(short *)(param_1 + 0x4d0) == 0x50);
              local_1ac[0] = -1;
              local_1ac[1] = 0xffffaa55;
              local_1ac[2] = 0xffffffff;
              local_1ac[3] = 0xffff0000;
              iVar3 = (**(code **)(*DAT_013b71e8 + 0x78))();
              if (param_1 == *(int *)(iVar3 + 0x54)) {
                uVar4 = 1;
                (**(code **)(*DAT_013b71e8 + 0x78))(1);
                FUN_004efb89(uVar4);
              }
              pvVar2 = operator_new(0xc0);
              local_8 = 0x23;
              if (pvVar2 == (void *)0x0) {
                local_948 = (int *)0x0;
              }
              else {
                local_948 = (int *)FUN_004df250(local_1b0 + 2,7,0x3f800000);
              }
              local_8 = 0xffffffff;
              local_1b4 = local_948;
              if (local_948 != (int *)0x0) {
                (**(code **)(*local_948 + 0x44))(local_1ac[local_1b0 * 2 + 1]);
                local_1b4[0x1b] = 1;
                puVar5 = (undefined4 *)
                         FUN_00430f20(*(undefined4 *)(*(int *)(param_1 + 0x4cc) + 0x28),
                                      *(undefined4 *)(*(int *)(param_1 + 0x4cc) + 0x2c));
                (**(code **)(*local_1b4 + 0x3c))(*puVar5,puVar5[1]);
                local_1b4[0x28] = 800;
                FUN_0054ac09(local_1b4);
              }
              for (local_1b8 = 0; local_1b8 < 2; local_1b8 = local_1b8 + 1) {
                pvVar2 = operator_new(0x180);
                local_8 = 0x24;
                if (pvVar2 == (void *)0x0) {
                  local_94c = (int *)0x0;
                }
                else {
                  local_94c = (int *)FUN_004c8f70(local_1b0 * 4 + 0x38,700,
                                                  (float)local_1b8 * _DAT_005a4158 + _DAT_005a3660,
                                                  (float)local_1b8 * _DAT_005a4158 + _DAT_005a3660,
                                                  (float)local_1b8 * _DAT_005a4158 + _DAT_005a3660,
                                                  (float)(int)local_1b0 * _DAT_005a43e8 +
                                                  _DAT_005a41ec,1,0x50);
                }
                local_8 = 0xffffffff;
                local_1bc = local_94c;
                if (local_94c != (int *)0x0) {
                  (**(code **)(*local_94c + 0x3c))(local_1ac[local_1b0 * 2 + local_1b8]);
                  local_1bc[0x1b] = 1;
                  local_1bc[0x41] = 1;
                  uVar4 = FUN_00430f50(*(undefined4 *)(*(int *)(param_1 + 0x4cc) + 0x28),
                                       *(float *)(*(int *)(param_1 + 0x4cc) + 0x30) + _DAT_005a3660,
                                       *(undefined4 *)(*(int *)(param_1 + 0x4cc) + 0x2c));
                  FUN_004310a0(uVar4);
                  FUN_0054ac09(local_1bc);
                }
              }
              if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(local_1b0 * 0xb + 0xa0), iVar3 != 0))
              {
                uVar10 = 0;
                uVar4 = 0;
                FUN_00429a6d(local_1b0 * 0xb + 0xa0);
                FUN_0042ad2b(uVar4,uVar10);
              }
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x4a) {
            for (local_1c0 = 0; local_1c0 < 5; local_1c0 = local_1c0 + 1) {
              local_1c4 = (int *)0x0;
              pvVar2 = operator_new(0x180);
              local_8 = 0x25;
              if (pvVar2 == (void *)0x0) {
                local_950 = (int *)0x0;
              }
              else {
                local_950 = (int *)FUN_004c8f70(0x38,local_1c0 * 200 + 500,
                                                (float)local_1c0 * _DAT_005a4160 + _DAT_005a3660,
                                                (float)local_1c0 * _DAT_005a34a0 + _DAT_005a3660,
                                                (float)local_1c0 * _DAT_005a4160 + _DAT_005a3660,0,1
                                                ,0x50);
              }
              local_8 = 0xffffffff;
              local_1c4 = local_950;
              if (local_950 != (int *)0x0) {
                (**(code **)(*local_950 + 0x3c))(0xff55aaff);
                local_1c4[0x1b] = 1;
                local_1c4[0x41] = 1;
                uVar4 = FUN_00430f50(local_2c,((float)local_1c0 * _DAT_005a40ec + local_28) -
                                              _DAT_005a34a0,local_24);
                FUN_004310a0(uVar4);
                FUN_0054ac09(local_1c4);
              }
            }
            if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0xae), iVar3 != 0)) {
              uVar10 = 0;
              uVar4 = 0;
              FUN_00429a6d(0xae);
              FUN_0042ad2b(uVar4,uVar10);
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x4c) {
            local_1ec[0] = -0x664401;
            local_1ec[1] = 0xff00ffaa;
            local_1ec[2] = 0xffffaa00;
            local_1ec[3] = 0xff880088;
            local_1ec[4] = 0xffcc8888;
            for (local_1f0 = 0; local_1f0 < 5; local_1f0 = local_1f0 + 1) {
              local_1ec[local_1f0 + 5] = 0;
              if (local_1ec[local_1f0 + 5] == 0) {
                pvVar2 = operator_new(0xcc);
                local_8 = 0x26;
                if (pvVar2 == (void *)0x0) {
                  local_954 = 0;
                }
                else {
                  local_954 = FUN_004d2230(0xc,local_1ec[local_1f0],0,1);
                }
                local_8 = 0xffffffff;
                local_1ec[local_1f0 + 5] = local_954;
                if (local_1ec[local_1f0 + 5] != 0) {
                  *(undefined4 *)(local_1ec[local_1f0 + 5] + 0xa4) = 0;
                  FUN_004310a0(param_1 + 0x50);
                  *(float *)(local_1ec[local_1f0 + 5] + 0x78) =
                       *(float *)(local_1ec[local_1f0 + 5] + 0x78) + _DAT_005a4428;
                  *(int *)(local_1ec[local_1f0 + 5] + 0xb0) = local_1f0 * 0x32 + 0x514;
                  *(undefined4 *)(local_1ec[local_1f0 + 5] + 0x98) = 0x3f800000;
                  *(undefined4 *)(local_1ec[local_1f0 + 5] + 0x9c) = 0x3f800000;
                  FUN_0054ac09(local_1ec[local_1f0 + 5]);
                }
              }
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x4d) {
            for (local_1f4 = 0; local_1f4 < 5; local_1f4 = local_1f4 + 1) {
              iVar3 = _rand();
              local_200 = (float)(iVar3 % 5);
              pvVar2 = operator_new(0x180);
              local_8 = 0x27;
              if (pvVar2 == (void *)0x0) {
                local_95c = (int *)0x0;
              }
              else {
                local_95c = (int *)FUN_004c8f70(0x65,local_1f4 * 100 + 500,
                                                _DAT_005a4be4 - (float)local_1f4 * _DAT_005a45d4,
                                                _DAT_005a4158 - (float)local_1f4 * _DAT_005a45d4,
                                                _DAT_005a4158 - (float)local_1f4 * _DAT_005a45d4,
                                                0x3b03126f,8,0x50);
              }
              local_8 = 0xffffffff;
              local_1f8 = local_95c;
              if (local_95c != (int *)0x0) {
                (**(code **)(*local_95c + 0x3c))(0xff5500ff);
                local_1f8[0x1b] = 1;
                local_1f8[0x41] = 1;
                uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                                     (float)local_1f4 * _DAT_005a4158 +
                                     *(float *)(param_1 + 0x30) + _DAT_005a4158,
                                     *(undefined4 *)(param_1 + 0x2c));
                FUN_004310a0(uVar4);
                FUN_0054ac09(local_1f8);
              }
              pvVar2 = operator_new(0x180);
              local_8 = 0x28;
              if (pvVar2 == (void *)0x0) {
                local_960 = (int *)0x0;
              }
              else {
                local_960 = (int *)FUN_004c8f70(0x65,local_1f4 * 100 + 500,
                                                local_200 * _DAT_005a45d4 + _DAT_005a4214,
                                                local_200 * _DAT_005a45d4 + _DAT_005a4214,
                                                local_200 * _DAT_005a45d4 + _DAT_005a4214,0x3a03126f
                                                ,8,0x50);
              }
              local_8 = 0xffffffff;
              local_1fc = local_960;
              if (local_960 != (int *)0x0) {
                (**(code **)(*local_960 + 0x3c))(0xffffffff);
                local_1fc[0x45] = (int)((_DAT_005a4148 * local_200) / _DAT_005a40f0);
                local_1fc[0x1b] = 1;
                local_1fc[0x41] = 1;
                uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                                     (float)local_1f4 * _DAT_005a4158 +
                                     *(float *)(param_1 + 0x30) + _DAT_005a4158,
                                     *(undefined4 *)(param_1 + 0x2c));
                FUN_004310a0(uVar4);
                FUN_0054ac09(local_1fc);
              }
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x4f) {
            for (local_204 = 0; local_204 < 6; local_204 = local_204 + 1) {
              local_208 = (int *)0x0;
              pvVar2 = operator_new(0x180);
              local_8 = 0x29;
              if (pvVar2 == (void *)0x0) {
                local_964 = (int *)0x0;
              }
              else {
                local_964 = (int *)FUN_004c8f70(0x38,local_204 * 0x32 + 800,0x40000000,
                                                (float)local_204 * _DAT_005a4158 + _DAT_005a3660,
                                                0x40000000,0,1,0x50);
              }
              local_8 = 0xffffffff;
              local_208 = local_964;
              if (local_964 != (int *)0x0) {
                (**(code **)(*local_964 + 0x3c))(0xffffffff);
                local_208[0x1b] = 1;
                local_208[0x41] = 1;
                uVar4 = FUN_00430f50(local_2c,local_28 + _DAT_005a4158,local_24);
                FUN_004310a0(uVar4);
                FUN_0054ac09(local_208);
              }
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x56) {
            pvVar2 = operator_new(0x90);
            local_8 = 0x2a;
            if (pvVar2 == (void *)0x0) {
              local_968 = 0;
            }
            else {
              uVar4 = 1;
              puVar5 = (undefined4 *)
                       FUN_00430f50(*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x30),
                                    *(undefined4 *)(param_1 + 0x2c));
              local_968 = FUN_004d19a0(*puVar5,puVar5[1],puVar5[2],uVar4);
            }
            local_8 = 0xffffffff;
            local_20c = local_968;
            if (((local_968 != 0) && (FUN_0054ac09(local_968), DAT_005ccf98 != 0)) &&
               (iVar3 = FUN_00429a6d(0x25), iVar3 != 0)) {
              uVar10 = 0;
              uVar4 = 0;
              FUN_00429a6d(0x25);
              FUN_0042ad2b(uVar4,uVar10);
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x57) {
            pvVar2 = operator_new(0x9c);
            local_8 = 0x2b;
            if (pvVar2 == (void *)0x0) {
              local_96c = 0;
            }
            else {
              uVar4 = 0;
              iVar3 = param_1;
              puVar5 = (undefined4 *)
                       FUN_00430f50(*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x30),
                                    *(undefined4 *)(param_1 + 0x2c));
              local_96c = FUN_004ed2a0(*puVar5,puVar5[1],puVar5[2],uVar4,iVar3);
            }
            local_8 = 0xffffffff;
            local_210 = local_96c;
            if (local_96c != 0) {
              FUN_0054ac09(local_96c);
            }
          }
          else if ((*(short *)(param_1 + 0x4d0) == 0x58) && (*(int *)(param_1 + 0x60) != 0)) {
            if (*(int *)(param_1 + 0x4cc) != 0) {
              FUN_00430f50(*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x30),
                           *(undefined4 *)(param_1 + 0x2c));
              FUN_00430f50(*(undefined4 *)(*(int *)(param_1 + 0x4cc) + 0x28),
                           *(undefined4 *)(*(int *)(param_1 + 0x4cc) + 0x30),
                           *(undefined4 *)(*(int *)(param_1 + 0x4cc) + 0x2c));
              for (local_220 = 0; local_220 < 5; local_220 = local_220 + 1) {
                local_234 = 0;
                if ((*(int *)(param_1 + 0x24c) == 4) || (*(int *)(param_1 + 0x24c) == 8)) {
                  local_234 = 1;
                }
                pvVar2 = operator_new(0x22c);
                local_8 = 0x2c;
                if (pvVar2 == (void *)0x0) {
                  local_970 = (int *)0x0;
                }
                else {
                  local_970 = (int *)FUN_004d5480(*(undefined4 *)(param_1 + 0x5c),local_21c,
                                                  local_218,local_214,local_22c,local_228,local_224,
                                                  0,0);
                }
                local_8 = 0xffffffff;
                local_230 = local_970;
                if (local_970 != (int *)0x0) {
                  local_970[0xe] = *(int *)(param_1 + 0x38);
                  if ((*(char *)(param_1 + 0x79c) < '\x01') || (*(int *)(param_1 + 0x19c) == 0)) {
                    FUN_0058f220(local_970 + 0x58,param_1 + 0x1d2,0x20);
                  }
                  else {
                    local_234 = 0;
                    FUN_0058f220(local_970 + 0x58,param_1 + 0x1a2,0x20);
                    local_230[0x17] = *(int *)(param_1 + 0x7a0);
                  }
                  local_230[0x75] = 1;
                  local_230[0x69] = 0x3f000000;
                  local_230[0x6a] = 0x3e99999a;
                  local_230[0x6b] = 0x3e4ccccd;
                  (**(code **)(*local_230 + 0x78))(local_234);
                  local_230[100] = param_2;
                  local_230[0x65] = local_220 * 200 + 100;
                  (**(code **)(*local_230 + 0x3c))
                            (*(undefined4 *)(param_1 + 0x28),
                             *(float *)(param_1 + 0x30) + _DAT_005a4214,
                             *(undefined4 *)(param_1 + 0x2c));
                  local_230[0x68] = *(int *)(param_1 + 0x34);
                  local_230[0x7d] = local_230[0x68];
                  local_230[0x66] = 1;
                  FUN_004bd906((int)*(short *)(&DAT_005d0508 +
                                              *(int *)(&DAT_00672490 +
                                                      *(int *)(param_1 + 0x740) * 4 +
                                                      *(int *)(param_1 + 0x5c) * 0x2a0) * 2 +
                                              *(int *)(param_1 + 0x25c) * 0x70 +
                                              *(int *)(param_1 + 0x5c) * 0x1a40));
                  *(undefined4 *)(local_230[0x18] + 0x2e4) =
                       *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2e4);
                  local_230[0x67] = 6;
                  FUN_0054ac09(local_230);
                }
              }
              if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0xa0), iVar3 != 0)) {
                uVar10 = 0;
                uVar4 = 0;
                FUN_00429a6d(0xa0);
                FUN_0042ad2b(uVar4,uVar10);
              }
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x59) {
            for (local_238 = 0; local_238 < 5; local_238 = local_238 + 1) {
              local_240 = 0;
              if ((*(int *)(param_1 + 0x24c) == 4) || (*(int *)(param_1 + 0x24c) == 8)) {
                local_240 = 1;
              }
              pvVar2 = operator_new(0x22c);
              local_8 = 0x2d;
              if (pvVar2 == (void *)0x0) {
                local_974 = (int *)0x0;
              }
              else {
                uVar12 = 0;
                uVar8 = 0;
                puVar5 = (undefined4 *)FUN_00430f50(0,0,0);
                uVar4 = *puVar5;
                uVar10 = puVar5[1];
                uVar13 = puVar5[2];
                puVar5 = (undefined4 *)FUN_00430f50(0,0,0);
                local_974 = (int *)FUN_004d5480(*(undefined4 *)(param_1 + 0x5c),*puVar5,puVar5[1],
                                                puVar5[2],uVar4,uVar10,uVar13,uVar8,uVar12);
              }
              local_8 = 0xffffffff;
              local_23c = local_974;
              if (local_974 != (int *)0x0) {
                if ((*(char *)(param_1 + 0x79c) < '\x01') || (*(int *)(param_1 + 0x19c) == 0)) {
                  FUN_0058f220(local_974 + 0x58,param_1 + 0x1d2,0x20);
                  local_23c[0xe] = *(int *)(param_1 + 0x38);
                  (**(code **)(*local_23c + 0x3c))
                            (*(undefined4 *)(param_1 + 0x28),
                             *(float *)(param_1 + 0x30) + _DAT_005a4214,
                             *(undefined4 *)(param_1 + 0x2c));
                }
                else {
                  local_240 = 0;
                  local_974[0x17] = *(int *)(param_1 + 0x7a0);
                  FUN_0058f220(local_974 + 0x58,param_1 + 0x1a2,0x20);
                  local_23c[0xe] = *(int *)(param_1 + 0x38);
                  (**(code **)(*local_23c + 0x3c))
                            (*(undefined4 *)(*(int *)(param_1 + 0x19c) + 0xc),
                             *(float *)(param_1 + 0x30) + _DAT_005a4214,
                             *(undefined4 *)(*(int *)(param_1 + 0x19c) + 0x14));
                }
                local_23c[0x75] = 1;
                local_23c[0x69] = 0x3e99999a;
                local_23c[0x6a] = 0x3e99999a;
                local_23c[0x6b] = 0x3e99999a;
                (**(code **)(*local_23c + 0x78))(local_240);
                local_23c[100] = param_2 + local_238 * 100;
                local_23c[0x65] = local_238 * 0x32 + 400;
                local_23c[0x68] = *(int *)(param_1 + 0x34);
                local_23c[0x7d] = local_23c[0x68];
                local_23c[0x66] = 1;
                FUN_004bd906((int)*(short *)(&DAT_005d0508 +
                                            *(int *)(&DAT_00672490 +
                                                    *(int *)(param_1 + 0x740) * 4 +
                                                    *(int *)(param_1 + 0x5c) * 0x2a0) * 2 +
                                            *(int *)(param_1 + 0x25c) * 0x70 +
                                            *(int *)(param_1 + 0x5c) * 0x1a40));
                if ((('\0' < *(char *)(param_1 + 0x79c)) && (*(int *)(param_1 + 0x19c) != 0)) &&
                   (local_23c[0x18] != 0)) {
                  FUN_004bd906(*(undefined4 *)(&DAT_0067249c + *(int *)(param_1 + 0x7a0) * 0x2a0));
                }
                *(undefined4 *)(local_23c[0x18] + 0x2e4) =
                     *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2e4);
                local_23c[0x67] = 0;
                FUN_0054ac09(local_23c);
              }
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x5a) {
            if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0xaf), iVar3 != 0)) {
              uVar10 = 0;
              uVar4 = 0;
              FUN_00429a6d(0xaf);
              FUN_0042ad2b(uVar4,uVar10);
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x5b) {
            if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0xa9), iVar3 != 0)) {
              uVar10 = 0;
              uVar4 = 0;
              FUN_00429a6d(0xa9);
              FUN_0042ad2b(uVar4,uVar10);
            }
          }
          else if (*(short *)(param_1 + 0x4d0) == 0x5f) {
            local_254 = local_34 + _DAT_005a4678;
            local_258 = local_38;
            local_250 = local_30;
            local_264 = 3;
            local_34 = local_254;
            if (*(int *)(param_1 + 0x4cc) != 0) {
              uVar4 = FUN_00430f50(*(undefined4 *)(*(int *)(param_1 + 0x4cc) + 0x28),
                                   *(float *)(*(int *)(param_1 + 0x4cc) + 0x38) / _DAT_005a430c +
                                   *(float *)(*(int *)(param_1 + 0x4cc) + 0x30),
                                   *(undefined4 *)(*(int *)(param_1 + 0x4cc) + 0x2c));
              FUN_004310a0(uVar4);
              local_264 = local_264 + *(byte *)(param_1 + 0x45f) / 0x50;
              if (local_264 < 3) {
                local_264 = 3;
              }
              if (6 < local_264) {
                local_264 = 6;
              }
            }
            local_24c = local_2c;
            local_248 = local_28;
            local_244 = local_24;
            local_260 = 0;
            for (local_25c = 0; local_25c < local_264; local_25c = local_25c + 1) {
              local_270 = _rand();
              local_270 = local_270 % 1000;
              if (1 < local_25c) {
                local_260 = local_25c + 3;
              }
              fVar11 = local_24c - _DAT_005a34a0;
              fVar9 = *(float *)(param_1 + 0x38) * _DAT_005a3660;
              fVar1 = ((float)local_25c / _DAT_005a3794) * _DAT_005a4148;
              fVar7 = (float10)FUN_00423b00(fVar1 + fVar1);
              local_24c = (float)(fVar7 * (float10)fVar9 + (float10)fVar11);
              fVar11 = local_244 - _DAT_005a34a0;
              fVar9 = *(float *)(param_1 + 0x38) * _DAT_005a3660;
              fVar1 = ((float)local_25c / _DAT_005a3794) * _DAT_005a4148;
              fVar7 = (float10)FUN_00493bf0(fVar1 + fVar1);
              local_244 = (float)(fVar7 * (float10)fVar9 + (float10)fVar11);
              local_248 = *(float *)(param_1 + 0x30) + *(float *)(param_1 + 0x38);
              local_274 = &DAT_00777777;
              if (local_260 == 8) {
                local_274 = &DAT_00883333;
              }
              else if (local_260 == 7) {
                local_274 = &DAT_00884388;
              }
              else if (local_260 == 6) {
                local_274 = (undefined *)0x338843;
              }
              else if (local_260 == 5) {
                local_274 = (undefined *)0x222288;
              }
              pvVar2 = operator_new(0x180);
              local_8 = 0x2e;
              if (pvVar2 == (void *)0x0) {
                local_988 = (int *)0x0;
              }
              else {
                local_988 = (int *)FUN_004c8f70(0x38,local_25c * 100 + 400,0x3f800000,0x3f800000,
                                                0x3f800000,0,1,0x50);
              }
              local_8 = 0xffffffff;
              local_26c = local_988;
              if (local_988 != (int *)0x0) {
                (**(code **)(*local_988 + 0x3c))(local_274);
                local_26c[0x1b] = 1;
                local_26c[0x41] = 2;
                FUN_004310a0(&local_24c);
                FUN_0054ac09(local_26c);
              }
              pvVar2 = operator_new(0x94);
              local_8 = 0x2f;
              if (pvVar2 == (void *)0x0) {
                local_98c = 0;
              }
              else {
                local_98c = FUN_004c5cc0(local_24c,local_248,local_244,local_258,local_254,local_250
                                         ,(int)*(short *)(param_1 + 0x4d2),0x97,0,local_260,0);
              }
              local_8 = 0xffffffff;
              local_268 = local_98c;
              *(int *)(local_98c + 0x24) = *(int *)(local_98c + 0x24) + local_25c * 100;
              if (local_98c != 0) {
                FUN_0054ac09(local_98c);
              }
            }
          }
          else if ((((0x96 < *(short *)(param_1 + 0x4d0)) && (*(short *)(param_1 + 0x4d0) < 0x9a))
                   || (*(short *)(param_1 + 0x4d0) == 0x68)) ||
                  (*(short *)(param_1 + 0x4d0) == 0x69)) {
            local_28 = *(float *)(&DAT_0092e7d4 + *(int *)(param_1 + 0x5c) * 8) *
                       *(float *)(param_1 + 0x38) * _DAT_005a4160 + *(float *)(param_1 + 0x30);
            if (*(int *)(param_1 + 0x5c) == 8) {
              local_28 = _DAT_005a3660 * *(float *)(param_1 + 0x38) + local_28;
            }
            local_27c = local_34 + _DAT_005a4678;
            local_280 = local_38;
            local_278 = local_30;
            local_34 = local_27c;
            if (*(int *)(param_1 + 0x4cc) != 0) {
              uVar4 = FUN_00430f50(*(undefined4 *)(*(int *)(param_1 + 0x4cc) + 0x28),
                                   *(float *)(*(int *)(param_1 + 0x4cc) + 0x38) / _DAT_005a430c +
                                   *(float *)(*(int *)(param_1 + 0x4cc) + 0x30),
                                   *(undefined4 *)(*(int *)(param_1 + 0x4cc) + 0x2c));
              FUN_004310a0(uVar4);
            }
            *(int *)(param_1 + 0x770) = *(int *)(param_1 + 0x770) + 1;
            if (((*(char *)(param_1 + 0x799) == '\x01') && (*(int *)(param_1 + 0x554) != 0)) &&
               (*(char *)(param_1 + 0x245) != '\x01')) {
              pvVar2 = operator_new(0x94);
              local_8 = 0x30;
              if (pvVar2 == (void *)0x0) {
                local_990 = 0;
              }
              else {
                iVar3 = *(int *)(param_1 + 0x554);
                local_990 = FUN_004c5cc0(*(undefined4 *)(iVar3 + 0x74),*(undefined4 *)(iVar3 + 0x78)
                                         ,*(undefined4 *)(iVar3 + 0x7c),local_280,local_27c,
                                         local_278,(int)*(short *)(param_1 + 0x4d2),10000,
                                         *(undefined1 *)(param_1 + 0x79a),0,
                                         *(undefined4 *)(param_1 + 0x7d4));
              }
              local_8 = 0xffffffff;
              local_284 = local_990;
              if (local_990 != 0) {
                FUN_0054ac09(local_990);
              }
            }
            if ((*(short *)(param_1 + 0x4d0) == 0x69) && (*(short *)(param_1 + 0x4d2) == 2)) {
              pvVar2 = operator_new(200);
              local_8 = 0x31;
              if (pvVar2 == (void *)0x0) {
                local_994 = 0;
              }
              else {
                local_994 = FUN_004e1290(local_2c,local_28,local_24,local_280,local_27c,local_278,3,
                                         *(undefined4 *)(param_1 + 0x4cc));
              }
              local_8 = 0xffffffff;
              local_288 = local_994;
              if (local_994 != 0) {
                FUN_0054ac09(local_994);
              }
              *(int *)(param_1 + 0x4bc) = *(int *)(param_1 + 0x4bc) + 700;
            }
            else {
              pvVar2 = operator_new(0x94);
              local_8 = 0x32;
              if (pvVar2 == (void *)0x0) {
                local_998 = 0;
              }
              else {
                local_998 = FUN_004c5cc0(local_2c,local_28,local_24,local_280,local_27c,local_278,
                                         (int)*(short *)(param_1 + 0x4d2),
                                         (int)*(short *)(param_1 + 0x4d0),
                                         *(undefined1 *)(param_1 + 0x79a),
                                         *(undefined1 *)(param_1 + 0x201),
                                         *(undefined4 *)(param_1 + 0x7d4));
              }
              local_8 = 0xffffffff;
              local_28c = local_998;
              if (local_998 != 0) {
                FUN_0054ac09(local_998);
              }
              *(int *)(param_1 + 0x4bc) = *(int *)(param_1 + 0x4bc) + 500;
            }
          }
        }
        if ((*(int *)(param_1 + 0x764) == 0) || (1 < *(int *)(param_1 + 0x770))) {
          _memset((void *)(param_1 + 0x4bc),0,0x18);
          *(undefined4 *)(param_1 + 0x770) = 0;
        }
      }
      if ((*(char *)(param_1 + 0x78a) == '\x01') && (DAT_0092e660 == 0)) {
        if (500 < param_2 - *(int *)(param_1 + 0x618)) {
          iVar3 = _rand();
          local_29c = (float)(iVar3 % 5);
          local_298 = 1.0;
          if (*(char *)(param_1 + 0x79c) == '\x01') {
            local_298 = 1.3;
          }
          for (local_294 = 0; local_294 < 2; local_294 = local_294 + 1) {
            if (*(int *)(param_1 + 0x528 + local_294 * 4) == 0) {
              pvVar2 = operator_new(0x180);
              local_8 = 0x33;
              if (pvVar2 == (void *)0x0) {
                local_9a0 = 0;
              }
              else {
                local_9a0 = FUN_004c8f70(0x6d,0,(_DAT_005a4158 * local_29c + _DAT_005a3660) *
                                                local_298 - (float)local_294 * _DAT_005a4154,
                                         (_DAT_005a4158 * local_29c + _DAT_005a3660) * local_298 -
                                         (float)local_294 * _DAT_005a4154,
                                         (_DAT_005a4158 * local_29c + _DAT_005a3660) * local_298 -
                                         (float)local_294 * _DAT_005a4154,0,8,0x50);
              }
              local_8 = 0xffffffff;
              *(undefined4 *)(param_1 + 0x528 + local_294 * 4) = local_9a0;
              (**(code **)(**(int **)(param_1 + 0x528 + local_294 * 4) + 0x3c))(0xffffdd00);
              *(undefined4 *)(*(int *)(param_1 + 0x528 + local_294 * 4) + 0x6c) = 1;
              *(undefined4 *)(*(int *)(param_1 + 0x528 + local_294 * 4) + 0x104) = 0;
              FUN_0054ac09(*(undefined4 *)(param_1 + 0x528 + local_294 * 4));
            }
            if (*(int *)(param_1 + 0x528 + local_294 * 4) != 0) {
              *(float *)(*(int *)(param_1 + 0x528 + local_294 * 4) + 0x114) =
                   ((float)((param_2 - *(int *)(*(int *)(param_1 + 0x528 + local_294 * 4) + 0xfc)) %
                           1000) * _DAT_005a4290) / _DAT_005a43c0 + (float)local_294;
              uVar4 = FUN_00430f50(*(float *)(param_1 + 0x28) + _DAT_005a34a0,
                                   *(float *)(param_1 + 0x30) + _DAT_005a4418,
                                   *(float *)(param_1 + 0x2c) + _DAT_005a34a0);
              FUN_004310a0(uVar4);
            }
          }
          *(uint *)(param_1 + 0x618) = param_2;
        }
        for (local_290 = 0; local_290 < 2; local_290 = local_290 + 1) {
          if (*(int *)(param_1 + 0x528 + local_290 * 4) != 0) {
            uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x50),
                                 *(float *)(param_1 + 0x54) + _DAT_005a4678,
                                 *(undefined4 *)(param_1 + 0x58));
            FUN_004310a0(uVar4);
          }
        }
      }
      else {
        for (local_2a0 = 0; local_2a0 < 2; local_2a0 = local_2a0 + 1) {
          if (*(int *)(param_1 + 0x528 + local_2a0 * 4) != 0) {
            (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x528 + local_2a0 * 4));
            *(undefined4 *)(param_1 + 0x528 + local_2a0 * 4) = 0;
          }
        }
      }
      if ((*(char *)(param_1 + 0x785) == '\x01') && (1000 < param_2 - *(int *)(param_1 + 0x60c))) {
        if (*(int *)(param_1 + 0x520) != 0) {
          FUN_004e8a95(param_2);
        }
        *(uint *)(param_1 + 0x60c) = param_2;
      }
      if (*(char *)(param_1 + 0x788) == '\x01') {
        local_2a4 = 1000;
        if ((*(int *)(param_1 + 0x24c) == 0x38) && (*(short *)(param_1 + 0x1d2) == 0)) {
          local_2a4 = 3000;
        }
        if ((local_2a4 < param_2 - *(int *)(param_1 + 0x610)) && (*(int *)(param_1 + 0x524) != 0)) {
          if ((*(int *)(param_1 + 0x24c) == 0x38) && (*(short *)(param_1 + 0x1d2) == 0)) {
            iVar3 = _rand();
            *(float *)(*(int *)(param_1 + 0x524) + 0xac) = (float)-(iVar3 % 0x32) / _DAT_005a40f8;
          }
          FUN_004e8a95(param_2);
          *(float *)(*(int *)(param_1 + 0x524) + 0x90) =
               *(float *)(&DAT_0092e7d0 + *(int *)(param_1 + 0x5c) * 8) * *(float *)(param_1 + 0x38)
          ;
          *(uint *)(param_1 + 0x610) = param_2;
        }
      }
      if ((*(char *)(param_1 + 0x78b) == '\x01') && (1000 < param_2 - *(int *)(param_1 + 0x624))) {
        FUN_00430f50(*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x30),
                     *(undefined4 *)(param_1 + 0x2c));
        pvVar2 = operator_new(0xa8);
        local_8 = 0x34;
        if (pvVar2 == (void *)0x0) {
          local_9b0 = 0;
        }
        else {
          local_9b0 = FUN_004ecd90(local_2c8[5],local_2b0,local_2ac,local_2c8[5],local_2b0,local_2ac
                                   ,1,*(undefined4 *)(param_1 + 0x4cc));
        }
        local_8 = 0xffffffff;
        local_2a8 = local_9b0;
        if (local_9b0 != 0) {
          FUN_0054ac09(local_9b0);
        }
        *(uint *)(param_1 + 0x624) = param_2;
      }
      if (*(char *)(param_1 + 0x78f) == '\x01') {
        if (*(int *)(param_1 + 0x548) == 0) {
          pvVar2 = operator_new(0xa4);
          local_8 = 0x35;
          if (pvVar2 == (void *)0x0) {
            local_9b4 = 0;
          }
          else {
            local_9b4 = FUN_004ce5e0(param_1,2,0xff66ff66);
          }
          local_8 = 0xffffffff;
          *(undefined4 *)(param_1 + 0x548) = local_9b4;
          if (*(int *)(param_1 + 0x548) != 0) {
            FUN_0054ac09(*(undefined4 *)(param_1 + 0x548));
          }
        }
      }
      else if (*(int *)(param_1 + 0x548) != 0) {
        (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x548));
        *(undefined4 *)(param_1 + 0x548) = 0;
      }
      if ((((*(char *)(param_1 + 0x792) == '\x01') || (*(int *)(param_1 + 0x24c) == 0x2c)) &&
          (*(char *)(param_1 + 0x245) != '\x01')) && (DAT_0092e660 == 0)) {
        local_2c8[0] = 0xff99bbff;
        local_2c8[1] = 0xff00ffaa;
        local_2c8[2] = 0xffffaa00;
        local_2c8[3] = 0xff880088;
        local_2c8[4] = 0xffcc8888;
        for (local_2cc = 0; (int)local_2cc < 2; local_2cc = local_2cc + 1) {
          if (*(int *)(param_1 + 0x5d0 + local_2cc * 4) == 0) {
            pvVar2 = operator_new(0xcc);
            local_8 = 0x36;
            if (pvVar2 == (void *)0x0) {
              local_9b8 = 0;
            }
            else {
              local_9b8 = FUN_004d2230(0xc,local_2c8[local_2cc],0,1);
            }
            local_8 = 0xffffffff;
            *(undefined4 *)(param_1 + 0x5d0 + local_2cc * 4) = local_9b8;
            if (*(int *)(param_1 + 0x5d0 + local_2cc * 4) != 0) {
              *(undefined4 *)(*(int *)(param_1 + 0x5d0 + local_2cc * 4) + 0xa4) = 0;
              FUN_004310a0(param_1 + 0x50);
              FUN_0054ac09(*(undefined4 *)(param_1 + 0x5d0 + local_2cc * 4));
            }
          }
          else {
            local_2d0 = ((float)(param_2 % 1000) / _DAT_005a43c0 +
                        ((float)((int)local_2cc % 5) * _DAT_005a3660) / _DAT_005a4104) *
                        _DAT_005a4290;
            uVar6 = local_2cc & 0x80000001;
            if ((int)uVar6 < 0) {
              uVar6 = (uVar6 - 1 | 0xfffffffe) + 1;
            }
            if (uVar6 == 0) {
              *(float *)(*(int *)(param_1 + 0x5d0 + local_2cc * 4) + 0x8c) = local_2d0;
            }
            if ((int)local_2cc % 3 == 1) {
              *(float *)(*(int *)(param_1 + 0x5d0 + local_2cc * 4) + 0x90) = local_2d0;
            }
            if (*(char *)(param_1 + 0x79c) == '\x01') {
              uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x50),*(undefined4 *)(param_1 + 0x54),
                                   *(undefined4 *)(param_1 + 0x58));
              FUN_004310a0(uVar4);
              *(undefined4 *)(*(int *)(param_1 + 0x5d0 + local_2cc * 4) + 0x98) = 0x3fc00000;
              *(undefined4 *)(*(int *)(param_1 + 0x5d0 + local_2cc * 4) + 0x9c) = 0x3fc00000;
            }
            else {
              uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                                   (*(float *)(&DAT_0092e7d4 + *(int *)(param_1 + 0x5c) * 8) *
                                   *(float *)(param_1 + 0x38)) / _DAT_005a365c +
                                   *(float *)(param_1 + 0x30),*(undefined4 *)(param_1 + 0x2c));
              FUN_004310a0(uVar4);
              *(undefined4 *)(*(int *)(param_1 + 0x5d0 + local_2cc * 4) + 0x98) = 0x3f800000;
              *(undefined4 *)(*(int *)(param_1 + 0x5d0 + local_2cc * 4) + 0x9c) = 0x3f800000;
            }
            if (*(int *)(param_1 + 0x24c) == 0x2c) {
              *(undefined4 *)(*(int *)(param_1 + 0x5d0 + local_2cc * 4) + 0xa4) = 0x2d;
              *(undefined4 *)(*(int *)(param_1 + 0x5d0 + local_2cc * 4) + 0x98) =
                   *(undefined4 *)(param_1 + 0x38);
              *(undefined4 *)(*(int *)(param_1 + 0x5d0 + local_2cc * 4) + 0x9c) =
                   *(undefined4 *)(param_1 + 0x38);
              *(float *)(*(int *)(param_1 + 0x5d0 + local_2cc * 4) + 0x78) =
                   *(float *)(param_1 + 0x38) * _DAT_005a5644 +
                   *(float *)(*(int *)(param_1 + 0x5d0 + local_2cc * 4) + 0x78);
            }
          }
        }
      }
      else {
        for (local_2d4 = 0; local_2d4 < 5; local_2d4 = local_2d4 + 1) {
          if (*(int *)(param_1 + 0x5d0 + local_2d4 * 4) != 0) {
            (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x5d0 + local_2d4 * 4));
            *(undefined4 *)(param_1 + 0x5d0 + local_2d4 * 4) = 0;
          }
        }
      }
      if (((DAT_0092e664 == 0) && (*(char *)(param_1 + 0x797) == '\x01')) &&
         (500 < param_2 - *(int *)(param_1 + 0x648))) {
        FUN_00430f50(*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x30),
                     *(undefined4 *)(param_1 + 0x2c));
        pvVar2 = operator_new(0xa8);
        local_8 = 0x37;
        if (pvVar2 == (void *)0x0) {
          local_9c8 = 0;
        }
        else {
          local_9c8 = FUN_004ecd90(local_2e4,local_2e0,local_2dc,local_2e4,local_2e0,local_2dc,2,
                                   param_1);
        }
        local_8 = 0xffffffff;
        local_2d8 = local_9c8;
        if (local_9c8 != 0) {
          FUN_0054ac09(local_9c8);
        }
        *(uint *)(param_1 + 0x648) = param_2;
        if (*(int *)(param_1 + 0x514) != 0) {
          FUN_004e8a95(param_2);
        }
      }
      if (*(char *)(param_1 + 0x786) == '\x01') {
        if (*(int *)(param_1 + 0x538) == 0) {
          pvVar2 = operator_new(0x124);
          local_8 = 0x38;
          if (pvVar2 == (void *)0x0) {
            local_9cc = 0;
          }
          else {
            local_9cc = FUN_004cb900(0x5e,0,0x3fc00000,0x3fc00000,0x3fc00000,0,3000);
          }
          local_8 = 0xffffffff;
          *(undefined4 *)(param_1 + 0x538) = local_9cc;
          if (*(int *)(param_1 + 0x538) != 0) {
            *(undefined4 *)(*(int *)(param_1 + 0x538) + 0x104) = 2;
            *(undefined4 *)(*(int *)(param_1 + 0x538) + 0x6c) = 1;
            uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                                 *(float *)(param_1 + 0x30) + _DAT_005a40ec,
                                 *(undefined4 *)(param_1 + 0x2c));
            FUN_004310a0(uVar4);
            (**(code **)(**(int **)(param_1 + 0x538) + 0x3c))(0x33333333);
            FUN_0054ac09(*(undefined4 *)(param_1 + 0x538));
          }
        }
        else {
          uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                               *(float *)(param_1 + 0x30) + _DAT_005a40ec,
                               *(undefined4 *)(param_1 + 0x2c));
          FUN_004310a0(uVar4);
        }
      }
      else if (*(int *)(param_1 + 0x538) != 0) {
        (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x538));
        *(undefined4 *)(param_1 + 0x538) = 0;
      }
      if (((DAT_0092e664 == 0) && (*(char *)(param_1 + 0x78e) == '\x01')) &&
         (((*(int *)(param_1 + 0x740) == 2 || (*(int *)(param_1 + 0x740) == 3)) &&
          (0x50 < param_2 - *(int *)(param_1 + 0x638))))) {
        local_2ec = 0;
        if ((*(int *)(param_1 + 0x24c) == 4) || (*(int *)(param_1 + 0x24c) == 8)) {
          local_2ec = 1;
        }
        pvVar2 = operator_new(0x22c);
        local_8 = 0x39;
        if (pvVar2 == (void *)0x0) {
          local_9d0 = (int *)0x0;
        }
        else {
          uVar12 = 0;
          uVar8 = 0;
          puVar5 = (undefined4 *)FUN_00430f50(0,0,0);
          uVar4 = *puVar5;
          uVar10 = puVar5[1];
          uVar13 = puVar5[2];
          puVar5 = (undefined4 *)FUN_00430f50(0,0,0);
          local_9d0 = (int *)FUN_004d5480(*(undefined4 *)(param_1 + 0x5c),*puVar5,puVar5[1],
                                          puVar5[2],uVar4,uVar10,uVar13,uVar8,uVar12);
        }
        local_8 = 0xffffffff;
        local_2e8 = local_9d0;
        if (local_9d0 != (int *)0x0) {
          if ((*(char *)(param_1 + 0x79c) < '\x01') || (*(int *)(param_1 + 0x19c) == 0)) {
            FUN_0058f220(local_9d0 + 0x58,param_1 + 0x1d2,0x20);
            local_2e8[0xe] = *(int *)(param_1 + 0x38);
            (**(code **)(*local_2e8 + 0x3c))
                      (*(undefined4 *)(param_1 + 0x28),*(float *)(param_1 + 0x30) + _DAT_005a4214,
                       *(undefined4 *)(param_1 + 0x2c));
          }
          else {
            local_2ec = 0;
            local_9d0[0x17] = *(int *)(param_1 + 0x7a0);
            FUN_0058f220(local_9d0 + 0x58,param_1 + 0x1a2,0x20);
            local_2e8[0xe] = *(int *)(param_1 + 0x38);
            (**(code **)(*local_2e8 + 0x3c))
                      (*(undefined4 *)(*(int *)(param_1 + 0x19c) + 0xc),
                       *(float *)(param_1 + 0x30) + _DAT_005a4214,
                       *(undefined4 *)(*(int *)(param_1 + 0x19c) + 0x14));
          }
          local_2e8[0x75] = 1;
          local_2e8[0x69] = 0x3e99999a;
          local_2e8[0x6a] = 0x3e99999a;
          local_2e8[0x6b] = 0x3e99999a;
          (**(code **)(*local_2e8 + 0x78))(local_2ec);
          local_2e8[100] = param_2;
          local_2e8[0x65] = 200;
          local_2e8[0x68] = *(int *)(param_1 + 0x34);
          local_2e8[0x7d] = local_2e8[0x68];
          local_2e8[0x66] = 1;
          FUN_004bd906((int)*(short *)(&DAT_005d0508 +
                                      *(int *)(&DAT_00672490 +
                                              *(int *)(param_1 + 0x740) * 4 +
                                              *(int *)(param_1 + 0x5c) * 0x2a0) * 2 +
                                      *(int *)(param_1 + 0x25c) * 0x70 +
                                      *(int *)(param_1 + 0x5c) * 0x1a40));
          if ((('\0' < *(char *)(param_1 + 0x79c)) && (*(int *)(param_1 + 0x19c) != 0)) &&
             (local_2e8[0x18] != 0)) {
            FUN_004bd906(*(undefined4 *)(&DAT_0067249c + *(int *)(param_1 + 0x7a0) * 0x2a0));
          }
          *(undefined4 *)(local_2e8[0x18] + 0x2e4) =
               *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2e4);
          local_2e8[0x67] = 0;
          FUN_0054ac09(local_2e8);
        }
        *(uint *)(param_1 + 0x638) = param_2;
      }
      if (*(char *)(param_1 + 0x789) == '\x01') {
        if (*(int *)(param_1 + 0x53c) == 0) {
          pvVar2 = operator_new(0x124);
          local_8 = 0x3a;
          if (pvVar2 == (void *)0x0) {
            local_9d4 = 0;
          }
          else {
            local_9d4 = FUN_004cb900(0x5d,0,0x3fc00000,0x3fc00000,0x3fc00000,0,5000);
          }
          local_8 = 0xffffffff;
          *(undefined4 *)(param_1 + 0x53c) = local_9d4;
          if (*(int *)(param_1 + 0x53c) != 0) {
            *(undefined4 *)(*(int *)(param_1 + 0x53c) + 0x104) = 3;
            *(undefined4 *)(*(int *)(param_1 + 0x53c) + 0x6c) = 1;
            uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                                 *(float *)(param_1 + 0x30) + _DAT_005a4154,
                                 *(undefined4 *)(param_1 + 0x2c));
            FUN_004310a0(uVar4);
            (**(code **)(**(int **)(param_1 + 0x53c) + 0x3c))(0x88888800);
            FUN_0054ac09(*(undefined4 *)(param_1 + 0x53c));
          }
        }
        else {
          uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                               *(float *)(param_1 + 0x30) + _DAT_005a4154,
                               *(undefined4 *)(param_1 + 0x2c));
          FUN_004310a0(uVar4);
        }
      }
      else if (*(int *)(param_1 + 0x53c) != 0) {
        (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x53c));
        *(undefined4 *)(param_1 + 0x53c) = 0;
      }
      if (*(char *)(param_1 + 0x78d) == '\x01') {
        if (*(int *)(param_1 + 0x540) == 0) {
          pvVar2 = operator_new(0x124);
          local_8 = 0x3b;
          if (pvVar2 == (void *)0x0) {
            local_9d8 = 0;
          }
          else {
            local_9d8 = FUN_004cb900(0x5e,0,0x3f99999a,0x3f99999a,0x3f99999a,0,5000);
          }
          local_8 = 0xffffffff;
          *(undefined4 *)(param_1 + 0x540) = local_9d8;
          if (*(int *)(param_1 + 0x540) != 0) {
            *(undefined4 *)(*(int *)(param_1 + 0x540) + 0x104) = 4;
            *(undefined4 *)(*(int *)(param_1 + 0x540) + 0x6c) = 1;
            uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                                 *(float *)(param_1 + 0x30) + _DAT_005a34a0,
                                 *(undefined4 *)(param_1 + 0x2c));
            FUN_004310a0(uVar4);
            (**(code **)(**(int **)(param_1 + 0x540) + 0x3c))(0xaaaa0000);
            FUN_0054ac09(*(undefined4 *)(param_1 + 0x540));
          }
        }
        else {
          uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                               *(float *)(param_1 + 0x30) + _DAT_005a34a0,
                               *(undefined4 *)(param_1 + 0x2c));
          FUN_004310a0(uVar4);
        }
      }
      else if (*(int *)(param_1 + 0x540) != 0) {
        (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x540));
        *(undefined4 *)(param_1 + 0x540) = 0;
      }
      if (((*(int *)(param_1 + 0x24c) != 8) || (*(char *)(param_1 + 0x796) != '\x01')) &&
         ((*(int *)(param_1 + 0x24c) == 8 && (*(int *)(param_1 + 0x56c) != 0)))) {
        for (local_2f0 = 0; local_2f0 < 7; local_2f0 = local_2f0 + 1) {
          if (*(int *)(param_1 + 0x56c + local_2f0 * 4) != 0) {
            (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x56c + local_2f0 * 4));
            *(undefined4 *)(param_1 + 0x56c + local_2f0 * 4) = 0;
          }
        }
      }
      if (*(char *)(param_1 + 0x791) == '\x01') {
        if (*(int *)(param_1 + 0x544) == 0) {
          pvVar2 = operator_new(0x180);
          local_8 = 0x3c;
          if (pvVar2 == (void *)0x0) {
            local_9dc = 0;
          }
          else {
            local_9dc = FUN_004c8f70(0x1b6,0,0x3ecccccd,0x3ecccccd,0x3ecccccd,0,1,0x50);
          }
          local_8 = 0xffffffff;
          *(undefined4 *)(param_1 + 0x544) = local_9dc;
          if (*(int *)(param_1 + 0x544) != 0) {
            *(undefined4 *)(*(int *)(param_1 + 0x544) + 0x6c) = 1;
            *(undefined4 *)(*(int *)(param_1 + 0x544) + 0x104) = 2;
            if (*(char *)(param_1 + 0x79c) == '\x01') {
              uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x50),
                                   (*(float *)(&DAT_0092e7d4 + *(int *)(param_1 + 0x5c) * 8) *
                                    *(float *)(param_1 + 0x38) + *(float *)(param_1 + 0x54)) -
                                   _DAT_005a40ec,*(undefined4 *)(param_1 + 0x58));
              FUN_004310a0(uVar4);
            }
            else {
              uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                                   *(float *)(&DAT_0092e7d4 + *(int *)(param_1 + 0x5c) * 8) *
                                   *(float *)(param_1 + 0x38) + *(float *)(param_1 + 0x30),
                                   *(undefined4 *)(param_1 + 0x2c));
              FUN_004310a0(uVar4);
            }
            *(undefined4 *)(*(int *)(param_1 + 0x544) + 0x148) = 10;
            *(undefined4 *)(*(int *)(param_1 + 0x544) + 0x13c) = 0x3dcccccd;
            *(undefined4 *)(*(int *)(param_1 + 0x544) + 0x140) = 0x3dcccccd;
            FUN_0054ac09(*(undefined4 *)(param_1 + 0x544));
          }
        }
        else if (*(char *)(param_1 + 0x79c) == '\x01') {
          uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x50),
                               (*(float *)(&DAT_0092e7d4 + *(int *)(param_1 + 0x5c) * 8) *
                                *(float *)(param_1 + 0x38) + *(float *)(param_1 + 0x54)) -
                               _DAT_005a40ec,*(undefined4 *)(param_1 + 0x58));
          FUN_004310a0(uVar4);
        }
        else {
          uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                               *(float *)(&DAT_0092e7d4 + *(int *)(param_1 + 0x5c) * 8) *
                               *(float *)(param_1 + 0x38) + *(float *)(param_1 + 0x30),
                               *(undefined4 *)(param_1 + 0x2c));
          FUN_004310a0(uVar4);
        }
      }
      else if (*(int *)(param_1 + 0x544) != 0) {
        (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x544));
        *(undefined4 *)(param_1 + 0x544) = 0;
      }
      if ((DAT_0092e664 == 0) && (*(char *)(param_1 + 0x793) == '\x01')) {
        local_2fc = _rand();
        local_2fc = local_2fc % 5;
        pvVar2 = operator_new(0x180);
        local_8 = 0x3d;
        if (pvVar2 == (void *)0x0) {
          local_9e0 = (int *)0x0;
        }
        else {
          local_9e0 = (int *)FUN_004c8f70(0x38,0x5dc,
                                          (float)local_2fc * _DAT_005a41e8 + _DAT_005a41e8,
                                          (float)local_2fc * _DAT_005a439c + _DAT_005a41e8,
                                          (float)local_2fc * _DAT_005a41e8 + _DAT_005a41e8,0,1,0x50)
          ;
        }
        local_8 = 0xffffffff;
        local_2f4 = local_9e0;
        if (local_9e0 != (int *)0x0) {
          iVar3 = _rand();
          fVar11 = (float)(iVar3 % 10 + -5) * _DAT_005a5640 + *(float *)(param_1 + 0x2c);
          fVar9 = (*(float *)(&DAT_0092e7d4 + *(int *)(param_1 + 0x5c) * 8) *
                   *(float *)(param_1 + 0x38) + *(float *)(param_1 + 0x30)) -
                  (float)local_2fc * _DAT_005a439c;
          iVar3 = _rand();
          uVar4 = FUN_00430f50((float)(iVar3 % 10 + -5) * _DAT_005a5640 + *(float *)(param_1 + 0x28)
                               ,fVar9,fVar11);
          uVar4 = FUN_004310a0(uVar4);
          FUN_004310a0(uVar4);
          local_2f4[0x1b] = 1;
          local_2f4[0x40] = 0;
          local_2f4[0x52] = local_2fc % 3;
          local_2f4[0x50] = -0x40666666;
          local_2f4[0x4f] = 0x3dcccccd;
          (**(code **)(*local_2f4 + 0x3c))(0xffff3300);
          FUN_0054ac09(local_2f4);
        }
        pvVar2 = operator_new(0x180);
        local_8 = 0x3e;
        if (pvVar2 == (void *)0x0) {
          local_9ec = (int *)0x0;
        }
        else {
          local_9ec = (int *)FUN_004c8f70(0,0x5dc,(float)local_2fc * _DAT_005a4214 + _DAT_005a4214,
                                          (float)local_2fc * _DAT_005a439c + _DAT_005a415c,
                                          (float)local_2fc * _DAT_005a4214 + _DAT_005a4214,
                                          0x38d1b717,1,0x50);
        }
        local_8 = 0xffffffff;
        local_2f8 = local_9ec;
        if (local_9ec != (int *)0x0) {
          iVar3 = _rand();
          fVar11 = (float)(iVar3 % 10 + -5) * _DAT_005a4214 + *(float *)(param_1 + 0x2c);
          uVar4 = *(undefined4 *)(param_1 + 0x30);
          iVar3 = _rand();
          uVar4 = FUN_00430f50((float)(iVar3 % 10 + -5) * _DAT_005a4214 + *(float *)(param_1 + 0x28)
                               ,uVar4,fVar11);
          uVar4 = FUN_004310a0(uVar4);
          FUN_004310a0(uVar4);
          local_2f8[0x1b] = 1;
          local_2f8[0x40] = 0;
          local_2f8[0x52] = 1;
          local_2f8[0x50] = 0x3fc00000;
          (**(code **)(*local_2f8 + 0x3c))(0xffff0000);
          FUN_0054ac09(local_2f8);
        }
      }
      if (((DAT_0092e664 == 0) && (*(char *)(param_1 + 0x78c) == '\x01')) &&
         (*(int *)(param_1 + 0x55c) == 0)) {
        local_300 = 3;
        FUN_00430f50(*(undefined4 *)(param_1 + 0x28),*(float *)(param_1 + 0x30) + _DAT_005a4bd8,
                     *(undefined4 *)(param_1 + 0x2c));
        pvVar2 = operator_new(0x22c);
        local_8 = 0x3f;
        if (pvVar2 == (void *)0x0) {
          local_9f8 = 0;
        }
        else {
          local_9f8 = FUN_004d5480(0x20,local_30c,local_308,local_304,local_30c,local_308,local_304,
                                   local_300,param_1);
        }
        local_8 = 0xffffffff;
        *(undefined4 *)(param_1 + 0x55c) = local_9f8;
        if (*(int *)(param_1 + 0x55c) != 0) {
          *(undefined2 *)(*(int *)(param_1 + 0x55c) + 0x164) = 0;
          *(undefined2 *)(*(int *)(param_1 + 0x55c) + 0x160) = 0;
          *(undefined2 *)(*(int *)(param_1 + 0x55c) + 0x166) = 0;
          *(undefined2 *)(*(int *)(param_1 + 0x55c) + 0x162) = 0;
          (**(code **)(**(int **)(param_1 + 0x55c) + 0x78))(0);
          *(undefined4 *)(*(int *)(param_1 + 0x55c) + 0x194) = 0;
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x55c) + 0x60) + 0x24) = 0x3f000000;
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x55c) + 0x60) + 0x28) = 0x3f000000;
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x55c) + 0x60) + 0x2c) = 0x3f000000;
          *(undefined4 *)(*(int *)(param_1 + 0x55c) + 0x198) = 1;
          *(undefined4 *)(*(int *)(param_1 + 0x55c) + 0x1a4) = 0x3f800000;
          *(undefined4 *)(*(int *)(param_1 + 0x55c) + 0x1a8) = 0x3f800000;
          *(undefined4 *)(*(int *)(param_1 + 0x55c) + 0x1ac) = 0x3f800000;
          *(undefined4 *)(*(int *)(param_1 + 0x55c) + 0x1d4) = 1;
          *(undefined4 *)(*(int *)(param_1 + 0x55c) + 0x1a0) = *(undefined4 *)(param_1 + 0x34);
          *(undefined4 *)(*(int *)(param_1 + 0x55c) + 0x19c) = 5;
          FUN_0054ac09(*(undefined4 *)(param_1 + 0x55c));
        }
      }
      else if ((*(char *)(param_1 + 0x78c) == '\0') && (*(int *)(param_1 + 0x55c) != 0)) {
        (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x55c));
        *(undefined4 *)(param_1 + 0x55c) = 0;
      }
      if (*(char *)(param_1 + 0x798) == '\x01') {
        if (*(int *)(param_1 + 0x550) == 0) {
          pvVar2 = operator_new(0xcc);
          local_8 = 0x40;
          if (pvVar2 == (void *)0x0) {
            local_9fc = 0;
          }
          else {
            local_9fc = FUN_004d2230(0xb16,0xff999999,*(undefined4 *)(param_1 + 0x34),0);
          }
          local_8 = 0xffffffff;
          *(undefined4 *)(param_1 + 0x550) = local_9fc;
          if (*(int *)(param_1 + 0x550) != 0) {
            *(undefined4 *)(*(int *)(param_1 + 0x550) + 0xa4) = 0x19d;
            *(undefined4 *)(*(int *)(param_1 + 0x550) + 0xb8) = 0x5dc;
            if (*(char *)(param_1 + 0x79c) == '\x01') {
              uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x50),
                                   (*(float *)(&DAT_0092e7d4 + *(int *)(param_1 + 0x5c) * 8) *
                                    *(float *)(param_1 + 0x38) + *(float *)(param_1 + 0x54)) -
                                   _DAT_005a40ec,*(undefined4 *)(param_1 + 0x58));
              FUN_004310a0(uVar4);
            }
            else {
              uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                                   *(float *)(&DAT_0092e7d4 + *(int *)(param_1 + 0x5c) * 8) *
                                   *(float *)(param_1 + 0x38) + *(float *)(param_1 + 0x30) +
                                   _DAT_005a40ec,*(undefined4 *)(param_1 + 0x2c));
              FUN_004310a0(uVar4);
            }
            *(undefined4 *)(*(int *)(param_1 + 0x550) + 0x98) = 0x3f333333;
            *(undefined4 *)(*(int *)(param_1 + 0x550) + 0x9c) = 0x3f333333;
            *(undefined4 *)(*(int *)(param_1 + 0x550) + 0x6c) = 1;
            *(undefined1 *)(*(int *)(param_1 + 0x550) + 0xac) = 1;
            FUN_0054ac09(*(undefined4 *)(param_1 + 0x550));
          }
        }
        else {
          if (*(char *)(param_1 + 0x79c) == '\x01') {
            uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x50),
                                 (*(float *)(&DAT_0092e7d4 + *(int *)(param_1 + 0x5c) * 8) *
                                  *(float *)(param_1 + 0x38) + *(float *)(param_1 + 0x54)) -
                                 _DAT_005a40ec,*(undefined4 *)(param_1 + 0x58));
            FUN_004310a0(uVar4);
          }
          else {
            uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                                 *(float *)(&DAT_0092e7d4 + *(int *)(param_1 + 0x5c) * 8) *
                                 *(float *)(param_1 + 0x38) + *(float *)(param_1 + 0x30),
                                 *(undefined4 *)(param_1 + 0x2c));
            FUN_004310a0(uVar4);
          }
          *(undefined4 *)(*(int *)(param_1 + 0x550) + 0x8c) = *(undefined4 *)(param_1 + 0x34);
        }
      }
      else if ((*(char *)(param_1 + 0x798) == '\0') && (*(int *)(param_1 + 0x550) != 0)) {
        (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x550));
        *(undefined4 *)(param_1 + 0x550) = 0;
      }
      if (((DAT_0092e664 == 0) && (*(char *)(param_1 + 0x799) == '\x01')) &&
         (*(char *)(param_1 + 0x245) != '\x01')) {
        for (local_310 = 0; local_310 < 2; local_310 = local_310 + 1) {
          if (*(int *)(param_1 + 0x554 + local_310 * 4) == 0) {
            pvVar2 = operator_new(0xcc);
            local_8 = 0x41;
            if (pvVar2 == (void *)0x0) {
              local_a00 = 0;
            }
            else {
              uVar13 = 0;
              uVar10 = 1;
              uVar4 = 1;
              iVar3 = param_1;
              puVar5 = (undefined4 *)
                       FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                                    (float)local_310 * _DAT_005a4158 +
                                    *(float *)(param_1 + 0x30) + _DAT_005a415c,
                                    *(undefined4 *)(param_1 + 0x2c));
              local_a00 = FUN_004d3d20(*puVar5,puVar5[1],puVar5[2],uVar4,iVar3,uVar10,uVar13);
            }
            local_8 = 0xffffffff;
            *(undefined4 *)(param_1 + 0x554 + local_310 * 4) = local_a00;
            if (*(int *)(param_1 + 0x554 + local_310 * 4) != 0) {
              if (*(char *)(param_1 + 0x79c) == '\x01') {
                uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x50),
                                     ((*(float *)(&DAT_0092e7d4 + *(int *)(param_1 + 0x5c) * 8) *
                                      *(float *)(param_1 + 0x38)) / _DAT_005a365c +
                                     *(float *)(param_1 + 0x54)) - _DAT_005a40ec,
                                     *(undefined4 *)(param_1 + 0x58));
                FUN_004310a0(uVar4);
              }
              else {
                uVar4 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                                     (*(float *)(&DAT_0092e7d4 + *(int *)(param_1 + 0x5c) * 8) *
                                     *(float *)(param_1 + 0x38)) / _DAT_005a365c +
                                     *(float *)(param_1 + 0x30),*(undefined4 *)(param_1 + 0x2c));
                FUN_004310a0(uVar4);
              }
              uVar4 = __ftol();
              *(undefined4 *)(*(int *)(param_1 + 0x554 + local_310 * 4) + 0x90) = uVar4;
              *(undefined4 *)(*(int *)(param_1 + 0x554 + local_310 * 4) + 0xb4) = 0xb17;
              *(undefined4 *)(*(int *)(param_1 + 0x554 + local_310 * 4) + 0x6c) = 1;
              *(undefined4 *)(*(int *)(param_1 + 0x554 + local_310 * 4) + 0xb8) = 0x3ecccccd;
              FUN_004d40f3(0xff999999);
              FUN_0054ac09(*(undefined4 *)(param_1 + 0x554 + local_310 * 4));
            }
          }
        }
      }
      else {
        for (local_314 = 0; local_314 < 2; local_314 = local_314 + 1) {
          if (*(int *)(param_1 + 0x554 + local_314 * 4) != 0) {
            (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x554 + local_314 * 4));
            *(undefined4 *)(param_1 + 0x554 + local_314 * 4) = 0;
          }
        }
      }
      if ((*(short *)(param_1 + 0x7ae) == 0x2f1) && (*(int *)(param_1 + 0x560) == 0)) {
        local_318 = 3;
        FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                     (*(float *)(&DAT_0092e7d4 + *(int *)(param_1 + 0x5c) * 8) + _DAT_005a40ec) *
                     *(float *)(param_1 + 0x38),*(undefined4 *)(param_1 + 0x2c));
        pvVar2 = operator_new(0x22c);
        local_8 = 0x42;
        if (pvVar2 == (void *)0x0) {
          local_a0c = 0;
        }
        else {
          local_a0c = FUN_004d5480(0x14,local_324,local_320,local_31c,local_324,local_320,local_31c,
                                   local_318,param_1);
        }
        local_8 = 0xffffffff;
        *(undefined4 *)(param_1 + 0x560) = local_a0c;
        if (*(int *)(param_1 + 0x560) != 0) {
          *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x164) = 2;
          *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x160) = 2;
          *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x166) = 0;
          *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x162) = 0;
          (**(code **)(**(int **)(param_1 + 0x560) + 0x78))(0);
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x194) = 0;
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x560) + 0x60) + 0x24) = 0x3db851ec;
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x560) + 0x60) + 0x28) = 0x3db851ec;
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x560) + 0x60) + 0x2c) = 0x3db851ec;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x198) = 0;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x1a4) = 0x3f800000;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x1a8) = 0x3f800000;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x1ac) = 0x3f800000;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x1d4) = 1;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x1a0) = *(undefined4 *)(param_1 + 0x34);
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x19c) = 7;
          FUN_0054ac09(*(undefined4 *)(param_1 + 0x560));
        }
      }
      else if ((*(short *)(param_1 + 0x7ae) == 0x301) && (*(int *)(param_1 + 0x560) == 0)) {
        local_328 = 4;
        FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                     (*(float *)(&DAT_0092e7d4 + *(int *)(param_1 + 0x5c) * 8) + _DAT_005a40ec) *
                     *(float *)(param_1 + 0x38) + *(float *)(param_1 + 0x30),
                     *(undefined4 *)(param_1 + 0x2c));
        pvVar2 = operator_new(0x22c);
        local_8 = 0x43;
        if (pvVar2 == (void *)0x0) {
          local_a10 = 0;
        }
        else {
          local_a10 = FUN_004d5480(0x20,local_334,local_330,local_32c,local_334,local_330,local_32c,
                                   local_328,param_1);
        }
        local_8 = 0xffffffff;
        *(undefined4 *)(param_1 + 0x560) = local_a10;
        if (*(int *)(param_1 + 0x560) != 0) {
          *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x164) = 1;
          *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x160) = 1;
          *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x166) = 0;
          *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x162) = 0;
          (**(code **)(**(int **)(param_1 + 0x560) + 0x78))(0);
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x194) = 0;
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x560) + 0x60) + 0x24) = 0x3f99999a;
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x560) + 0x60) + 0x28) = 0x3f99999a;
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x560) + 0x60) + 0x2c) = 0x3f99999a;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x198) = 1;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x1a4) = 0x3f800000;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x1a8) = 0x3f800000;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x1ac) = 0x3f800000;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x1d4) = 1;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x1a0) = *(undefined4 *)(param_1 + 0x34);
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x19c) = 5;
          FUN_0054ac09(*(undefined4 *)(param_1 + 0x560));
        }
      }
      else if ((*(short *)(param_1 + 0x7ae) == 0x6be) && (*(int *)(param_1 + 0x560) == 0)) {
        local_338 = 5;
        FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                     (*(float *)(&DAT_0092e7d4 + *(int *)(param_1 + 0x5c) * 8) + _DAT_005a40ec) *
                     *(float *)(param_1 + 0x38) + *(float *)(param_1 + 0x30),
                     *(undefined4 *)(param_1 + 0x2c));
        pvVar2 = operator_new(0x22c);
        local_8 = 0x44;
        if (pvVar2 == (void *)0x0) {
          local_a14 = 0;
        }
        else {
          local_a14 = FUN_004d5480(0x20,local_344,local_340,local_33c,local_344,local_340,local_33c,
                                   local_338,param_1);
        }
        local_8 = 0xffffffff;
        *(undefined4 *)(param_1 + 0x560) = local_a14;
        if (*(int *)(param_1 + 0x560) != 0) {
          *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x164) = 2;
          *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x160) = 2;
          *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x166) = 0;
          *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x162) = 0;
          (**(code **)(**(int **)(param_1 + 0x560) + 0x78))(0);
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x194) = 0;
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x560) + 0x60) + 0x24) = 0x3f800000;
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x560) + 0x60) + 0x28) = 0x3f800000;
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x560) + 0x60) + 0x2c) = 0x3f800000;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x198) = 0;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x1a4) = 0x3f800000;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x1a8) = 0x3f800000;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x1ac) = 0x3f800000;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x1d4) = 1;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x1a0) = *(undefined4 *)(param_1 + 0x34);
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x19c) = 5;
          FUN_0054ac09(*(undefined4 *)(param_1 + 0x560));
        }
      }
      else if ((((*(short *)(param_1 + 0x7ae) < 0xf3c) || (0xf44 < *(short *)(param_1 + 0x7ae))) &&
               ((*(short *)(param_1 + 0x7ae) < 0xf47 || (0xf4b < *(short *)(param_1 + 0x7ae))))) ||
              (*(int *)(param_1 + 0x560) != 0)) {
        if ((((((*(short *)(param_1 + 0x7ae) != 0x2f1) && (*(short *)(param_1 + 0x7ae) != 0x301)) &&
              (*(short *)(param_1 + 0x7ae) != 0x6be)) &&
             ((*(short *)(param_1 + 0x7ae) < 0xf3c || (0xf44 < *(short *)(param_1 + 0x7ae))))) &&
            ((*(short *)(param_1 + 0x7ae) < 0xf47 || (0xf4b < *(short *)(param_1 + 0x7ae))))) &&
           (*(int *)(param_1 + 0x560) != 0)) {
          (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x560));
          *(undefined4 *)(param_1 + 0x560) = 0;
        }
      }
      else {
        local_348 = 6;
        FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                     (*(float *)(&DAT_0092e7d4 + *(int *)(param_1 + 0x5c) * 8) + _DAT_005a40ec) *
                     *(float *)(param_1 + 0x38) + *(float *)(param_1 + 0x30),
                     *(undefined4 *)(param_1 + 0x2c));
        pvVar2 = operator_new(0x22c);
        local_8 = 0x45;
        if (pvVar2 == (void *)0x0) {
          local_a18 = 0;
        }
        else {
          local_a18 = FUN_004d5480(0x20,local_354,local_350,local_34c,local_354,local_350,local_34c,
                                   local_348,param_1);
        }
        local_8 = 0xffffffff;
        *(undefined4 *)(param_1 + 0x560) = local_a18;
        if (*(int *)(param_1 + 0x560) != 0) {
          *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x164) = 3;
          *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x160) = 3;
          if (((*(short *)(param_1 + 0x7ae) == 0xf3c) || (*(short *)(param_1 + 0x7ae) == 0xf3f)) ||
             ((*(short *)(param_1 + 0x7ae) == 0xf42 ||
              ((0xf46 < *(short *)(param_1 + 0x7ae) && (*(short *)(param_1 + 0x7ae) < 0xf4a)))))) {
            *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x166) = 0;
            *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x162) = 0;
          }
          else if (((*(short *)(param_1 + 0x7ae) == 0xf3d) || (*(short *)(param_1 + 0x7ae) == 0xf40)
                   ) || (*(short *)(param_1 + 0x7ae) == 0xf43)) {
            *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x166) = 1;
            *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x162) = 1;
          }
          else if (((*(short *)(param_1 + 0x7ae) == 0xf3e) || (*(short *)(param_1 + 0x7ae) == 0xf41)
                   ) || (*(short *)(param_1 + 0x7ae) == 0xf44)) {
            *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x166) = 2;
            *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x162) = 2;
          }
          else if (*(short *)(param_1 + 0x7ae) == 0xf4a) {
            *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x166) = 3;
            *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x162) = 3;
          }
          else if (*(short *)(param_1 + 0x7ae) == 0xf4b) {
            *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x166) = 4;
            *(undefined2 *)(*(int *)(param_1 + 0x560) + 0x162) = 4;
          }
          (**(code **)(**(int **)(param_1 + 0x560) + 0x78))(0);
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x194) = 0;
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x560) + 0x60) + 0x24) = 0x3f800000;
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x560) + 0x60) + 0x28) = 0x3f800000;
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x560) + 0x60) + 0x2c) = 0x3f800000;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x198) = 1;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x1a4) = 0x3f800000;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x1a8) = 0x3f800000;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x1ac) = 0x3f800000;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x1d4) = 1;
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x1a0) = *(undefined4 *)(param_1 + 0x34);
          *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x19c) = 5;
          FUN_0054ac09(*(undefined4 *)(param_1 + 0x560));
        }
      }
      if ((0x5dc < param_2 - *(int *)(param_1 + 0x64c)) && (*(char *)(param_1 + 0x780) == '\x01')) {
        *(undefined1 *)(param_1 + 0x780) = 0;
      }
      if ((*(char *)(param_1 + 0x780) == '\x01') && (300 < param_2 - *(int *)(param_1 + 0x620))) {
        local_35c = 0;
        if ((*(int *)(param_1 + 0x24c) == 4) || (*(int *)(param_1 + 0x24c) == 8)) {
          local_35c = 1;
        }
        pvVar2 = operator_new(0x22c);
        local_8 = 0x46;
        if (pvVar2 == (void *)0x0) {
          local_a1c = (int *)0x0;
        }
        else {
          uVar12 = 0;
          uVar8 = 0;
          puVar5 = (undefined4 *)FUN_00430f50(0,0,0);
          uVar4 = *puVar5;
          uVar10 = puVar5[1];
          uVar13 = puVar5[2];
          puVar5 = (undefined4 *)FUN_00430f50(0,0,0);
          local_a1c = (int *)FUN_004d5480(*(undefined4 *)(param_1 + 0x5c),*puVar5,puVar5[1],
                                          puVar5[2],uVar4,uVar10,uVar13,uVar8,uVar12);
        }
        local_8 = 0xffffffff;
        local_358 = local_a1c;
        if (local_a1c != (int *)0x0) {
          if ((*(char *)(param_1 + 0x79c) < '\x01') || (*(int *)(param_1 + 0x19c) == 0)) {
            FUN_0058f220(local_a1c + 0x58,param_1 + 0x1d2,0x20);
          }
          else {
            local_a1c[0x17] = *(int *)(param_1 + 0x7a0);
            FUN_0058f220(local_a1c + 0x58,param_1 + 0x1a2,0x20);
            local_358[0x88] = *(int *)(param_1 + 0x5c);
            FUN_0058f220(local_358 + 0x7f,param_1 + 0x1d2,0x20);
          }
          local_358[0x75] = 1;
          local_358[0x69] = 0x3f000000;
          local_358[0x6a] = 0x3f000000;
          local_358[0x6b] = 0x3f000000;
          (**(code **)(*local_358 + 0x78))(local_35c);
          local_358[100] = param_2;
          local_358[0x65] = 700;
          (**(code **)(*local_358 + 0x3c))
                    (*(undefined4 *)(param_1 + 0x28),*(float *)(param_1 + 0x30) + _DAT_005a4214,
                     *(undefined4 *)(param_1 + 0x2c));
          local_358[0x68] = *(int *)(param_1 + 0x34);
          local_358[0x7d] = local_358[0x68];
          local_358[0x66] = 1;
          if ((*(char *)(param_1 + 0x79c) < '\x01') || (*(int *)(param_1 + 0x19c) == 0)) {
            FUN_004bd906((int)*(short *)(&DAT_005d0508 +
                                        *(int *)(&DAT_00672490 +
                                                *(int *)(param_1 + 0x740) * 4 +
                                                *(int *)(param_1 + 0x5c) * 0x2a0) * 2 +
                                        *(int *)(param_1 + 0x25c) * 0x70 +
                                        *(int *)(param_1 + 0x5c) * 0x1a40));
          }
          else {
            if (local_358[0x18] != 0) {
              FUN_004bd906((int)*(short *)(&DAT_005d0508 +
                                          *(int *)(&DAT_00672490 +
                                                  *(int *)(param_1 + 0x740) * 4 +
                                                  *(int *)(param_1 + 0x7a0) * 0x2a0) * 2 +
                                          *(int *)(param_1 + 0x25c) * 0x70 +
                                          *(int *)(param_1 + 0x5c) * 0x1a40));
            }
            if (local_358[0x87] != 0) {
              FUN_004bd906((int)*(short *)(&DAT_005d0508 +
                                          *(int *)(&DAT_006724f0 +
                                                  *(int *)(param_1 + 0x740) * 4 +
                                                  *(int *)(param_1 + 0x5c) * 0x2a0) * 2 +
                                          *(int *)(param_1 + 0x25c) * 0x70 +
                                          *(int *)(param_1 + 0x5c) * 0x1a40));
            }
          }
          *(undefined4 *)(local_358[0x18] + 0x2e4) =
               *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2e4);
          local_358[0x67] = 0;
          local_360 = 0xa0;
          if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0xa0), iVar3 != 0)) {
            FUN_00429a6d(local_360);
            iVar3 = FUN_0042afd0();
            if (iVar3 == 0) {
              uVar10 = 0;
              uVar4 = 0;
              FUN_00429a6d(local_360);
              FUN_0042ad2b(uVar4,uVar10);
            }
          }
          FUN_0054ac09(local_358);
        }
        *(uint *)(param_1 + 0x620) = param_2;
      }
      if ((*(short *)(param_1 + 0x4d0) == 0x66) && (*(int *)(param_1 + 0x7dc) == 0)) {
        local_364 = (int)*(short *)(param_1 + 0x250) % 10;
        if (local_364 == 6) {
          FUN_00510fd3();
        }
        else if (local_364 == 7) {
          FUN_0051148a();
        }
        else if ((local_364 == 8) ||
                (((0x15 < *(short *)(param_1 + 0x250) && (*(short *)(param_1 + 0x250) < 0x1a)) ||
                 (*(short *)(param_1 + 0x250) == 0x20)))) {
          FUN_00511ec4();
        }
        else if (local_364 == 9) {
          FUN_0051253c();
        }
        *(undefined4 *)(param_1 + 0x7dc) = 1;
        uVar4 = (**(code **)(*DAT_0092e654 + 8))();
        *(undefined4 *)(param_1 + 0x7d8) = uVar4;
      }
      if (((*(int *)(param_1 + 0x7dc) == 1) && (*(char *)(param_1 + 0x79a) == '\x01')) &&
         (*(int *)(param_1 + 0x7d8) + 0x4e2U < param_2)) {
        CSimpleArray<>();
        local_378 = *(undefined4 *)(param_1 + 0x28);
        local_374 = *(float *)(param_1 + 0x30) + _DAT_005a365c;
        local_370 = *(undefined4 *)(param_1 + 0x2c);
        local_368 = 0xffffffff;
        for (local_36c = 0; local_36c < 3; local_36c = local_36c + 1) {
          pvVar2 = operator_new(0x180);
          local_8 = 0x47;
          if (pvVar2 == (void *)0x0) {
            local_a20 = (int *)0x0;
          }
          else {
            local_a20 = (int *)FUN_004c8f70(0x38,700,(float)local_36c * _DAT_005a430c +
                                                     _DAT_005a40f0,
                                            (float)local_36c * _DAT_005a430c + _DAT_005a40f0,
                                            (float)local_36c * _DAT_005a34a0 + _DAT_005a40f0,
                                            0x3a83126f,1,0x50);
          }
          local_8 = 0xffffffff;
          if (local_a20 == (int *)0x0) break;
          (**(code **)(*local_a20 + 0x3c))(local_368);
          local_a20[0x1b] = 1;
          local_a20[0x41] = 1;
          FUN_004310a0(&local_378);
          FUN_0054ac09(local_a20);
        }
        *(undefined4 *)(param_1 + 0x7dc) = 0;
      }
    }
    else {
      if (*(int *)(param_1 + 0x51c) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x51c) + 0x24) = 0;
      }
      if (*(int *)(param_1 + 0x520) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x520) + 0x24) = 0;
      }
      if (*(int *)(param_1 + 0x524) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x524) + 0x24) = 0;
      }
      if (*(int *)(param_1 + 0x514) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x514) + 0x24) = 0;
      }
      if (*(int *)(param_1 + 0x518) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x518) + 0x24) = 0;
      }
      if (*(int *)(param_1 + 0x528) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x528) + 0x24) = 0;
      }
      if (*(int *)(param_1 + 0x52c) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x52c) + 0x24) = 0;
      }
      if (*(int *)(param_1 + 0x538) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x538) + 0x24) = 0;
      }
      if (*(int *)(param_1 + 0x53c) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x53c) + 0x24) = 0;
      }
      if (*(int *)(param_1 + 0x540) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x540) + 0x24) = 0;
      }
      if (*(int *)(param_1 + 0x544) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x544) + 0x24) = 0;
      }
      if (*(int *)(param_1 + 0x548) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x548) + 0x24) = 0;
      }
      if (*(int *)(param_1 + 0x54c) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x54c) + 0x24) = 0;
      }
      if (*(int *)(param_1 + 0x550) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x550) + 0x24) = 0;
      }
      if (*(int *)(param_1 + 0x554) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x554) + 0x24) = 0;
      }
      if (*(int *)(param_1 + 0x558) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x558) + 0x24) = 0;
      }
      if (*(int *)(param_1 + 0x55c) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x55c) + 0x48) = 0;
      }
      if (*(int *)(param_1 + 0x560) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x560) + 0x48) = 0;
      }
      for (local_18 = 0; local_18 < 7; local_18 = local_18 + 1) {
        if (*(int *)(param_1 + 0x56c + local_18 * 4) != 0) {
          *(undefined4 *)(*(int *)(param_1 + 0x56c + local_18 * 4) + 0x24) = 0;
        }
        if (*(int *)(param_1 + 0x588 + local_18 * 4) != 0) {
          *(undefined4 *)(*(int *)(param_1 + 0x588 + local_18 * 4) + 0x24) = 0;
        }
        if (*(int *)(param_1 + 0x5a4 + local_18 * 4) != 0) {
          *(undefined4 *)(*(int *)(param_1 + 0x5a4 + local_18 * 4) + 0x24) = 0;
        }
      }
      for (local_18 = 0; local_18 < 4; local_18 = local_18 + 1) {
        if (*(int *)(param_1 + 0x5c0 + local_18 * 4) != 0) {
          *(undefined4 *)(*(int *)(param_1 + 0x5c0 + local_18 * 4) + 0x48) = 0;
        }
      }
      for (local_18 = 0; local_18 < 5; local_18 = local_18 + 1) {
        if (*(int *)(param_1 + 0x5d0 + local_18 * 4) != 0) {
          *(undefined4 *)(*(int *)(param_1 + 0x5d0 + local_18 * 4) + 0x24) = 0;
        }
      }
      if (*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) {
        *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f8) = 0;
        *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f0) = 0;
        *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f3) = 0;
        *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f1) = 0;
        *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f5) = 0;
        *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f2) = 0;
        *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f4) = 0;
      }
      if (*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0) {
        *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f8) = 0;
        *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f0) = 0;
        *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f3) = 0;
        *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f1) = 0;
        *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f5) = 0;
        *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f2) = 0;
        *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f4) = 0;
      }
    }
  }
  ExceptionList = local_10;
  return;
}

