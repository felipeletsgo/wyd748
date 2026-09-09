// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00489a3e | Name: FUN_00489a3e


/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_00489a3e(int param_1)

{
  longlong lVar1;
  bool bVar2;
  uint uVar3;
  size_t sVar4;
  int iVar5;
  float fVar6;
  void *pvVar7;
  undefined4 uVar8;
  float *pfVar9;
  int iVar10;
  int extraout_ECX;
  uint uVar11;
  int unaff_EDI;
  float10 fVar12;
  int local_1014;
  int local_1008;
  int local_1000;
  int local_ff8;
  int local_ff0;
  int local_fe8;
  int local_fd8;
  int local_fd0;
  int local_fcc;
  int local_fc8;
  int local_fc4;
  int local_fb8;
  int local_fb4;
  int local_fa0;
  int local_f9c;
  int local_f98;
  int local_f94;
  int local_f90;
  int local_f88;
  int local_f80;
  int local_f6c;
  int local_f64;
  int local_f40;
  int local_f3c;
  int local_f38;
  int local_f34;
  int *local_f30;
  int *local_f2c;
  int local_f28;
  int local_f24;
  int local_f20;
  int local_f1c;
  int *local_f18;
  int local_f14;
  int *local_f10;
  int local_f0c;
  int local_f08;
  int local_f04;
  int local_ef8;
  int *local_ee4;
  int local_ed4;
  int local_ed0;
  int local_eb8;
  int local_eac;
  int local_e94;
  int local_e90;
  int local_e8c;
  int local_e88;
  int local_e7c;
  int *local_e5c;
  int *local_e58;
  int *local_e54;
  int local_e50;
  int local_e38;
  int local_e34;
  int local_e30;
  int local_e2c;
  int local_93c;
  int local_934;
  uint local_920 [32];
  int local_8a0;
  int local_89c;
  int local_898;
  int local_894;
  uint *local_88c;
  int local_888;
  uint local_884 [32];
  uint local_804;
  int local_800;
  uint *local_7fc;
  int local_7f8;
  uint local_7f4 [32];
  uint local_774;
  int local_770;
  uint *local_76c;
  int local_768;
  uint local_764;
  int local_760 [37];
  int local_6cc;
  int local_6c8;
  int local_6c4;
  int local_6c0;
  int local_6bc;
  int local_6b8;
  int local_6b4;
  uint local_6b0;
  uint local_6ac;
  uint *local_6a8;
  uint *local_6a4;
  uint *local_6a0;
  int local_69c;
  int local_698;
  uint *local_694;
  uint *local_690;
  uint *local_68c;
  float local_688;
  uint *local_684;
  uint *local_680;
  uint local_67c;
  int local_678;
  int local_674;
  int local_670;
  float local_66c;
  uint *local_668;
  float local_664;
  uint *local_660;
  int local_65c;
  int local_658;
  int local_654;
  uint *local_650;
  float local_64c;
  uint *local_648;
  int local_644;
  int local_640;
  int local_63c;
  int local_638;
  uint *local_634;
  uint local_630;
  uint *local_62c;
  int local_628;
  uint local_624 [32];
  int local_5a4;
  int local_5a0;
  int local_598;
  int local_594;
  uint local_590 [32];
  uint local_510;
  int local_50c;
  uint *local_508;
  int local_504;
  uint local_500 [32];
  uint local_480;
  int local_47c;
  uint *local_478;
  int local_474;
  int local_470;
  int local_46c;
  float local_468;
  float local_464;
  int local_460;
  int local_45c;
  int local_458;
  int local_454;
  uint *local_450;
  float local_44c;
  float local_448;
  float local_444;
  uint local_434;
  float local_430;
  uint local_42c;
  int local_41c;
  int local_418;
  int local_414;
  int local_410;
  int local_40c;
  uint *local_408;
  uint *local_404;
  uint *local_400;
  float local_3f0;
  float local_3ec;
  float local_3e8;
  int local_3d8;
  int *local_3d4;
  uint *local_3b8;
  uint *local_3b4;
  int local_3b0;
  int local_3ac;
  uint local_3a8;
  uint local_3a4;
  undefined1 local_3a0 [4];
  undefined2 local_39c;
  undefined2 uStackY_39a;
  short local_394;
  short local_392;
  undefined4 local_390;
  undefined4 local_38c;
  short local_388;
  short sStackY_386;
  char local_36c [48];
  int local_33c;
  int local_338;
  int *local_334;
  int local_330;
  int local_32c;
  uint local_328;
  int local_324;
  int local_320;
  int local_31c;
  int local_318;
  int local_314;
  int local_310;
  int local_30c;
  undefined4 local_308;
  int local_304;
  uint *local_300;
  float local_2fc;
  float local_2f8;
  uint *local_2f4;
  uint *local_2f0;
  uint *local_2ec;
  uint local_2e8;
  float local_2e4;
  uint local_2e0;
  float local_2dc;
  int local_2d8;
  uint local_2d4;
  float local_2d0;
  uint local_2cc;
  float local_2c8;
  int local_2c4;
  uint local_2c0;
  int local_2bc;
  int *local_2b8;
  int local_2b4;
  undefined4 local_2b0;
  undefined4 local_2ac;
  int *local_2a8;
  uint *local_2a4;
  uint *local_2a0;
  uint *local_29c;
  int local_290;
  uint *local_28c;
  uint *local_288;
  uint *local_284;
  int local_278;
  int local_274;
  int local_270;
  int local_26c;
  uint *local_268;
  uint *local_264;
  uint *local_260;
  int local_254;
  uint *local_250;
  uint *local_24c;
  uint *local_248;
  int local_244;
  int local_240;
  int local_23c;
  int local_238;
  float local_234;
  uint local_230;
  uint *local_22c;
  int local_228;
  int local_21c;
  int local_218;
  float local_214;
  float local_210;
  float local_20c;
  int local_208;
  int *local_204;
  uint local_200;
  int local_1fc;
  int local_1f8;
  int local_1f4;
  float local_1f0;
  uint *local_1ec;
  uint *local_1e8;
  int local_1e0;
  undefined4 local_1dc;
  uint *local_1d8;
  uint *local_1d4;
  undefined4 local_1cc;
  int local_1c8;
  float local_1c4;
  uint *local_1b4;
  uint *local_1b0;
  uint *local_1ac;
  int local_19c;
  int local_198;
  int local_194;
  undefined4 local_190;
  uint *local_18c;
  uint *local_188;
  uint *local_184;
  uint *local_180;
  uint *local_17c;
  int local_174;
  int local_170;
  uint local_16c;
  float local_168;
  uint local_164;
  uint local_160;
  uint *local_150;
  int local_14c;
  float local_148;
  int *local_144;
  float local_140;
  undefined4 local_13c;
  int local_134;
  int aiStackY_130 [6];
  int *local_118;
  int *local_114;
  int local_110;
  int local_10c;
  undefined4 local_108;
  int local_104;
  uint *local_100;
  uint *local_fc;
  int local_f4;
  uint *local_f0;
  float local_ec;
  int local_e4;
  uint local_e0;
  float local_dc;
  uint local_d8;
  int local_d4;
  undefined4 local_d0;
  int local_cc;
  uint *local_c8;
  uint *local_c4;
  undefined4 local_bc;
  uint *local_b8;
  uint *local_b4;
  uint *local_b0;
  float local_a0;
  float local_9c;
  int *local_78;
  int *local_74;
  undefined4 local_70;
  int local_6c;
  undefined1 local_68 [4];
  undefined2 local_64;
  undefined2 local_62;
  undefined4 local_5c;
  int local_58;
  int local_54;
  undefined4 local_50;
  uint local_4c;
  float local_48;
  uint *local_44;
  uint *local_40;
  uint *local_3c;
  uint *local_38;
  uint *local_34;
  uint *local_30;
  uint *local_2c;
  char *pcVar13;
  uint *puVar14;
  uint *local_1c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059fee6;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0058f730();
  local_30 = (uint *)0x0;
  FUN_00430f10();
  local_40 = (uint *)(**(code **)(*DAT_013b71e8 + 0x34))();
  local_1c = (uint *)(uint)*(ushort *)(param_1 + 0x2c);
  local_2c = (uint *)(**(code **)(*DAT_013b71e8 + 0x34))();
  if (local_40 == *(uint **)(extraout_ECX + 0x4c)) {
    local_50 = 0;
  }
  if (local_40 == (uint *)0x0) {
    local_54 = 0;
    local_58 = 0;
    while( true ) {
      if (((0xc < local_58) || ((*(short *)(param_1 + 4) == 0x39d && (0 < local_58)))) ||
         ((*(short *)(param_1 + 4) == 0x39e && (1 < local_58)))) goto LAB_00489b54;
      if ((uint)*(ushort *)(param_1 + 0x2c + local_58 * 4) ==
          *(uint *)(*(int *)(extraout_ECX + 0x4c) + 0x20)) break;
      local_58 = local_58 + 1;
    }
    local_54 = 1;
LAB_00489b54:
    if ((local_54 == 1) ||
       ((0x37 < *(short *)(param_1 + 0x18) && (0x3f < *(short *)(param_1 + 0x18))))) {
      FUN_00430f20();
      FUN_0040bd30();
      local_2c = (uint *)0x489bd9;
      _memset(local_68,0,0x10);
      local_62 = (undefined2)DAT_013b71e8[0x317];
      local_64 = 0x369;
      local_5c = CONCAT22(local_5c._2_2_,*(undefined2 *)(param_1 + 0xc));
      FUN_0055f2dd();
    }
  }
  else {
    if ((*(byte *)(param_1 + 0x1f) & 8) != 0) {
      local_70 = (**(code **)(*DAT_0092e654 + 8))();
      if (*(int *)(local_40[0x18] + 0x2f8) != 0) {
        if (((local_40[0x17] == 3) || (local_40[0x17] == 8)) || (local_40[0x93] == 0x28)) {
          *(undefined1 *)(*(int *)(local_40[0x18] + 0x2f8) + 0x28f7) = 5;
        }
        else if (local_40[0x58] == 0x65) {
          *(undefined1 *)(*(int *)(local_40[0x18] + 0x2f8) + 0x28f7) = 2;
        }
        else if (local_40[0x58] == 0x29) {
          *(undefined1 *)(*(int *)(local_40[0x18] + 0x2f8) + 0x28f7) = 3;
        }
        else if (local_40[0x58] == 0x67) {
          *(undefined1 *)(*(int *)(local_40[0x18] + 0x2f8) + 0x28f7) = 4;
        }
        else {
          *(undefined1 *)(*(int *)(local_40[0x18] + 0x2f8) + 0x28f7) = 1;
        }
        *(undefined4 *)(*(int *)(local_40[0x18] + 0x2f8) + 0x80) = local_70;
      }
      if (*(int *)(local_40[0x18] + 0x2fc) != 0) {
        if (((local_40[0x17] == 3) || (local_40[0x17] == 8)) || (local_40[0x93] == 0x28)) {
          *(undefined1 *)(*(int *)(local_40[0x18] + 0x2fc) + 0x28f7) = 5;
        }
        else if (local_40[0x58] == 0x65) {
          *(undefined1 *)(*(int *)(local_40[0x18] + 0x2fc) + 0x28f7) = 2;
        }
        else if (local_40[0x58] == 0x29) {
          *(undefined1 *)(*(int *)(local_40[0x18] + 0x2fc) + 0x28f7) = 3;
        }
        else if (local_40[0x58] == 0x67) {
          *(undefined1 *)(*(int *)(local_40[0x18] + 0x2fc) + 0x28f7) = 4;
        }
        else {
          *(undefined1 *)(*(int *)(local_40[0x18] + 0x2fc) + 0x28f7) = 1;
        }
        *(undefined4 *)(*(int *)(local_40[0x18] + 0x2fc) + 0x80) = local_70;
      }
    }
    if (((local_40[0x93] == 0x20) && (*(char *)(param_1 + 0x1c) == '\x04')) &&
       (local_2c == *(uint **)(extraout_ECX + 0x4c))) {
      uVar11 = (**(code **)(*DAT_0092e654 + 8))();
      local_40[399] = uVar11;
    }
    for (local_6c = 0;
        ((local_6c < 0xd && ((*(short *)(param_1 + 4) != 0x39d || (local_6c < 1)))) &&
        ((*(short *)(param_1 + 4) != 0x39e || (local_6c < 2)))); local_6c = local_6c + 1) {
      *(undefined2 *)((int)local_40 + local_6c * 2 + 0x652) =
           *(undefined2 *)(param_1 + 0x2c + local_6c * 4);
    }
    FUN_00480c5d();
    FUN_0040bd30();
    if ((local_40[0x17] == 0x14) && (*(short *)((int)local_40 + 0x1d6) == 0)) {
      local_30 = (uint *)0x1;
    }
    if ((((local_40 != *(uint **)(extraout_ECX + 0x4c)) ||
         ((*(char *)(param_1 + 0x1e) == '\0' && (local_40 == *(uint **)(extraout_ECX + 0x4c))))) &&
        (-1 < *(short *)(param_1 + 0x18))) &&
       ((*(short *)(param_1 + 0x18) < 0x68 &&
        (*(undefined2 *)((int)local_40 + 0x452) = *(undefined2 *)(param_1 + 0x1a),
        local_40 == *(uint **)(extraout_ECX + 0x4c))))) {
      *(undefined2 *)((int)DAT_013b71e8 + 0x73a) = *(undefined2 *)(param_1 + 0x1a);
      local_78 = (int *)(**(code **)(**(int **)(extraout_ECX + 0x28) + 0x48))();
      local_74 = (int *)(**(code **)(**(int **)(extraout_ECX + 0x28) + 0x48))();
      if (local_78 != (int *)0x0) {
        (**(code **)(*local_78 + 0x88))();
      }
      if (local_74 != (int *)0x0) {
        local_2c = (uint *)0x489fc8;
        FUN_0058f078();
        (**(code **)(*local_74 + 0x80))();
      }
    }
    if (((local_40 != *(uint **)(extraout_ECX + 0x4c)) ||
        ((*(char *)(param_1 + 0x1e) == '\x01' && (local_40 == *(uint **)(extraout_ECX + 0x4c))))) ||
       ((*(char *)(param_1 + 0x1e) == '\0' &&
        ((local_40 == *(uint **)(extraout_ECX + 0x4c) && (*(char *)(param_1 + 0x1c) == -2)))))) {
      if (*(short *)(param_1 + 0x18) == 4) {
        *(undefined1 *)(local_40 + 0x1e0) = 1;
        uVar11 = (**(code **)(*DAT_0092e654 + 8))();
        local_40[0x193] = uVar11;
      }
      local_9c = (float)local_40[0x8b];
      if (local_2c == (uint *)0x0) {
        local_2c = (uint *)0x48a0c9;
        FUN_00430f20();
        local_2c = (uint *)(uint)*(byte *)(param_1 + 0x1c);
        local_30 = (uint *)0x48a0e1;
        FUN_00522126();
      }
      else if (*(char *)(param_1 + 0x1c) != -2) {
        local_2c = (uint *)0x48a10e;
        FUN_00521d53();
        if (local_40 != local_2c) {
          fVar12 = (float10)FUN_00493c30();
          local_9c = (float)(fVar12 + (float10)_DAT_005a4380);
        }
      }
      if (*(short *)(param_1 + 0x18) == 0x62) {
        fVar12 = (float10)FUN_00493c30();
        local_9c = (float)(fVar12 + (float10)_DAT_005a4380);
      }
      if ((*(byte *)(param_1 + 0x1f) & 1) != 0) {
        local_40[0x1d9] = 1;
      }
      if (local_40[0x93] != 0x2c) {
        FUN_00500e75();
      }
      if (((((local_2c != (uint *)0x0) && (local_2c != local_40)) &&
           (local_2c != *(uint **)(extraout_ECX + 0x4c))) &&
          ((local_2c[0x93] != 0x38 || (*(short *)((int)local_2c + 0x1d2) != 0)))) &&
         (*(short *)(param_1 + 0x18) != 0x1b)) {
        FUN_00500e75();
      }
      if ((((-1 < *(short *)(param_1 + 0x18)) && (*(short *)(param_1 + 0x18) < 0x68)) ||
          ((0x96 < *(short *)(param_1 + 0x18) && (*(short *)(param_1 + 0x18) < 0x9c)))) ||
         (((*(short *)(param_1 + 0x18) == 0x68 || (*(short *)(param_1 + 0x18) == 0x69)) ||
          (*(short *)(param_1 + 0x18) == 0x6f)))) {
        *(undefined2 *)(local_40 + 0x134) = *(undefined2 *)(param_1 + 0x18);
        *(ushort *)((int)local_40 + 0x4d2) = (ushort)*(byte *)(param_1 + 0x1d);
        if (*(char *)(param_1 + 0x1c) == -2) {
          *(undefined2 *)((int)local_40 + 0x4d2) = 1;
        }
        if (local_2c == (uint *)0x0) {
          FUN_00430f20();
          iVar5 = FUN_0049c70f();
          local_a0 = (float)iVar5 * _DAT_005a4214;
          local_2c = (uint *)0x48a39a;
          FUN_00430f50();
          FUN_004310a0();
          local_40[0x133] = 0;
        }
        else {
          local_40[0x133] = (uint)local_2c;
          if (((local_2c == (uint *)0x0) || (local_40 == (uint *)0x0)) ||
             ((local_2c[0x93] != 0x38 || (*(short *)((int)local_2c + 0x1d2) != 0)))) {
            local_2c = (uint *)0x48a4a0;
            FUN_00430f50();
            FUN_004310a0();
          }
          else {
            local_2c = (uint *)0x48a431;
            FUN_00430f50();
            local_2c = (uint *)0x48a46f;
            FUN_00430f50();
            FUN_004310a0();
          }
        }
        if (((*(short *)(param_1 + 0x18) < 0x97) || (0x99 < *(short *)(param_1 + 0x18))) &&
           ((*(short *)(param_1 + 0x18) != 0x68 && (*(short *)(param_1 + 0x18) != 0x69)))) {
          iVar5 = (**(code **)(*DAT_0092e654 + 8))();
          local_40[0x12f] = iVar5 + 500;
        }
        else {
          iVar5 = (**(code **)(*DAT_0092e654 + 8))();
          local_40[0x12f] = iVar5 + 200;
        }
      }
      if ((local_40[0x93] == 0x3e) && (*(short *)((int)local_40 + 0x1d2) == 2)) {
        local_2c = (uint *)0x48a572;
        FUN_00430f50();
        local_2c = (uint *)0x48a592;
        FUN_00430f50();
        local_c4 = (uint *)((float)local_c4 + _DAT_005a3660);
        local_b4 = (uint *)((float)local_b4 + _DAT_005a3660);
        local_bc = 5;
        pvVar7 = operator_new(0xc0);
        local_8 = 0;
        if (pvVar7 == (void *)0x0) {
          local_e2c = 0;
        }
        else {
          local_30 = local_c8;
          local_2c = local_c4;
          local_3c = local_b8;
          local_38 = local_b4;
          local_34 = local_b0;
          local_40 = (uint *)0x48a635;
          local_e2c = FUN_004e7610();
        }
        local_8 = 0xffffffff;
        local_cc = local_e2c;
        if (local_e2c != 0) {
          FUN_0054ac09();
        }
      }
      if (*(short *)(param_1 + 0x18) == 6) {
        local_2c = (uint *)0x48a6bd;
        FUN_00430f50();
        pvVar7 = operator_new(0xdc);
        local_8 = 1;
        if (pvVar7 == (void *)0x0) {
          local_e30 = 0;
        }
        else {
          local_2c = (uint *)0xff222299;
          local_30 = (uint *)0xff5555ff;
          local_34 = (uint *)0x0;
          local_38 = (uint *)0x0;
          local_3c = (uint *)0x0;
          local_40 = (uint *)0x48a704;
          pfVar9 = (float *)FUN_00430f50();
          local_3c = (uint *)*pfVar9;
          local_38 = (uint *)pfVar9[1];
          local_34 = (uint *)pfVar9[2];
          local_40 = local_2c;
          local_4c = local_e0;
          local_48 = local_dc;
          local_44 = (uint *)local_d8;
          local_50 = 0x48a747;
          local_e30 = FUN_004d8020();
        }
        local_8 = 0xffffffff;
        local_d4 = local_e30;
        if (local_e30 != 0) {
          FUN_0054ac09();
        }
        local_d0 = (**(code **)(*DAT_0092e654 + 8))();
        if (local_2c == *(uint **)(extraout_ECX + 0x4c)) {
          *(undefined4 *)(extraout_ECX + 0x28e58) = 1;
          *(undefined4 *)(extraout_ECX + 0x28e5c) = local_d0;
        }
      }
      else if (*(short *)(param_1 + 0x18) == 0xd) {
        if (local_40 != (uint *)0x0) {
          local_2c = (uint *)0x48a825;
          FUN_00430f50();
          local_ec = local_ec - _DAT_005a34a0;
          pvVar7 = operator_new(0x90);
          local_8 = 2;
          if (pvVar7 == (void *)0x0) {
            local_e34 = 0;
          }
          else {
            local_2c = local_f0;
            local_30 = (uint *)0x48a886;
            local_e34 = FUN_004e5650();
          }
          local_8 = 0xffffffff;
          local_e4 = local_e34;
          if ((*(int *)(extraout_ECX + 0x2c) != 0) && (local_e34 != 0)) {
            FUN_0054ac09();
          }
        }
      }
      else if (*(short *)(param_1 + 0x18) == 5) {
        if (local_40 != (uint *)0x0) {
          local_2c = (uint *)0x48a929;
          FUN_00430f50();
          local_fc = (uint *)((float)local_fc - _DAT_005a34a0);
          pvVar7 = operator_new(0x94);
          local_8 = 3;
          if (pvVar7 == (void *)0x0) {
            local_e38 = 0;
          }
          else {
            local_30 = local_100;
            local_2c = local_fc;
            local_34 = (uint *)0x48a98c;
            local_e38 = FUN_004d8ef0();
          }
          local_8 = 0xffffffff;
          local_f4 = local_e38;
          if ((*(int *)(extraout_ECX + 0x2c) != 0) && (local_e38 != 0)) {
            FUN_0054ac09();
          }
          if ((DAT_005ccf98 != 0) && (iVar5 = FUN_00429a6d(), iVar5 != 0)) {
            local_2c = (uint *)0x48aa18;
            FUN_00429a6d();
            FUN_0042ad2b();
          }
        }
      }
      else if (*(short *)(param_1 + 0x18) == 0x2d) {
        local_2c = (uint *)0x48aa9c;
        FUN_00430f20();
        local_2c = (uint *)0x48aaae;
        FUN_0049c70f();
        local_2c = (uint *)0x48aaee;
        FUN_00430f50();
        if (local_2c != (uint *)0x0) {
          local_2c = (uint *)0x48ab1d;
          FUN_00430f50();
          FUN_004310a0();
        }
        pvVar7 = operator_new(0x90);
        local_8 = 4;
        if (pvVar7 == (void *)0x0) {
          local_e50 = 0;
        }
        else {
          local_2c = (uint *)0x0;
          local_30 = (uint *)0x48ab62;
          pfVar9 = (float *)FUN_00430f50();
          local_2c = (uint *)*pfVar9;
          local_30 = (uint *)0x1;
          local_34 = (uint *)0x3f800000;
          local_38 = (uint *)0x38;
          local_3c = (uint *)0x1;
          local_40 = (uint *)0x0;
          local_44 = (uint *)0x3dcccccd;
          local_48 = 2.8026e-44;
          local_4c = 0;
          local_58 = local_110;
          local_54 = local_10c;
          local_50 = local_108;
          local_5c = 0x48abb7;
          local_e50 = FUN_004d4de0();
        }
        local_8 = 0xffffffff;
        local_104 = local_e50;
        if ((*(int *)(extraout_ECX + 0x2c) != 0) && (local_e50 != 0)) {
          FUN_0054ac09();
        }
        if ((DAT_005ccf98 != 0) && (iVar5 = FUN_00429a6d(), iVar5 != 0)) {
          local_2c = (uint *)0x48ac43;
          FUN_00429a6d();
          FUN_0042ad2b();
        }
      }
      else if ((*(short *)(param_1 + 0x18) == 0x22) && (*(char *)(param_1 + 0x1c) == -2)) {
        if ((local_40 != (uint *)0x0) && (local_40[0x158] != 0)) {
          pvVar7 = operator_new(0x180);
          local_8 = 5;
          if (pvVar7 == (void *)0x0) {
            local_e54 = (int *)0x0;
          }
          else {
            local_2c = (uint *)0x3dcccccd;
            local_30 = (uint *)0x3dcccccd;
            local_34 = (uint *)0x3dcccccd;
            local_38 = (uint *)0x2bc;
            local_3c = (uint *)0x1;
            local_40 = (uint *)0x48acda;
            local_e54 = (int *)FUN_004c8f70();
          }
          local_8 = 0xffffffff;
          local_114 = local_e54;
          if (local_e54 != (int *)0x0) {
            local_e54[0x1b] = 1;
            local_2c = (uint *)0x48ad60;
            FUN_00430f50();
            FUN_004310a0();
            FUN_004310a0();
            (**(code **)(*local_114 + 0x3c))();
            FUN_0054ac09();
          }
        }
      }
      else if ((*(short *)(param_1 + 0x18) == 0x20) && (*(char *)(param_1 + 0x1c) == -2)) {
        if ((local_40 != (uint *)0x0) && (local_40[0x158] != 0)) {
          pvVar7 = operator_new(0x180);
          local_8 = 6;
          if (pvVar7 == (void *)0x0) {
            local_e58 = (int *)0x0;
          }
          else {
            local_2c = (uint *)0x3dcccccd;
            local_30 = (uint *)0x3dcccccd;
            local_34 = (uint *)0x3dcccccd;
            local_38 = (uint *)0x2bc;
            local_3c = (uint *)0x1;
            local_40 = (uint *)0x48ae3c;
            local_e58 = (int *)FUN_004c8f70();
          }
          local_8 = 0xffffffff;
          local_118 = local_e58;
          if (local_e58 != (int *)0x0) {
            local_e58[0x1b] = 1;
            local_2c = (uint *)0x48aec2;
            FUN_00430f50();
            FUN_004310a0();
            FUN_004310a0();
            (**(code **)(*local_118 + 0x3c))();
            FUN_0054ac09();
          }
        }
      }
      else if (*(short *)(param_1 + 0x18) == 0x24) {
        if (((*(char *)(param_1 + 0x1c) == -2) && (local_40 != (uint *)0x0)) &&
           (local_40[0x158] != 0)) {
          pvVar7 = operator_new(0x180);
          local_8 = 7;
          if (pvVar7 == (void *)0x0) {
            local_e5c = (int *)0x0;
          }
          else {
            local_2c = (uint *)0x3dcccccd;
            local_30 = (uint *)0x3dcccccd;
            local_34 = (uint *)0x3dcccccd;
            local_38 = (uint *)0x2bc;
            local_3c = (uint *)0x1;
            local_40 = (uint *)0x48af9e;
            local_e5c = (int *)FUN_004c8f70();
          }
          local_8 = 0xffffffff;
          local_144 = local_e5c;
          if (local_e5c != (int *)0x0) {
            local_e5c[0x1b] = 1;
            local_2c = (uint *)0x48b024;
            FUN_00430f50();
            FUN_004310a0();
            FUN_004310a0();
            (**(code **)(*local_144 + 0x3c))();
            FUN_0054ac09();
          }
        }
        CSimpleArray<>();
        if (local_2c == (uint *)0x0) {
          FUN_00430f20();
          iVar5 = FUN_0049c70f();
          local_148 = (float)iVar5 * _DAT_005a4214;
          local_2c = (uint *)0x48b148;
          FUN_00430f50();
          FUN_004310a0();
        }
        else {
          local_2c = (uint *)0x48b176;
          FUN_00430f50();
          FUN_004310a0();
        }
        for (local_134 = 0; local_134 < 6; local_134 = local_134 + 1) {
          pvVar7 = operator_new(0xc0);
          local_8 = 8;
          if (pvVar7 == (void *)0x0) {
            local_e7c = 0;
          }
          else {
            local_2c = (uint *)local_13c;
            local_30 = (uint *)((float)(local_134 % 3) * _DAT_005a40ec + local_140);
            local_34 = (uint *)0x48b23e;
            pfVar9 = (float *)FUN_00430f50();
            local_30 = (uint *)*pfVar9;
            local_2c = (uint *)pfVar9[1];
            local_34 = (uint *)0x0;
            local_38 = (uint *)0x0;
            local_3c = (uint *)0x0;
            local_40 = (uint *)0x48b264;
            pfVar9 = (float *)FUN_00430f50();
            local_3c = (uint *)*pfVar9;
            local_38 = (uint *)pfVar9[1];
            local_34 = (uint *)pfVar9[2];
            local_40 = (uint *)0x48b284;
            local_e7c = FUN_004e9cf0();
          }
          local_8 = 0xffffffff;
          aiStackY_130[local_134] = local_e7c;
          if ((aiStackY_130[local_134] != 0) && (*(int *)(extraout_ECX + 0x2c) != 0)) {
            FUN_0054ac09();
          }
        }
      }
      else if ((((*(short *)(param_1 + 0x18) == 0x29) || (*(short *)(param_1 + 0x18) == 0x1d)) ||
               (*(short *)(param_1 + 0x18) == 0x21)) || (*(short *)(param_1 + 0x18) == 2)) {
        for (local_14c = 0; local_14c < 0xd; local_14c = local_14c + 1) {
          if (*(char *)(param_1 + 0x1c) == -2) {
            if (3 < local_14c) break;
          }
          else if (*(int *)(&DAT_0092ff44 + *(short *)(param_1 + 0x18) * 0x60) <= local_14c) break;
          local_150 = (uint *)(**(code **)(*DAT_013b71e8 + 0x34))();
          if ((local_150 != (uint *)0x0) && (local_150 != local_40)) {
            if (*(short *)(param_1 + 0x18) == 0x21) {
              if ((*(char *)(param_1 + 0x1e) == '\0') ||
                 (((*(char *)(param_1 + 0x1e) == '\x01' && (local_40 != (uint *)0x0)) &&
                  (local_40 == *(uint **)(extraout_ECX + 0x4c))))) {
                local_160 = (uint)(*(char *)(param_1 + 0x1c) == -2);
                if (*(char *)(param_1 + 0x1c) == -3) {
                  local_160 = 2;
                }
                if (*(char *)(param_1 + 0x1c) == -4) {
                  local_2c = (uint *)0x48b633;
                  FUN_00430f50();
                  if (local_40 != (uint *)0x0) {
                    local_16c = local_40[10];
                    local_168 = (float)local_40[0xe] * _DAT_005a4430 + (float)local_40[0xc];
                    local_164 = local_40[0xb];
                  }
                  pvVar7 = operator_new(0xdc);
                  local_8 = 9;
                  if (pvVar7 == (void *)0x0) {
                    local_e88 = 0;
                  }
                  else {
                    local_2c = (uint *)0xff222299;
                    local_30 = (uint *)0xff5555ff;
                    local_34 = (uint *)0x0;
                    local_38 = (uint *)0x0;
                    local_3c = (uint *)0x0;
                    local_40 = (uint *)0x48b6b0;
                    pfVar9 = (float *)FUN_00430f50();
                    local_3c = (uint *)*pfVar9;
                    local_38 = (uint *)pfVar9[1];
                    local_34 = (uint *)pfVar9[2];
                    local_40 = local_150;
                    local_4c = local_16c;
                    local_48 = local_168;
                    local_44 = (uint *)local_164;
                    local_50 = 0x48b6f6;
                    local_e88 = FUN_004d8020();
                  }
                  local_8 = 0xffffffff;
                  local_170 = local_e88;
                  if ((*(int *)(extraout_ECX + 0x2c) != 0) && (local_e88 != 0)) {
                    FUN_0054ac09();
                  }
                }
                else {
                  pvVar7 = operator_new(0x90);
                  local_8 = 10;
                  if (pvVar7 == (void *)0x0) {
                    local_e8c = 0;
                  }
                  else {
                    local_2c = (uint *)local_150[10];
                    local_30 = (uint *)0x48b7a9;
                    pfVar9 = (float *)FUN_00430f50();
                    local_2c = (uint *)*pfVar9;
                    local_30 = (uint *)0x48b7c9;
                    local_e8c = FUN_004ee190();
                  }
                  local_8 = 0xffffffff;
                  local_174 = local_e8c;
                  if ((*(int *)(extraout_ECX + 0x2c) != 0) && (local_e8c != 0)) {
                    FUN_0054ac09();
                  }
                }
              }
            }
            else {
              *(undefined2 *)(local_150 + 0x134) = *(undefined2 *)(param_1 + 0x18);
              *(ushort *)((int)local_150 + 0x4d2) = (ushort)*(byte *)(param_1 + 0x1d);
              local_150[0x133] = (uint)local_150;
              if ((((local_150 == (uint *)0x0) || (local_40 == (uint *)0x0)) ||
                  (local_150[0x93] != 0x38)) || (*(short *)((int)local_150 + 0x1d2) != 0)) {
                local_2c = (uint *)0x48b522;
                FUN_00430f50();
                FUN_004310a0();
              }
              else {
                local_2c = (uint *)0x48b4a7;
                FUN_00430f50();
                local_2c = (uint *)0x48b4e8;
                FUN_00430f50();
                FUN_004310a0();
              }
              iVar5 = (**(code **)(*DAT_0092e654 + 8))();
              local_150[0x12f] = iVar5 + 500;
            }
          }
        }
      }
      else if (*(short *)(param_1 + 0x18) == 0x34) {
        if (*(char *)(param_1 + 0x1c) == -2) {
          CSimpleArray<>();
          local_2c = (uint *)0x48b885;
          FUN_00430f50();
          if (local_2c != (uint *)0x0) {
            local_2c = (uint *)0x48b8af;
            FUN_00430f50();
            FUN_004310a0();
            pvVar7 = operator_new(0xa8);
            local_8 = 0xb;
            if (pvVar7 == (void *)0x0) {
              local_e90 = 0;
            }
            else {
              local_30 = local_180;
              local_2c = local_17c;
              local_3c = local_18c;
              local_38 = local_188;
              local_34 = local_184;
              local_40 = (uint *)0x48b92d;
              local_e90 = FUN_004ecd90();
            }
            local_8 = 0xffffffff;
            local_198 = local_e90;
            if (local_e90 != 0) {
              FUN_0054ac09();
            }
            local_190 = (**(code **)(*DAT_0092e654 + 8))();
            pvVar7 = operator_new(0x90);
            local_8 = 0xc;
            if (pvVar7 == (void *)0x0) {
              local_e94 = 0;
            }
            else {
              local_2c = local_180;
              local_30 = (uint *)0x48b9e1;
              local_e94 = FUN_004d19a0();
            }
            local_8 = 0xffffffff;
            local_194 = local_e94;
            if (local_e94 != 0) {
              FUN_0054ac09();
            }
            if (local_2c[0x146] != 0) {
              FUN_004e8a95();
            }
            if ((DAT_005ccf98 != 0) && (iVar5 = FUN_00429a6d(), iVar5 != 0)) {
              local_2c = (uint *)0x48ba82;
              FUN_00429a6d();
              FUN_0042ad2b();
            }
          }
        }
        else {
          CSimpleArray<>();
          local_2c = (uint *)0x48bac2;
          FUN_00430f50();
          if (local_2c == (uint *)0x0) {
            FUN_00430f20();
            iVar5 = FUN_0049c70f();
            local_1c4 = (float)iVar5 * _DAT_005a4214;
            local_2c = (uint *)0x48bb8f;
            FUN_00430f50();
            FUN_004310a0();
          }
          else {
            local_2c = (uint *)0x48bbbd;
            FUN_00430f50();
            FUN_004310a0();
          }
          FUN_00493db0();
          FUN_00493e10();
          FUN_004310a0();
          FUN_00493d50();
          FUN_004310a0();
          pvVar7 = operator_new(0xb0);
          local_8 = 0xd;
          if (pvVar7 == (void *)0x0) {
            local_eac = 0;
          }
          else {
            local_34 = local_1b4;
            local_30 = local_1b0;
            local_2c = local_1ac;
            local_38 = (uint *)0x48bc7d;
            local_eac = FUN_004e3ba0();
          }
          local_8 = 0xffffffff;
          local_19c = local_eac;
          if (local_eac != 0) {
            FUN_00430f20();
            FUN_0040bd30();
          }
          if ((*(int *)(extraout_ECX + 0x2c) != 0) && (local_19c != 0)) {
            FUN_0054ac09();
          }
        }
      }
      else if ((*(short *)(param_1 + 0x18) == 0x1e) || (*(short *)(param_1 + 0x18) == 0x17)) {
        if (local_40 != (uint *)0x0) {
          local_1cc = 0;
          local_2c = (uint *)0x48bd6a;
          FUN_00430f50();
          if (*(short *)(param_1 + 0x18) == 0x17) {
            local_1cc = 2;
            if (local_2c == (uint *)0x0) {
              local_2c = (uint *)0x48bde0;
              FUN_00430f50();
              FUN_004310a0();
            }
            else {
              local_2c = (uint *)0x48be17;
              FUN_00430f50();
              FUN_004310a0();
            }
          }
          pvVar7 = operator_new(0x94);
          local_8 = 0xe;
          if (pvVar7 == (void *)0x0) {
            local_eb8 = 0;
          }
          else {
            local_30 = local_1d8;
            local_2c = local_1d4;
            local_34 = (uint *)0x48be7c;
            local_eb8 = FUN_004e6590();
          }
          local_8 = 0xffffffff;
          local_1c8 = local_eb8;
          if (local_eb8 != 0) {
            FUN_0054ac09();
          }
        }
      }
      else if (*(short *)(param_1 + 0x18) == 0x37) {
        local_1dc = 3;
        CSimpleArray<>();
        if (local_2c == (uint *)0x0) {
          FUN_00430f20();
          iVar5 = FUN_0049c70f();
          local_1f0 = (float)iVar5 * _DAT_005a4214;
          local_2c = (uint *)0x48bfcb;
          FUN_00430f50();
          FUN_004310a0();
        }
        else {
          local_2c = (uint *)0x48c005;
          FUN_00430f50();
          FUN_004310a0();
          pvVar7 = operator_new(0x94);
          local_8 = 0xf;
          if (pvVar7 == (void *)0x0) {
            local_ed0 = 0;
          }
          else {
            local_30 = local_1ec;
            local_2c = local_1e8;
            local_34 = (uint *)0x48c06a;
            local_ed0 = FUN_004e6590();
          }
          local_8 = 0xffffffff;
          local_1f4 = local_ed0;
          if (local_ed0 != 0) {
            FUN_0054ac09();
          }
        }
        for (local_1e0 = 0;
            ((local_1e0 < 0xd && ((*(short *)(param_1 + 4) != 0x39d || (local_1e0 < 1)))) &&
            ((*(short *)(param_1 + 4) != 0x39e || (local_1e0 < 2)))); local_1e0 = local_1e0 + 1) {
          local_1f8 = (**(code **)(*DAT_013b71e8 + 0x34))();
          if (local_1f8 != 0) {
            pvVar7 = operator_new(0x94);
            local_8 = 0x10;
            if (pvVar7 == (void *)0x0) {
              local_ed4 = 0;
            }
            else {
              local_2c = (uint *)(*(float *)(local_1f8 + 0x30) + _DAT_005a34a0);
              local_30 = *(uint **)(local_1f8 + 0x28);
              local_34 = (uint *)0x48c1b0;
              pfVar9 = (float *)FUN_00430f50();
              local_30 = (uint *)*pfVar9;
              local_2c = (uint *)pfVar9[1];
              local_34 = (uint *)0x48c1d0;
              local_ed4 = FUN_004d8ef0();
            }
            local_8 = 0xffffffff;
            local_1fc = local_ed4;
            if (local_ed4 != 0) {
              *(undefined4 *)(local_ed4 + 0x84) = 2000;
              FUN_0054ac09();
            }
          }
        }
      }
      else if (*(short *)(param_1 + 0x18) == 0x4c) {
        if ((DAT_005ccf98 != 0) && (iVar5 = FUN_00429a6d(), iVar5 != 0)) {
          local_2c = (uint *)0x48c270;
          FUN_00429a6d();
          FUN_0042ad2b();
        }
      }
      else if (*(short *)(param_1 + 0x18) == 0x4d) {
        if ((DAT_005ccf98 != 0) && (iVar5 = FUN_00429a6d(), iVar5 != 0)) {
          local_2c = (uint *)0x48c2b3;
          FUN_00429a6d();
          FUN_0042ad2b();
        }
      }
      else if (*(short *)(param_1 + 0x18) == 0x57) {
        if (DAT_0092e660 == 0) {
          for (local_200 = 0; (int)local_200 < 10; local_200 = local_200 + 1) {
            local_208 = _rand();
            pvVar7 = operator_new(0x180);
            local_8 = 0x11;
            if (pvVar7 == (void *)0x0) {
              local_ee4 = (int *)0x0;
            }
            else {
              uVar11 = local_200 & 0x80000001;
              if ((int)uVar11 < 0) {
                uVar11 = (uVar11 - 1 | 0xfffffffe) + 1;
              }
              local_2c = (uint *)((float)(int)uVar11 * _DAT_005a4214 + _DAT_005a4158);
              uVar11 = local_200 & 0x80000001;
              if ((int)uVar11 < 0) {
                uVar11 = (uVar11 - 1 | 0xfffffffe) + 1;
              }
              local_30 = (uint *)((float)(int)uVar11 * _DAT_005a4214 + _DAT_005a4158);
              uVar11 = local_200 & 0x80000001;
              if ((int)uVar11 < 0) {
                uVar11 = (uVar11 - 1 | 0xfffffffe) + 1;
              }
              local_34 = (uint *)((float)(int)uVar11 * _DAT_005a4214 + _DAT_005a4158);
              local_38 = (uint *)(local_200 * 0x96 + 1000);
              local_3c = (uint *)0x38;
              local_40 = (uint *)0x48c3e8;
              local_ee4 = (int *)FUN_004c8f70();
            }
            local_8 = 0xffffffff;
            local_204 = local_ee4;
            if (local_ee4 != (int *)0x0) {
              local_ee4[0x4f] = (int)((float)((int)local_200 % 3) * _DAT_005a442c + _DAT_005a4428);
              local_ee4[0x50] = -0x40000000;
              local_ee4[0x1b] = 1;
              local_ee4[0x52] = 3;
              local_2c = (uint *)0x48c536;
              FUN_00430f50();
              FUN_004310a0();
              FUN_004310a0();
              (**(code **)(*local_204 + 0x3c))();
              FUN_0054ac09();
            }
          }
        }
      }
      else if (*(short *)(param_1 + 0x18) == 0x5e) {
        if (local_40 != (uint *)0x0) {
          local_2c = (uint *)0x48c5cf;
          FUN_00430f50();
          FUN_00430f20();
          local_228 = 0;
          for (local_21c = 0;
              ((local_21c < 0xd && ((*(short *)(param_1 + 4) != 0x39d || (local_21c < 1)))) &&
              ((*(short *)(param_1 + 4) != 0x39e || (local_21c < 2)))); local_21c = local_21c + 1) {
            local_240 = (**(code **)(*DAT_013b71e8 + 0x34))();
            if (local_240 != 0) {
              local_2c = (uint *)0x48c6b0;
              FUN_00430f50();
              local_214 = local_214 + *(float *)(local_240 + 0x28);
              local_210 = local_210 + *(float *)(local_240 + 0x2c);
              local_228 = local_228 + 1;
              pvVar7 = operator_new(0x94);
              local_8 = 0x12;
              if (pvVar7 == (void *)0x0) {
                local_ef8 = 0;
              }
              else {
                local_2c = (uint *)0x98;
                local_30 = (uint *)0x2;
                local_3c = local_250;
                local_38 = local_24c;
                local_34 = local_248;
                local_48 = local_234;
                local_44 = (uint *)local_230;
                local_40 = local_22c;
                local_4c = 0x48c762;
                local_ef8 = FUN_004c5cc0();
              }
              local_8 = 0xffffffff;
              local_244 = local_ef8;
              if (local_ef8 != 0) {
                FUN_0054ac09();
              }
            }
          }
          FUN_00430f10();
          if (local_228 < 1) {
            FUN_00430f20();
            FUN_0040bd30();
          }
          else {
            local_214 = local_214 / (float)local_228;
            local_210 = local_210 / (float)local_228;
            FUN_0040bd30();
          }
          fVar12 = (float10)FUN_00493c30();
          local_20c = (float)(fVar12 - (float10)_DAT_005a4380);
          if (local_20c < _DAT_005a3534) {
            local_20c = local_20c + _DAT_005a4290;
          }
          if (_DAT_005a4290 < local_20c) {
            local_20c = local_20c - _DAT_005a4290;
          }
          local_23c = __ftol();
          local_238 = local_23c + -3;
          local_218 = local_23c + 3;
          if ((local_238 < 0) || (iVar5 = local_238, 0xc < local_218)) {
            if (local_238 < 0) {
              local_274 = local_23c + 9;
              local_26c = local_218;
            }
            else {
              local_26c = local_23c + -9;
              local_274 = local_238;
            }
            for (local_270 = 0; local_270 < local_26c; local_270 = local_270 + 1) {
              FUN_00430f20();
              local_2c = (uint *)0x48cb91;
              FUN_00430f50();
              pvVar7 = operator_new(0x94);
              local_8 = 0x14;
              if (pvVar7 == (void *)0x0) {
                local_f08 = 0;
              }
              else {
                local_2c = (uint *)0x98;
                local_30 = (uint *)0x2;
                local_3c = local_28c;
                local_38 = local_288;
                local_34 = local_284;
                local_48 = local_234;
                local_44 = (uint *)local_230;
                local_40 = local_22c;
                local_4c = 0x48cc0a;
                local_f08 = FUN_004c5cc0();
              }
              local_8 = 0xffffffff;
              local_278 = local_f08;
              if (local_f08 != 0) {
                FUN_0054ac09();
              }
            }
            for (local_270 = local_274; local_270 < 0xc; local_270 = local_270 + 1) {
              FUN_00430f20();
              local_2c = (uint *)0x48ccf6;
              FUN_00430f50();
              pvVar7 = operator_new(0x94);
              local_8 = 0x15;
              if (pvVar7 == (void *)0x0) {
                local_f0c = 0;
              }
              else {
                local_2c = (uint *)0x98;
                local_30 = (uint *)0x2;
                local_3c = local_2a4;
                local_38 = local_2a0;
                local_34 = local_29c;
                local_48 = local_234;
                local_44 = (uint *)local_230;
                local_40 = local_22c;
                local_4c = 0x48cd6f;
                local_f0c = FUN_004c5cc0();
              }
              local_8 = 0xffffffff;
              local_290 = local_f0c;
              if (local_f0c != 0) {
                FUN_0054ac09();
              }
            }
          }
          else {
            while (local_21c = iVar5, local_21c < local_218) {
              FUN_00430f20();
              local_2c = (uint *)0x48c9cf;
              FUN_00430f50();
              pvVar7 = operator_new(0x94);
              local_8 = 0x13;
              if (pvVar7 == (void *)0x0) {
                local_f04 = 0;
              }
              else {
                local_2c = (uint *)0x98;
                local_30 = (uint *)0x2;
                local_3c = local_268;
                local_38 = local_264;
                local_34 = local_260;
                local_48 = local_234;
                local_44 = (uint *)local_230;
                local_40 = local_22c;
                local_4c = 0x48ca48;
                local_f04 = FUN_004c5cc0();
              }
              local_8 = 0xffffffff;
              local_254 = local_f04;
              if (local_f04 != 0) {
                FUN_0054ac09();
              }
              iVar5 = local_21c + 1;
            }
          }
        }
      }
      else if (*(short *)(param_1 + 0x18) == 99) {
        if (local_2c != (uint *)0x0) {
          local_2b0 = 0;
          if ((local_2c[0x93] == 4) || (local_2c[0x93] == 8)) {
            local_2b0 = 1;
          }
          pvVar7 = operator_new(0x22c);
          local_8 = 0x16;
          if (pvVar7 == (void *)0x0) {
            local_f10 = (int *)0x0;
          }
          else {
            local_2c = (uint *)0x0;
            local_30 = (uint *)0x0;
            local_34 = (uint *)0x48ce46;
            pfVar9 = (float *)FUN_00430f50();
            local_30 = (uint *)*pfVar9;
            local_2c = (uint *)pfVar9[1];
            local_34 = (uint *)0x0;
            local_38 = (uint *)0x0;
            local_3c = (uint *)0x0;
            local_40 = (uint *)0x48ce6c;
            pfVar9 = (float *)FUN_00430f50();
            local_3c = (uint *)*pfVar9;
            local_38 = (uint *)pfVar9[1];
            local_34 = (uint *)pfVar9[2];
            local_40 = (uint *)local_2c[0x17];
            local_44 = (uint *)0x48ce93;
            local_f10 = (int *)FUN_004d5480();
          }
          local_8 = 0xffffffff;
          local_2a8 = local_f10;
          if (local_f10 != (int *)0x0) {
            if (((local_2c == (uint *)0x0) || ((char)local_2c[0x1e7] < '\x01')) ||
               (local_2c[0x67] == 0)) {
              local_2c = (uint *)0x48cf79;
              FUN_0058f220();
            }
            else {
              local_f10[0x17] = local_2c[0x1e8];
              local_2c = (uint *)0x48cf24;
              FUN_0058f220();
              local_2a8[0x88] = *(int *)((int)local_2c + 0x5c);
              local_2c = (uint *)0x48cf57;
              FUN_0058f220();
            }
            local_2a8[0x69] = 0x3f000000;
            local_2a8[0x6a] = 0x3f000000;
            local_2a8[0x6b] = 0x3f000000;
            (**(code **)(*local_2a8 + 0x78))();
            local_2a8[0x75] = 1;
            local_2a8[0x65] = 3000;
            local_2c = *(uint **)((int)local_2c + 0x28);
            local_30 = (uint *)0x48d013;
            (**(code **)(*local_2a8 + 0x3c))();
            if (((char)local_2c[0x1e7] < '\x01') || (local_2c[0x67] == 0)) {
              FUN_004bd906();
            }
            else {
              if (local_2a8[0x18] != 0) {
                FUN_004bd906();
              }
              if (local_2a8[0x87] != 0) {
                FUN_004bd906();
              }
            }
            local_2a8[0x68] = local_2c[0xd];
            local_2a8[0x66] = 1;
            local_2a8[0x67] = 8;
            FUN_0054ac09();
          }
          local_2ac = 0xaaaaaaaa;
          for (local_2b4 = -1; local_2b4 < 2; local_2b4 = local_2b4 + 1) {
            pvVar7 = operator_new(0xcc);
            local_8 = 0x17;
            if (pvVar7 == (void *)0x0) {
              local_f14 = 0;
            }
            else {
              local_2c = (uint *)0x1fa;
              local_30 = (uint *)0x48d173;
              local_f14 = FUN_004d2230();
            }
            local_8 = 0xffffffff;
            local_2bc = local_f14;
            if (local_f14 != 0) {
              *(undefined4 *)(local_f14 + 0x6c) = 1;
              *(undefined1 *)(local_f14 + 0xac) = 1;
              *(undefined4 *)(local_f14 + 0xb8) = 3000;
              *(undefined4 *)(local_f14 + 0xb0) = 3000;
              *(undefined4 *)(local_f14 + 0x98) = 0x3fc00000;
              *(undefined4 *)(local_f14 + 0x9c) = 0x40400000;
              local_2c = (uint *)0x48d252;
              FUN_00430f50();
              FUN_004310a0();
              FUN_0054ac09();
            }
          }
          pvVar7 = operator_new(0xc0);
          local_8 = 0x18;
          if (pvVar7 == (void *)0x0) {
            local_f18 = (int *)0x0;
          }
          else {
            local_2c = (uint *)0x48d2b2;
            local_f18 = (int *)FUN_004df250();
          }
          local_8 = 0xffffffff;
          local_2b8 = local_f18;
          if (local_f18 != (int *)0x0) {
            local_f18[0x28] = 8000;
            (**(code **)(*local_f18 + 0x44))();
            local_2b8[0x1b] = 1;
            local_2c = (uint *)0x48d33a;
            FUN_00430f20();
            local_2c = (uint *)0x48d352;
            (**(code **)(*local_2b8 + 0x3c))();
            FUN_0054ac09();
          }
          if ((DAT_005ccf98 != 0) && (iVar5 = FUN_00429a6d(), iVar5 != 0)) {
            local_2c = (uint *)0x48d392;
            FUN_00429a6d();
            FUN_0042ad2b();
          }
        }
      }
      else if (*(short *)(param_1 + 0x18) == 100) {
        if ((DAT_005ccf98 != 0) && (iVar5 = FUN_00429a6d(), iVar5 != 0)) {
          local_2c = (uint *)0x48d3db;
          FUN_00429a6d();
          FUN_0042ad2b();
        }
      }
      else if ((*(short *)(param_1 + 0x18) == 0x6a) || (*(short *)(param_1 + 0x18) == 0x6c)) {
        local_2c0 = (uint)(*(short *)(param_1 + 0x18) == 0x6c);
        if ((local_2c != (uint *)0x0) && (local_40 != (uint *)0x0)) {
          FUN_0052236f();
        }
      }
      else if (*(short *)(param_1 + 0x18) == 0x6b) {
        if ((local_2c != (uint *)0x0) && (local_40 != (uint *)0x0)) {
          local_2c8 = *(float *)(&DAT_0092e7d4 + local_40[0x17] * 8) * (float)local_40[0xe] *
                      _DAT_005a4160 + (float)local_40[0xc];
          if (local_40[0x17] == 8) {
            local_2c8 = _DAT_005a3660 * (float)local_40[0xe] + local_2c8;
          }
          local_2c = (uint *)0x48d4cd;
          FUN_00430f50();
          pvVar7 = operator_new(0xdc);
          local_8 = 0x19;
          if (pvVar7 == (void *)0x0) {
            local_f1c = 0;
          }
          else {
            local_2c = (uint *)0xff554411;
            local_30 = (uint *)0xffffaa00;
            local_34 = (uint *)0x0;
            local_38 = (uint *)0x0;
            local_3c = (uint *)0x0;
            local_40 = (uint *)0x48d514;
            pfVar9 = (float *)FUN_00430f50();
            local_3c = (uint *)*pfVar9;
            local_38 = (uint *)pfVar9[1];
            local_34 = (uint *)pfVar9[2];
            local_40 = local_2c;
            local_4c = local_2d4;
            local_48 = local_2d0;
            local_44 = (uint *)local_2cc;
            local_50 = 0x48d557;
            local_f1c = FUN_004d8020();
          }
          local_8 = 0xffffffff;
          local_2c4 = local_f1c;
          if (local_f1c != 0) {
            FUN_0054ac09();
          }
        }
      }
      else if (*(short *)(param_1 + 0x18) == 0x6d) {
        if ((local_2c != (uint *)0x0) && (local_40 != (uint *)0x0)) {
          local_2dc = *(float *)(&DAT_0092e7d4 + local_40[0x17] * 8) * (float)local_40[0xe] *
                      _DAT_005a4160 + (float)local_40[0xc];
          if (local_40[0x17] == 0x2a) {
            local_2dc = _DAT_005a3660 * (float)local_40[0xe] + local_2dc;
          }
          local_2c = (uint *)0x48d635;
          FUN_00430f50();
          pvVar7 = operator_new(0xdc);
          local_8 = 0x1a;
          if (pvVar7 == (void *)0x0) {
            local_f20 = 0;
          }
          else {
            local_2c = (uint *)0xaa663311;
            local_30 = (uint *)0xff550000;
            local_34 = (uint *)0x0;
            local_38 = (uint *)0x0;
            local_3c = (uint *)0x0;
            local_40 = (uint *)0x48d67c;
            pfVar9 = (float *)FUN_00430f50();
            local_3c = (uint *)*pfVar9;
            local_38 = (uint *)pfVar9[1];
            local_34 = (uint *)pfVar9[2];
            local_40 = local_2c;
            local_4c = local_2e8;
            local_48 = local_2e4;
            local_44 = (uint *)local_2e0;
            local_50 = 0x48d6bf;
            local_f20 = FUN_004d8020();
          }
          local_8 = 0xffffffff;
          local_2d8 = local_f20;
          if (local_f20 != 0) {
            FUN_0054ac09();
          }
        }
      }
      else if (*(short *)(param_1 + 0x18) == 0x6e) {
        local_2c = (uint *)0x48d734;
        FUN_00430f50();
        local_2c = (uint *)0x48d745;
        FUN_00430f50();
        local_310 = 0;
        for (local_6c = 0;
            ((local_6c < 0xd && ((*(short *)(param_1 + 4) != 0x39d || (local_6c < 1)))) &&
            ((*(short *)(param_1 + 4) != 0x39e || (local_6c < 2)))); local_6c = local_6c + 1) {
          local_318 = _rand();
          local_318 = local_318 % 0x12;
          local_31c = _rand();
          local_31c = local_31c % 0x12;
          local_314 = (**(code **)(*DAT_013b71e8 + 0x34))();
          if (local_314 == 0) {
            if ((((float)local_300 != _DAT_005a3534) && (local_2fc != _DAT_005a3534)) &&
               (local_2f8 != _DAT_005a3534)) {
              pvVar7 = operator_new(0xc0);
              local_8 = 0x1c;
              if (pvVar7 == (void *)0x0) {
                local_f28 = 0;
              }
              else {
                local_2c = (uint *)local_2fc;
                local_30 = (uint *)(((float)local_318 + (float)local_300) - _DAT_005a4424);
                local_34 = (uint *)0x48da1d;
                pfVar9 = (float *)FUN_00430f50();
                local_30 = (uint *)*pfVar9;
                local_2c = (uint *)pfVar9[1];
                local_34 = (uint *)(((float)local_31c + (float)local_2ec) - _DAT_005a4424);
                local_38 = local_2f0;
                local_3c = (uint *)(((float)local_318 + (float)local_2f4) - _DAT_005a4424);
                local_40 = (uint *)0x48da70;
                pfVar9 = (float *)FUN_00430f50();
                local_3c = (uint *)*pfVar9;
                local_38 = (uint *)pfVar9[1];
                local_34 = (uint *)pfVar9[2];
                local_40 = (uint *)0x48da90;
                local_f28 = FUN_004e9cf0();
              }
              local_8 = 0xffffffff;
              local_324 = local_f28;
              iVar5 = _rand();
              *(int *)(local_324 + 0x80) = *(int *)(local_324 + 0x80) + iVar5 % 1000;
              if ((*(int *)(extraout_ECX + 0x2c) != 0) && (local_324 != 0)) {
                FUN_0054ac09();
              }
            }
          }
          else {
            if (local_314 == *(int *)(extraout_ECX + 0x4c)) {
              local_310 = 1;
            }
            local_2c = (uint *)0x48d847;
            FUN_00430f50();
            FUN_004310a0();
            FUN_004310a0();
            local_2f0 = (uint *)0x3f800000;
            pvVar7 = operator_new(0xc0);
            local_8 = 0x1b;
            if (pvVar7 == (void *)0x0) {
              local_f24 = 0;
            }
            else {
              local_30 = local_300;
              local_2c = (uint *)local_2fc;
              local_3c = local_2f4;
              local_38 = local_2f0;
              local_34 = local_2ec;
              local_40 = (uint *)0x48d8df;
              local_f24 = FUN_004e9cf0();
            }
            local_8 = 0xffffffff;
            local_320 = local_f24;
            if (local_f24 != 0) {
              iVar5 = _rand();
              *(int *)(local_320 + 0x80) = *(int *)(local_320 + 0x80) + iVar5 % 1000;
            }
            if ((*(int *)(extraout_ECX + 0x2c) != 0) && (local_320 != 0)) {
              FUN_0054ac09();
            }
          }
        }
        uVar11 = local_40[399];
        uVar3 = (**(code **)(*DAT_0092e654 + 8))();
        if (uVar11 + 0x6a4 < uVar3) {
          iVar5 = (**(code **)(*DAT_0092e654 + 8))();
          local_40[399] = iVar5 - 1000;
          for (local_328 = 0xfffffffd; (int)local_328 < 3; local_328 = local_328 + 1) {
            for (local_330 = -3; local_330 < 3; local_330 = local_330 + 1) {
              pvVar7 = operator_new(0x180);
              local_8 = 0x1d;
              if (pvVar7 == (void *)0x0) {
                local_f2c = (int *)0x0;
              }
              else {
                local_2c = (uint *)0x40000000;
                local_30 = (uint *)0x40000000;
                local_34 = (uint *)0x40000000;
                local_38 = (uint *)0xfa0;
                local_3c = (uint *)0xc1;
                local_40 = (uint *)0x48dbfd;
                local_f2c = (int *)FUN_004c8f70();
              }
              local_8 = 0xffffffff;
              local_334 = local_f2c;
              if (local_f2c != (int *)0x0) {
                uVar11 = local_328 & 0x80000001;
                if ((int)uVar11 < 0) {
                  uVar11 = (uVar11 - 1 | 0xfffffffe) + 1;
                }
                local_f2c[0x40] = uVar11;
                local_2c = (uint *)0x48dc8f;
                FUN_00430f50();
                FUN_004310a0();
                (**(code **)(*local_334 + 0x3c))();
                FUN_0054ac09();
              }
            }
          }
          local_32c = *(int *)(DAT_0067cf38 + 0x40);
          if (local_32c != 0) {
            uVar8 = (**(code **)(*DAT_0092e654 + 8))();
            *(undefined4 *)(local_32c + 0x20a6c) = uVar8;
            FUN_0040bd30();
          }
        }
        __ftol();
        __ftol();
        __ftol();
        local_2c = (uint *)0x48dd5b;
        local_2c = (uint *)__ftol();
        local_30 = (uint *)0x48dd61;
        local_30c = FUN_005563a0();
        local_304 = 2000;
        local_308 = (**(code **)(*DAT_0092e654 + 8))();
        if (local_310 == 0) {
          local_304 = 1000;
        }
        if ((((local_30c < 0x12) &&
             (uVar3 = *(int *)(*(int *)(extraout_ECX + 0x4c) + 0x5ec) + local_304,
             uVar11 = (**(code **)(*DAT_0092e654 + 8))(), uVar3 < uVar11)) &&
            (*(char *)(*(int *)(extraout_ECX + 0x4c) + 0x245) == '\0')) &&
           ((*(int *)(*(int *)(extraout_ECX + 0x4c) + 0x740) != 0xc &&
            (*(int *)(*(int *)(extraout_ECX + 0x4c) + 0x740) != 0xb)))) {
          local_33c = 4;
          local_338 = 5;
          if (local_310 == 0) {
            local_33c = 8;
            local_338 = 9;
          }
          iVar5 = _rand();
          if (iVar5 % 10 < local_33c) {
            *(undefined4 *)(extraout_ECX + 0x26e90) = 0;
            if (DAT_005d0408 == 0) {
              FUN_00450962();
            }
            local_3b4 = (uint *)__ftol();
            local_3b8 = (uint *)__ftol();
            local_3b0 = 0;
            local_3ac = 0;
            if ((int)local_3b4 < 0x93b) {
              local_3b0 = -local_338;
            }
            else if (0x941 < (int)local_3b4) {
              local_3b0 = local_338;
            }
            if ((int)local_3b8 < 0xf58) {
              local_3ac = -local_338;
            }
            else if (0xf5e < (int)local_3b8) {
              local_3ac = local_338;
            }
            local_3a4 = __ftol();
            local_3a8 = __ftol();
            if ((int)local_3a4 < 0x925) {
              local_3a4 = 0x925;
            }
            else if (0x957 < (int)local_3a4) {
              local_3a4 = 0x957;
            }
            if ((int)local_3a8 < 0xf43) {
              local_3a4 = 0xf43;
            }
            else if (0xf70 < (int)local_3a8) {
              local_3a8 = 0xf70;
            }
            local_2c = (uint *)0x48dfda;
            _memset(local_36c,0,0x30);
            local_2c = (uint *)local_36c;
            local_30 = &local_3a8;
            local_34 = &local_3a4;
            local_38 = local_3b8;
            local_3c = local_3b4;
            local_40 = (uint *)0x48e016;
            FUN_00555ad8();
            sVar4 = _strlen(local_36c);
            if (sVar4 == 0) {
              ExceptionList = local_10;
              return 1;
            }
            local_2c = (uint *)0x48e046;
            _memset(local_3a0,0,0x34);
            local_394 = (short)local_3a4;
            local_392 = (short)local_3a8;
            local_38c = 2;
            _local_39c = CONCAT22(*(undefined2 *)(*(int *)(extraout_ECX + 0x4c) + 0x20),0x366);
            local_390 = DAT_005bb418;
            local_388 = (short)local_3a4;
            sStackY_386 = (short)local_3a8;
            DAT_005d03fc = 0x366;
            *(int *)(extraout_ECX + 0x2878c) = (int)(short)local_3a4;
            *(int *)(extraout_ECX + 0x28790) = (int)(short)local_3a8;
            *(uint *)(extraout_ECX + 0x28794) = local_3a4 & 0xffff;
            *(uint *)(extraout_ECX + 0x28798) = local_3a8 & 0xffff;
            FUN_0055f2dd();
            uVar8 = (**(code **)(*DAT_0092e654 + 8))();
            *(undefined4 *)(*(int *)(extraout_ECX + 0x4c) + 0x5ec) = uVar8;
            (**(code **)(**(int **)(extraout_ECX + 0x4c) + 4))();
          }
        }
      }
      else if (*(short *)(param_1 + 0x18) == 0x6f) {
        local_2c = (uint *)0x48e1ac;
        FUN_00430f50();
        pvVar7 = operator_new(0xc0);
        local_8 = 0x1e;
        if (pvVar7 == (void *)0x0) {
          local_f30 = (int *)0x0;
        }
        else {
          local_2c = (uint *)0x48e1e3;
          local_f30 = (int *)FUN_004df250();
        }
        local_8 = 0xffffffff;
        local_3d4 = local_f30;
        if (local_f30 != (int *)0x0) {
          (**(code **)(*local_f30 + 0x44))();
          local_3d4[0x1b] = 1;
          local_2c = (uint *)0x48e259;
          FUN_00430f20();
          local_2c = (uint *)0x48e271;
          (**(code **)(*local_3d4 + 0x3c))();
          local_3d4[0x28] = 0xc80;
          FUN_0054ac09();
        }
        local_2c = (uint *)0x48e2a7;
        FUN_00430f50();
        CSimpleArray<>();
        CSimpleArray<>();
        CSimpleArray<>();
        FUN_00430f10();
        for (local_3d8 = 0;
            ((local_3d8 < 0xd && ((*(short *)(param_1 + 4) != 0x39d || (local_3d8 < 1)))) &&
            ((*(short *)(param_1 + 4) != 0x39e || (local_3d8 < 2)))); local_3d8 = local_3d8 + 1) {
          local_410 = _rand();
          local_410 = local_410 % 0x12;
          local_414 = _rand();
          local_414 = local_414 % 0x12;
          local_40c = (**(code **)(*DAT_013b71e8 + 0x34))();
          if (local_40c == 0) {
            if (((local_3f0 != _DAT_005a3534) && (local_3ec != _DAT_005a3534)) &&
               (local_3e8 != _DAT_005a3534)) {
              local_2c = (uint *)0x48e53c;
              FUN_00430f50();
              FUN_004310a0();
              pvVar7 = operator_new(0xb0);
              local_8 = 0x20;
              if (pvVar7 == (void *)0x0) {
                local_f38 = 0;
              }
              else {
                local_34 = local_408;
                local_30 = local_404;
                local_2c = local_400;
                local_38 = (uint *)0x48e59b;
                local_f38 = FUN_004e3ba0();
              }
              local_8 = 0xffffffff;
              local_41c = local_f38;
              if (local_f38 != 0) {
                FUN_00430f20();
                FUN_0040bd30();
              }
              if (*(int *)(extraout_ECX + 0x2c) != 0) {
                FUN_0054ac09();
              }
            }
          }
          else {
            local_2c = (uint *)0x48e3bd;
            FUN_00430f50();
            FUN_004310a0();
            FUN_004310a0();
            pvVar7 = operator_new(0xb0);
            local_8 = 0x1f;
            if (pvVar7 == (void *)0x0) {
              local_f34 = 0;
            }
            else {
              local_34 = local_408;
              local_30 = local_404;
              local_2c = local_400;
              local_38 = (uint *)0x48e42e;
              local_f34 = FUN_004e3ba0();
            }
            local_8 = 0xffffffff;
            local_418 = local_f34;
            if (local_f34 != 0) {
              FUN_00430f20();
              FUN_0040bd30();
            }
            if (*(int *)(extraout_ECX + 0x2c) != 0) {
              FUN_0054ac09();
            }
          }
        }
      }
      else if ((local_40[0x93] == 0x38) && (*(short *)((int)local_40 + 0x1d2) == 0)) {
        CSimpleArray<>();
        CSimpleArray<>();
        local_2c = (uint *)0x48e676;
        FUN_00430f50();
        FUN_004310a0();
        FUN_004310a0();
        CSimpleArray<>();
        for (local_6c = 0;
            ((local_6c < 0xd && ((*(short *)(param_1 + 4) != 0x39d || (local_6c < 1)))) &&
            ((*(short *)(param_1 + 4) != 0x39e || (local_6c < 2)))); local_6c = local_6c + 1) {
          local_454 = _rand();
          local_454 = local_454 % 0x12;
          local_458 = _rand();
          local_458 = local_458 % 0x12;
          local_450 = (uint *)(**(code **)(*DAT_013b71e8 + 0x34))();
          if (local_450 == (uint *)0x0) {
            if (((local_44c != _DAT_005a3534) && (local_448 != _DAT_005a3534)) &&
               (local_444 != _DAT_005a3534)) {
              FUN_004310a0();
              pvVar7 = operator_new(0xdc);
              local_8 = 0x22;
              if (pvVar7 == (void *)0x0) {
                local_f40 = 0;
              }
              else {
                local_2c = (uint *)0xff222299;
                local_30 = (uint *)0xffff0000;
                local_34 = (uint *)(((float)local_458 + local_444) - _DAT_005a4424);
                local_38 = (uint *)local_448;
                local_3c = (uint *)(((float)local_454 + local_44c) - _DAT_005a4424);
                local_40 = (uint *)0x48e9a3;
                pfVar9 = (float *)FUN_00430f50();
                local_3c = (uint *)*pfVar9;
                local_38 = (uint *)pfVar9[1];
                local_34 = (uint *)pfVar9[2];
                local_40 = (uint *)0x0;
                local_4c = local_434;
                local_48 = local_430;
                local_44 = (uint *)local_42c;
                local_50 = 0x48e9e4;
                local_f40 = FUN_004d8020();
              }
              local_8 = 0xffffffff;
              local_460 = local_f40;
              if (local_f40 != 0) {
                *(undefined4 *)(local_f40 + 0x84) = 0x3ecccccd;
              }
              if ((*(int *)(extraout_ECX + 0x2c) != 0) && (local_f40 != 0)) {
                FUN_0054ac09();
              }
            }
          }
          else {
            FUN_004310a0();
            local_2c = (uint *)0x48e7ab;
            FUN_00430f50();
            FUN_004310a0();
            pvVar7 = operator_new(0xdc);
            local_8 = 0x21;
            if (pvVar7 == (void *)0x0) {
              local_f3c = 0;
            }
            else {
              local_2c = (uint *)0xff222299;
              local_30 = (uint *)0xffff0000;
              local_34 = (uint *)0x0;
              local_38 = (uint *)0x0;
              local_3c = (uint *)0x0;
              local_40 = (uint *)0x48e7fe;
              pfVar9 = (float *)FUN_00430f50();
              local_3c = (uint *)*pfVar9;
              local_38 = (uint *)pfVar9[1];
              local_34 = (uint *)pfVar9[2];
              local_40 = local_450;
              local_4c = local_434;
              local_48 = local_430;
              local_44 = (uint *)local_42c;
              local_50 = 0x48e844;
              local_f3c = FUN_004d8020();
            }
            local_8 = 0xffffffff;
            local_45c = local_f3c;
            if (local_f3c != 0) {
              *(undefined4 *)(local_f3c + 0x84) = 0x3e4ccccd;
            }
            if ((*(int *)(extraout_ECX + 0x2c) != 0) && (local_f3c != 0)) {
              FUN_0054ac09();
            }
          }
        }
      }
    }
  }
  local_44 = (uint *)0x0;
  CSimpleArray<>();
  CSimpleArray<>();
  if (local_40 == (uint *)0x0) {
    FUN_00430f20();
    iVar5 = FUN_0049c70f();
    local_464 = (float)iVar5 * _DAT_005a4214;
    local_2c = (uint *)0x48eb3b;
    FUN_00430f50();
    FUN_004310a0();
  }
  else {
    local_2c = (uint *)0x48eaa3;
    FUN_00430f50();
    FUN_004310a0();
  }
  if ((((*(int *)(&DAT_0092fefc + *(short *)(param_1 + 0x18) * 0x60) == 3) ||
       (*(int *)(&DAT_0092fefc + *(short *)(param_1 + 0x18) * 0x60) == 4)) ||
      (*(int *)(&DAT_0092fefc + *(short *)(param_1 + 0x18) * 0x60) == 5)) ||
     (*(int *)(&DAT_0092fefc + *(short *)(param_1 + 0x18) * 0x60) == 6)) {
    FUN_00430f20();
    iVar5 = FUN_0049c70f();
    local_468 = (float)iVar5 * _DAT_005a4214;
    local_2c = (uint *)0x48ec46;
    fVar6 = (float)FUN_00430f50();
    puVar14 = (uint *)0x48ec4f;
    FUN_004310a0();
    if ((local_40 != *(uint **)(extraout_ECX + 0x4c)) ||
       ((*(char *)(param_1 + 0x1e) == '\0' && (local_40 == *(uint **)(extraout_ECX + 0x4c))))) {
      for (local_46c = 0;
          ((local_46c < 0xd && ((*(short *)(param_1 + 4) != 0x39d || (local_46c < 1)))) &&
          ((*(short *)(param_1 + 4) != 0x39e || (local_46c < 2)))); local_46c = local_46c + 1) {
        fVar6 = (float)(uint)*(ushort *)(param_1 + 0x2c + local_46c * 4);
        puVar14 = (uint *)0x48ed07;
        local_470 = (**(code **)(*DAT_013b71e8 + 0x34))();
        if (local_470 != 0) {
          local_474 = (int)*(char *)(local_470 + 0x7a9);
          if (*(char *)(local_470 + 0x7a9) == '\0') {
            local_474 = 1;
          }
          if ((*(short *)(param_1 + 0x2e + local_46c * 4) == -3) ||
             (*(short *)(param_1 + 0x2e + local_46c * 4) == -4)) {
            local_2c = (uint *)(*(float *)(local_470 + 0x30) + _DAT_005a3660);
            local_30 = *(uint **)(local_470 + 0x28);
            local_34 = (uint *)0x48edac;
            iVar5 = FUN_00430d79();
            if (iVar5 != 0) {
              if (DAT_0092e660 == 0) {
                local_480 = 1;
              }
              else {
                local_480 = (uint)(local_40 == *(uint **)(extraout_ECX + 0x4c));
                if (local_470 == *(int *)(extraout_ECX + 0x4c)) {
                  local_480 = 1;
                }
              }
              if (local_480 != 0) {
                FUN_0058f078();
                pvVar7 = operator_new(0x2a0);
                local_8 = 0x23;
                if (pvVar7 == (void *)0x0) {
                  local_f64 = 0;
                }
                else {
                  local_2c = (uint *)0x1;
                  local_30 = (uint *)0x5dc;
                  local_34 = (uint *)0x40000000;
                  local_38 = (uint *)0xffffffff;
                  local_3c = (uint *)0x48ee74;
                  iVar5 = __ftol();
                  local_3c = (uint *)(local_47c + iVar5);
                  local_40 = local_478;
                  local_44 = local_500;
                  local_48 = 6.697744e-39;
                  local_f64 = FUN_00422930();
                }
                local_8 = 0xffffffff;
                local_504 = local_f64;
                if (local_f64 != 0) {
                  FUN_0054ac09();
                }
              }
            }
          }
          else if ((((*(short *)(param_1 + 0x18) < 0) || (0x67 < *(short *)(param_1 + 0x18))) ||
                   (*(int *)(&DAT_0092ff0c + *(short *)(param_1 + 0x18) * 0x60) != 6)) ||
                  ((*(short *)(param_1 + 0x2e + local_46c * 4) < 0 &&
                   (-6 < *(short *)(param_1 + 0x2e + local_46c * 4))))) {
            if (*(char *)(param_1 + 0x1e) == '\0') {
              if (*(int *)(local_470 + 0x21c) == 0) {
                iVar5 = (int)*(short *)(param_1 + 0x2e + local_46c * 4) / local_474;
                if (*(short *)(local_470 + 0x450) == iVar5 ||
                    *(short *)(local_470 + 0x450) - iVar5 < 0) {
                  *(undefined2 *)(local_470 + 0x450) = 0;
                }
                else {
                  *(short *)(local_470 + 0x450) =
                       *(short *)(local_470 + 0x450) -
                       (short)((int)*(short *)(param_1 + 0x2e + local_46c * 4) / local_474);
                }
              }
              else {
                if (*(int *)(local_470 + 0x218) == (int)*(short *)(param_1 + 0x2e + local_46c * 4))
                {
                  *(undefined4 *)(local_470 + 0x218) = 0;
                }
                else {
                  *(int *)(local_470 + 0x218) =
                       *(int *)(local_470 + 0x218) - (int)*(short *)(param_1 + 0x2e + local_46c * 4)
                  ;
                }
                *(undefined2 *)(local_470 + 0x450) = *(undefined2 *)(local_470 + 0x218);
              }
              if (local_470 == *(int *)(extraout_ECX + 0x4c)) {
                iVar5 = (int)*(short *)(param_1 + 0x2e + local_46c * 4);
                if (*(short *)(extraout_ECX + 0x26e80) == iVar5 ||
                    *(short *)(extraout_ECX + 0x26e80) - iVar5 < 0) {
                  *(undefined2 *)(extraout_ECX + 0x26e80) = 0;
                }
                else {
                  *(short *)(extraout_ECX + 0x26e80) =
                       *(short *)(extraout_ECX + 0x26e80) -
                       (short)((int)*(short *)(param_1 + 0x2e + local_46c * 4) / local_474);
                }
                local_2c = (uint *)0x48f490;
                FUN_0058f220();
              }
            }
            *(undefined2 *)(local_470 + 0x650) = *(undefined2 *)(param_1 + 0xc);
            local_5a0 = 0;
            if (((*(int *)(local_470 + 0x24c) == 0x38) && (*(short *)(local_470 + 0x1d2) == 0)) &&
               ((local_470 != 0 && (local_40 != (uint *)0x0)))) {
              local_2c = (uint *)(*(float *)(local_470 + 0x30) - _DAT_005a3660);
              local_30 = (uint *)((float)local_40[10] * _DAT_005a34a0 +
                                 *(float *)(local_470 + 0x28) * _DAT_005a34a0);
              local_34 = (uint *)0x48f552;
              local_5a0 = FUN_00430d79();
            }
            else {
              local_2c = (uint *)(*(float *)(local_470 + 0x30) + _DAT_005a3660);
              local_30 = *(uint **)(local_470 + 0x28);
              local_34 = (uint *)0x48f597;
              local_5a0 = FUN_00430d79();
            }
            if (local_5a0 != 0) {
              for (local_5a4 = 0; local_5a4 < (int)((*(byte *)(param_1 + 0x1f) & 1) + 1);
                  local_5a4 = local_5a4 + 1) {
                local_628 = FUN_013c0900();
                local_628 = local_628 / (int)((*(byte *)(param_1 + 0x1f) & 1) + 1);
                if (0 < local_628) {
                  iVar5 = __ftol();
                  if (((iVar5 >> 7 < 0x11) || (iVar5 = __ftol(), 0x13 < iVar5 >> 7)) ||
                     (iVar5 = __ftol(), iVar5 >> 7 < 0x1e)) {
                    bVar2 = false;
                  }
                  else {
                    bVar2 = true;
                  }
                  if (bVar2) {
                    iVar5 = __ftol();
                    if ((iVar5 >> 7 == 0x12) && (iVar5 = __ftol(), iVar5 >> 7 == 0x1e)) {
                      bVar2 = true;
                    }
                    else {
                      bVar2 = false;
                    }
                    if (((bVar2) ||
                        (((uint)*(ushort *)(param_1 + 0xc) ==
                          *(uint *)(*(int *)(extraout_ECX + 0x4c) + 0x20) &&
                         (999 < *(uint *)(local_470 + 0x20))))) ||
                       ((*(int *)(local_470 + 0x20) ==
                         *(int *)(*(int *)(extraout_ECX + 0x4c) + 0x20) &&
                        (999 < *(ushort *)(param_1 + 0xc))))) goto LAB_0048f736;
                    puVar14 = (uint *)0x48f731;
                    FUN_0058f078();
                  }
                  else {
LAB_0048f736:
                    puVar14 = local_624;
                    local_2c = (uint *)0x48f74e;
                    FUN_0058f078();
                  }
                  local_638 = 0;
                  local_62c = (uint *)0x3f800000;
                  local_634 = (uint *)0xffffffff;
                  if (*(short *)(param_1 + 0x18) == 0x5f) {
                    puVar14 = puVar14 + local_46c * 0x19;
                  }
                  if (DAT_0092e660 == 0) {
                    local_630 = 1;
                  }
                  else {
                    local_630 = (uint)(local_40 == *(uint **)(extraout_ECX + 0x4c));
                    if (local_470 == *(int *)(extraout_ECX + 0x4c)) {
                      local_630 = 1;
                    }
                  }
                  if (local_630 != 0) {
                    if ((*(byte *)(param_1 + 0x1f) & 2) == 0) {
                      if (local_470 == *(int *)(extraout_ECX + 0x4c)) {
                        pvVar7 = operator_new(0x2a0);
                        local_8 = 0x26;
                        if (pvVar7 == (void *)0x0) {
                          local_f88 = 0;
                        }
                        else {
                          local_2c = (uint *)0x1;
                          local_30 = (uint *)0x5dc;
                          local_34 = local_62c;
                          local_38 = local_634;
                          local_3c = (uint *)0x48f971;
                          iVar5 = __ftol();
                          local_3c = (uint *)0x48f984;
                          iVar10 = __ftol();
                          local_3c = (uint *)(iVar5 + iVar10);
                          local_40 = (uint *)(local_598 + local_5a4 * -10);
                          local_44 = local_624;
                          local_48 = 6.70172e-39;
                          local_f88 = FUN_00422930();
                        }
                        local_638 = local_f88;
                      }
                      else {
                        pvVar7 = operator_new(0x2a0);
                        local_8 = 0x27;
                        if (pvVar7 == (void *)0x0) {
                          local_f90 = 0;
                        }
                        else {
                          FUN_005a2fc2();
                          local_30 = puVar14;
                          local_34 = local_62c;
                          local_38 = local_634;
                          local_3c = (uint *)0x48fa4d;
                          iVar5 = __ftol();
                          local_3c = (uint *)0x48fa60;
                          iVar10 = __ftol();
                          local_3c = (uint *)(iVar5 + iVar10);
                          local_40 = (uint *)(local_598 + local_5a4 * -10);
                          local_44 = local_624;
                          local_48 = 6.702028e-39;
                          local_f90 = FUN_00422930();
                        }
                        local_638 = local_f90;
                      }
                    }
                    else {
                      pvVar7 = operator_new(0x2a0);
                      local_8 = 0x25;
                      if (pvVar7 == (void *)0x0) {
                        local_f80 = 0;
                      }
                      else {
                        local_2c = (uint *)0x1;
                        local_30 = (uint *)0x5dc;
                        local_34 = local_62c;
                        local_38 = local_634;
                        local_3c = (uint *)0x48f884;
                        __ftol();
                        local_3c = (uint *)0x48f895;
                        local_3c = (uint *)__ftol();
                        local_40 = (uint *)(local_598 + local_5a4 * -10);
                        local_44 = local_624;
                        local_48 = 6.701382e-39;
                        local_f80 = FUN_00422930();
                      }
                      local_638 = local_f80;
                    }
                    local_8 = 0xffffffff;
                    if ((((*(int *)(local_470 + 0x24c) == 0x38) &&
                         (*(short *)(local_470 + 0x1d2) == 0)) &&
                        (local_40 != *(uint **)(extraout_ECX + 0x4c))) &&
                       (*(undefined4 *)(local_638 + 0xd4) = 0x3f000000,
                       *(int *)(local_638 + 0xac) == 5)) {
                      *(undefined4 *)(local_638 + 0xac) = 6;
                    }
                    if (local_638 != 0) {
                      FUN_0054ac09();
                    }
                  }
                  if ((local_470 == *(int *)(extraout_ECX + 0x4c)) && (local_40 != (uint *)0x0)) {
                    local_2c = (uint *)0x48fb6c;
                    FUN_0058f078();
                  }
                  if (*(char *)(local_470 + 0x798) == '\x01') {
                    pvVar7 = operator_new(0xcc);
                    local_8 = 0x28;
                    if (pvVar7 == (void *)0x0) {
                      local_f94 = 0;
                    }
                    else {
                      local_2c = (uint *)0xb16;
                      local_30 = (uint *)0x48fbc9;
                      local_f94 = FUN_004d2230();
                    }
                    local_8 = 0xffffffff;
                    local_63c = local_f94;
                    if (local_f94 != 0) {
                      *(undefined4 *)(local_f94 + 0xa4) = 0x19d;
                      *(undefined4 *)(local_f94 + 0xb0) = 500;
                      *(undefined4 *)(local_f94 + 0xb8) = 500;
                      if (*(char *)(local_470 + 0x79c) == '\x01') {
                        local_2c = (uint *)0x48fc9a;
                        FUN_00430f50();
                        FUN_004310a0();
                      }
                      else {
                        local_2c = (uint *)0x48fcfc;
                        FUN_00430f50();
                        FUN_004310a0();
                      }
                      *(undefined4 *)(local_63c + 0x98) = 0x40200000;
                      *(undefined4 *)(local_63c + 0x9c) = 0x40200000;
                      *(undefined4 *)(local_63c + 0x6c) = 1;
                      *(undefined1 *)(local_63c + 0xac) = 0;
                      FUN_0054ac09();
                    }
                  }
                }
              }
            }
          }
          else {
            if (*(char *)(param_1 + 0x1e) == '\0') {
              iVar5 = (int)*(short *)(param_1 + 0x2e + local_46c * 4) / local_474;
              if (*(short *)(local_470 + 0x450) == iVar5 ||
                  *(short *)(local_470 + 0x450) - iVar5 < 0) {
                *(undefined2 *)(local_470 + 0x450) = 0;
              }
              else {
                *(short *)(local_470 + 0x450) =
                     *(short *)(local_470 + 0x450) -
                     (short)((int)*(short *)(param_1 + 0x2e + local_46c * 4) / local_474);
              }
              if (local_470 == *(int *)(extraout_ECX + 0x4c)) {
                iVar5 = (int)*(short *)(param_1 + 0x2e + local_46c * 4);
                if (*(short *)(extraout_ECX + 0x26e80) == iVar5 ||
                    *(short *)(extraout_ECX + 0x26e80) - iVar5 < 0) {
                  *(undefined2 *)(extraout_ECX + 0x26e80) = 0;
                }
                else {
                  *(short *)(extraout_ECX + 0x26e80) =
                       *(short *)(extraout_ECX + 0x26e80) -
                       (short)((int)*(short *)(param_1 + 0x2e + local_46c * 4) / local_474);
                }
                local_2c = (uint *)0x48f05c;
                FUN_0058f220();
              }
            }
            local_2c = (uint *)(*(float *)(local_470 + 0x30) + _DAT_005a3660);
            local_30 = *(uint **)(local_470 + 0x28);
            local_34 = (uint *)0x48f099;
            iVar5 = FUN_00430d79();
            if (iVar5 != 0) {
              iVar5 = __ftol();
              if (((iVar5 >> 7 < 0x11) || (iVar5 = __ftol(), 0x13 < iVar5 >> 7)) ||
                 (iVar5 = __ftol(), iVar5 >> 7 < 0x1e)) {
                bVar2 = false;
              }
              else {
                bVar2 = true;
              }
              if (((bVar2) &&
                  (((uint)*(ushort *)(param_1 + 0xc) !=
                    *(uint *)(*(int *)(extraout_ECX + 0x4c) + 0x20) ||
                   (*(uint *)(local_470 + 0x20) < 1000)))) &&
                 ((*(int *)(local_470 + 0x20) != *(int *)(*(int *)(extraout_ECX + 0x4c) + 0x20) ||
                  (*(ushort *)(param_1 + 0xc) < 1000)))) {
                FUN_0058f078();
              }
              else {
                local_2c = (uint *)0x48f1a3;
                FUN_0058f078();
              }
              if (DAT_0092e660 == 0) {
                local_510 = 1;
              }
              else {
                local_510 = (uint)(local_40 == *(uint **)(extraout_ECX + 0x4c));
                if (local_470 == *(int *)(extraout_ECX + 0x4c)) {
                  local_510 = 1;
                }
              }
              if (local_510 != 0) {
                pvVar7 = operator_new(0x2a0);
                local_8 = 0x24;
                if (pvVar7 == (void *)0x0) {
                  local_f6c = 0;
                }
                else {
                  local_2c = (uint *)0x1;
                  local_30 = (uint *)0x5dc;
                  local_34 = (uint *)0x40000000;
                  local_38 = (uint *)0xff5555ff;
                  local_3c = (uint *)0x48f252;
                  iVar5 = __ftol();
                  local_3c = (uint *)(local_50c + iVar5);
                  local_40 = local_508;
                  local_44 = local_590;
                  local_48 = 6.699132e-39;
                  local_f6c = FUN_00422930();
                }
                local_8 = 0xffffffff;
                local_594 = local_f6c;
                if (local_f6c != 0) {
                  FUN_0054ac09();
                }
              }
            }
            if ((uint)*(ushort *)(param_1 + 0xc) == *(uint *)(*(int *)(extraout_ECX + 0x4c) + 0x20))
            {
              FUN_0048150a();
            }
          }
          if ((*(short *)(local_470 + 0x450) < 0) && (*(char *)(param_1 + 0x1e) == '\0')) {
            *(undefined2 *)(local_470 + 0x450) = 0;
            if (local_470 == *(int *)(extraout_ECX + 0x4c)) {
              local_2c = (uint *)0x48fdba;
              FUN_0058f220();
            }
            *(undefined2 *)(extraout_ECX + 0x26e80) = 0;
          }
          if (((*(short *)(local_470 + 0x44c) < *(short *)(local_470 + 0x450)) &&
              (*(char *)(param_1 + 0x1e) == '\0')) &&
             (*(undefined2 *)(local_470 + 0x450) = *(undefined2 *)(local_470 + 0x44c),
             local_470 == *(int *)(extraout_ECX + 0x4c))) {
            local_2c = (uint *)0x48fe42;
            FUN_0058f220();
          }
          if (((local_470 == *(int *)(extraout_ECX + 0x4c)) && (*(char *)(param_1 + 0x1e) == '\0'))
             && (*(undefined2 *)(DAT_013b71e8 + 0x1ce) = *(undefined2 *)(local_470 + 0x450),
                local_470 == *(int *)(extraout_ECX + 0x4c))) {
            local_2c = (uint *)0x48fead;
            FUN_0058f220();
          }
          fVar6 = 0.0;
          puVar14 = (uint *)0x48febd;
          FUN_004fff0b();
        }
      }
    }
    if ((local_40 != *(uint **)(extraout_ECX + 0x4c)) ||
       ((*(char *)(param_1 + 0x1e) == '\x01' && (local_40 == *(uint **)(extraout_ECX + 0x4c))))) {
      if (*(short *)(param_1 + 0x18) == 0) {
        pvVar7 = operator_new(0x90);
        local_8 = 0x29;
        if (pvVar7 == (void *)0x0) {
          local_f98 = 0;
        }
        else {
          local_2c = local_3c;
          local_30 = (uint *)0x48ff48;
          local_f98 = FUN_004e52b0();
        }
        local_8 = 0xffffffff;
        local_640 = local_f98;
        if ((*(int *)(extraout_ECX + 0x2c) != 0) && (local_f98 != 0)) {
          FUN_0054ac09();
        }
      }
      else if (*(short *)(param_1 + 0x18) == 1) {
        local_64c = fVar6 - _DAT_005a34a0;
        local_650 = puVar14;
        local_648 = local_1c;
        pvVar7 = operator_new(0x90);
        local_8 = 0x2a;
        if (pvVar7 == (void *)0x0) {
          local_f9c = 0;
        }
        else {
          local_2c = local_650;
          local_30 = (uint *)0x490034;
          local_f9c = FUN_004e5650();
        }
        local_8 = 0xffffffff;
        local_644 = local_f9c;
        if ((*(int *)(extraout_ECX + 0x2c) != 0) && (local_f9c != 0)) {
          FUN_0054ac09();
        }
      }
      else if (*(short *)(param_1 + 0x18) == 0x1a) {
        local_664 = fVar6 - _DAT_005a34a0;
        local_668 = puVar14;
        local_660 = local_1c;
        pvVar7 = operator_new(0x90);
        local_8 = 0x2b;
        if (pvVar7 == (void *)0x0) {
          local_fa0 = 0;
        }
        else {
          local_2c = local_668;
          local_30 = (uint *)0x490120;
          local_fa0 = FUN_004e3610();
        }
        local_8 = 0xffffffff;
        local_654 = local_fa0;
        if ((*(int *)(extraout_ECX + 0x2c) != 0) && (local_fa0 != 0)) {
          FUN_0054ac09();
        }
        if (local_40 == (uint *)0x0) {
          ExceptionList = local_10;
          return 1;
        }
        iVar5 = (**(code **)(**(int **)(extraout_ECX + 0x4c) + 0x50))();
        if (iVar5 == 1) {
          ExceptionList = local_10;
          return 1;
        }
        if (((short)local_40[0x1a0] != 0) &&
           (*(short *)(*(int *)(extraout_ECX + 0x4c) + 0x680) != 0)) {
          if (*(short *)(*(int *)(extraout_ECX + 0x4c) + 0x680) == (short)local_40[0x1a0]) {
            ExceptionList = local_10;
            return 1;
          }
          if (*(short *)((int)DAT_013b71e8 + 0xf12) == (short)local_40[0x1a0]) {
            ExceptionList = local_10;
            return 1;
          }
        }
        local_658 = 0;
        for (local_65c = 0; local_65c < 0xd; local_65c = local_65c + 1) {
          if ((uint)*(ushort *)(param_1 + 0x2c + local_65c * 4) ==
              *(uint *)(*(int *)(extraout_ECX + 0x4c) + 0x20)) {
            local_658 = 1;
            break;
          }
        }
        if (((local_658 == 1) && (local_40 != *(uint **)(extraout_ECX + 0x4c))) &&
           ((char)local_40[0x92] == '\0')) {
          if (*(short *)(*(int *)(extraout_ECX + 0x4c) + 0x444) < 1) {
            local_66c = (float)(int)*(short *)(*(int *)(extraout_ECX + 0x4c) + 0x444) *
                        _DAT_005a4420 * (float)*(byte *)((int)local_40 + 0x45d);
          }
          else {
            local_66c = (float)((int)(short)local_40[0x111] /
                               (int)*(short *)(*(int *)(extraout_ECX + 0x4c) + 0x444)) *
                        _DAT_005a4420 * (float)*(byte *)((int)local_40 + 0x45d);
          }
          local_66c = local_66c / _DAT_005a40f8;
          if (local_66c < _DAT_005a441c) {
            local_66c = 2000.0;
          }
          if (_DAT_005a4420 < local_66c) {
            local_66c = 4000.0;
          }
          *(float *)(extraout_ECX + 0x26ed4) = local_66c;
          uVar8 = (**(code **)(*DAT_0092e654 + 8))();
          *(undefined4 *)(extraout_ECX + 0x26ed0) = uVar8;
        }
      }
      else if (*(short *)(param_1 + 0x18) == 7) {
        for (local_670 = 0;
            ((local_670 < 0xd && ((*(short *)(param_1 + 4) != 0x39d || (local_670 < 1)))) &&
            ((*(short *)(param_1 + 4) != 0x39e || (local_670 < 2)))); local_670 = local_670 + 1) {
          local_674 = (**(code **)(*DAT_013b71e8 + 0x34))();
          if (local_674 != 0) {
            local_67c = 0;
            if (local_40 != (uint *)0x0) {
              local_67c = local_40[0xc];
            }
            local_2c = (uint *)0x4904c6;
            FUN_00430f50();
            local_2c = (uint *)0x490501;
            FUN_00430f50();
            pvVar7 = operator_new(0x94);
            local_8 = 0x2c;
            if (pvVar7 == (void *)0x0) {
              local_fb4 = 0;
            }
            else {
              local_2c = (uint *)0x2711;
              local_30 = (uint *)0x0;
              local_3c = local_694;
              local_38 = local_690;
              local_34 = local_68c;
              local_48 = local_688;
              local_44 = local_684;
              local_40 = local_680;
              local_4c = 0x49057a;
              local_fb4 = FUN_004c5cc0();
            }
            local_8 = 0xffffffff;
            local_678 = local_fb4;
            if (local_fb4 != 0) {
              FUN_0054ac09();
            }
          }
        }
      }
      else if (*(short *)(param_1 + 0x18) == 0x23) {
        local_38 = (uint *)((float)local_38 + _DAT_005a3660);
        pvVar7 = operator_new(0xc0);
        local_8 = 0x2d;
        if (pvVar7 == (void *)0x0) {
          local_fb8 = 0;
        }
        else {
          local_30 = local_3c;
          local_2c = local_38;
          local_34 = (uint *)0x0;
          local_38 = (uint *)0x0;
          local_3c = (uint *)0x0;
          local_40 = (uint *)0x49063d;
          pfVar9 = (float *)FUN_00430f50();
          local_3c = (uint *)*pfVar9;
          local_38 = (uint *)pfVar9[1];
          local_34 = (uint *)pfVar9[2];
          local_40 = (uint *)0x49065d;
          local_fb8 = FUN_004e9cf0();
        }
        local_8 = 0xffffffff;
        local_698 = local_fb8;
        if ((*(int *)(extraout_ECX + 0x2c) != 0) && (local_fb8 != 0)) {
          FUN_0054ac09();
        }
      }
      else if (*(short *)(param_1 + 0x18) == 0x27) {
        local_38 = (uint *)((float)local_38 + _DAT_005a3660);
        local_2c = (uint *)0x4906ea;
        FUN_00430f50();
        for (local_6ac = 0; (int)local_6ac < 4; local_6ac = local_6ac + 1) {
          local_6b0 = local_6ac;
          pvVar7 = operator_new(0xc0);
          local_8 = 0x2e;
          if (pvVar7 == (void *)0x0) {
            local_fc4 = 0;
          }
          else {
            local_2c = local_38;
            uVar11 = local_6b0 & 0x80000001;
            if ((int)uVar11 < 0) {
              uVar11 = (uVar11 - 1 | 0xfffffffe) + 1;
            }
            local_30 = (uint *)((float)(int)uVar11 * _DAT_005a4414 +
                               ((float)local_3c - _DAT_005a4418));
            local_34 = (uint *)0x4907b8;
            pfVar9 = (float *)FUN_00430f50();
            local_30 = (uint *)*pfVar9;
            local_2c = (uint *)pfVar9[1];
            local_3c = local_6a8;
            local_38 = local_6a4;
            local_34 = local_6a0;
            local_40 = (uint *)0x4907f7;
            local_fc4 = FUN_004e9cf0();
          }
          local_8 = 0xffffffff;
          local_6b4 = local_fc4;
          if ((*(int *)(extraout_ECX + 0x2c) != 0) && (local_fc4 != 0)) {
            *(uint *)(local_fc4 + 0x80) = *(int *)(local_fc4 + 0x80) + local_6ac * 200;
            FUN_0054ac09();
          }
        }
        pvVar7 = operator_new(0xc0);
        local_8 = 0x2f;
        if (pvVar7 == (void *)0x0) {
          local_fc8 = 0;
        }
        else {
          local_2c = local_38;
          local_30 = local_3c;
          local_34 = (uint *)0x4908bb;
          pfVar9 = (float *)FUN_00430f50();
          local_30 = (uint *)*pfVar9;
          local_2c = (uint *)pfVar9[1];
          local_3c = local_6a8;
          local_38 = local_6a4;
          local_34 = local_6a0;
          local_40 = (uint *)0x4908fa;
          local_fc8 = FUN_004e9cf0();
        }
        local_8 = 0xffffffff;
        local_69c = local_fc8;
        if ((*(int *)(extraout_ECX + 0x2c) != 0) && (local_fc8 != 0)) {
          *(int *)(local_fc8 + 0x80) = *(int *)(local_fc8 + 0x80) + 0x10e;
          FUN_0054ac09();
        }
      }
      else if (*(short *)(param_1 + 0x18) == 0x61) {
        local_38 = (uint *)((float)local_38 + _DAT_005a3660);
        local_6bc = 0;
        do {
          if (0x5f < local_6bc) {
LAB_00490a9f:
            pvVar7 = operator_new(0xc0);
            local_8 = 0x30;
            if (pvVar7 == (void *)0x0) {
              local_fcc = 0;
            }
            else {
              local_30 = local_3c;
              local_2c = local_38;
              local_3c = (uint *)0x2;
              local_38 = (uint *)0x0;
              local_34 = local_1c;
              local_40 = (uint *)0x490afd;
              local_fcc = FUN_004e9cf0();
            }
            local_8 = 0xffffffff;
            local_6b8 = local_fcc;
            if (*(int *)(extraout_ECX + 0x2c) == 0) {
              ExceptionList = local_10;
              return 1;
            }
            if (local_fcc == 0) {
              ExceptionList = local_10;
              return 1;
            }
            FUN_0054ac09();
            ExceptionList = local_10;
            return 1;
          }
          local_6c0 = (**(code **)(*DAT_013b71e8 + 0x38))();
          if ((local_6c0 != 0) && (*(short *)(local_6c0 + 0x160) == 0x2ea)) {
            local_6c4 = local_6c0;
            iVar5 = __ftol();
            iVar10 = __ftol();
            if (iVar5 == iVar10) {
              iVar5 = __ftol();
              iVar10 = __ftol();
              if (iVar5 == iVar10) {
                *(undefined1 *)(local_6c4 + 0x2fd) = 1;
                local_1c = (uint *)((float)local_1c -
                                   *(float *)(local_6c4 + 0x2ec) * *(float *)(local_6c4 + 0x2f8));
                goto LAB_00490a9f;
              }
            }
          }
          local_6bc = local_6bc + 1;
        } while( true );
      }
    }
  }
  else {
    for (local_6c8 = 0;
        ((local_6c8 < 0xd && ((*(short *)(param_1 + 4) != 0x39d || (local_6c8 < 1)))) &&
        ((*(short *)(param_1 + 4) != 0x39e || (local_6c8 < 2)))); local_6c8 = local_6c8 + 1) {
      local_6cc = (**(code **)(*DAT_013b71e8 + 0x34))();
      if (local_6cc != 0) {
        local_764 = (uint)*(char *)(local_6cc + 0x7a9);
        if (*(char *)(local_6cc + 0x7a9) == '\0') {
          local_764 = 1;
        }
        if ((local_40 != *(uint **)(extraout_ECX + 0x4c)) ||
           ((*(char *)(param_1 + 0x1e) == '\0' && (local_40 == *(uint **)(extraout_ECX + 0x4c))))) {
          if ((*(short *)(param_1 + 0x2e + local_6c8 * 4) == -3) ||
             (*(short *)(param_1 + 0x2e + local_6c8 * 4) == -4)) {
            local_2c = (uint *)(*(float *)(local_6cc + 0x30) + _DAT_005a3660);
            local_30 = *(uint **)(local_6cc + 0x28);
            local_34 = (uint *)0x490cc2;
            iVar5 = FUN_00430d79();
            if (iVar5 != 0) {
              if (DAT_0092e660 == 0) {
                local_774 = 1;
              }
              else {
                local_774 = (uint)(local_40 == *(uint **)(extraout_ECX + 0x4c));
                if (local_6cc == *(int *)(extraout_ECX + 0x4c)) {
                  local_774 = 1;
                }
              }
              if (local_774 != 0) {
                FUN_0058f078();
                pvVar7 = operator_new(0x2a0);
                local_8 = 0x31;
                if (pvVar7 == (void *)0x0) {
                  local_fd0 = 0;
                }
                else {
                  local_2c = (uint *)0x1;
                  local_30 = (uint *)0x5dc;
                  local_34 = (uint *)0x40000000;
                  local_38 = (uint *)0xffffffff;
                  local_3c = (uint *)0x490d8a;
                  iVar5 = __ftol();
                  local_3c = (uint *)(local_770 + iVar5);
                  local_40 = local_76c;
                  local_44 = local_7f4;
                  local_48 = 6.708896e-39;
                  local_fd0 = FUN_00422930();
                }
                local_8 = 0xffffffff;
                local_7f8 = local_fd0;
                if (local_fd0 != 0) {
                  FUN_0054ac09();
                }
              }
            }
          }
          else if ((((*(short *)(param_1 + 0x18) < 0) || (0x67 < *(short *)(param_1 + 0x18))) ||
                   (*(int *)(&DAT_0092ff0c + *(short *)(param_1 + 0x18) * 0x60) != 6)) ||
                  ((*(short *)(param_1 + 0x2e + local_6c8 * 4) < 0 &&
                   (-6 < *(short *)(param_1 + 0x2e + local_6c8 * 4))))) {
            if (*(char *)(param_1 + 0x1e) == '\0') {
              local_898 = (int)*(short *)(param_1 + 0x2e + local_6c8 * 4);
              if (-1 < local_898) {
                if (*(int *)(local_6cc + 0x21c) == 0) {
                  local_898 = local_898 / (int)local_764;
                  *(short *)(local_6cc + 0x450) = *(short *)(local_6cc + 0x450) - (short)local_898;
                }
                else {
                  *(int *)(local_6cc + 0x218) = *(int *)(local_6cc + 0x218) - local_898;
                  *(short *)(local_6cc + 0x450) = (short)(*(uint *)(local_6cc + 0x218) / local_764);
                }
              }
              if (*(short *)(local_6cc + 0x450) < 0) {
                *(undefined2 *)(local_6cc + 0x450) = 0;
              }
              if (local_6cc == *(int *)(extraout_ECX + 0x4c)) {
                iVar5 = (int)*(short *)(param_1 + 0x2e + local_6c8 * 4);
                if (*(short *)(extraout_ECX + 0x26e80) == iVar5 ||
                    *(short *)(extraout_ECX + 0x26e80) - iVar5 < 0) {
                  *(undefined2 *)(extraout_ECX + 0x26e80) = 0;
                }
                else {
                  *(short *)(extraout_ECX + 0x26e80) =
                       *(short *)(extraout_ECX + 0x26e80) - (short)local_898;
                }
                local_2c = (uint *)0x491409;
                FUN_0058f220();
              }
            }
            *(undefined2 *)(local_6cc + 0x650) = *(undefined2 *)(param_1 + 0xc);
            local_894 = 0;
            if (((*(int *)(local_6cc + 0x24c) == 0x38) && (*(short *)(local_6cc + 0x1d2) == 0)) &&
               ((local_6cc != 0 && (local_40 != (uint *)0x0)))) {
              pcVar13 = (char *)((float)local_40[0xb] * _DAT_005a34a0 +
                                *(float *)(local_6cc + 0x2c) * _DAT_005a34a0);
              local_2c = (uint *)(*(float *)(local_6cc + 0x30) - _DAT_005a3660);
              local_30 = (uint *)((float)local_40[10] * _DAT_005a34a0 +
                                 *(float *)(local_6cc + 0x28) * _DAT_005a34a0);
              local_34 = (uint *)0x4914cb;
              local_894 = FUN_00430d79();
            }
            else {
              pcVar13 = *(char **)(local_6cc + 0x2c);
              local_2c = (uint *)(*(float *)(local_6cc + 0x30) + _DAT_005a3660);
              local_30 = *(uint **)(local_6cc + 0x28);
              local_34 = (uint *)0x491510;
              local_894 = FUN_00430d79();
            }
            if (local_894 != 0) {
              for (local_89c = 0; local_89c < (int)((*(byte *)(param_1 + 0x1f) & 1) + 1);
                  local_89c = local_89c + 1) {
                local_8a0 = 0;
                if ((*(byte *)(param_1 + 0x1f) & 4) != 0) {
                  local_8a0 = (int)*(short *)(param_1 + 0x32);
                }
                iVar5 = FUN_013c0a00();
                if (0 < (iVar5 - local_8a0) / (int)((*(byte *)(param_1 + 0x1f) & 1) + 1)) {
                  iVar5 = __ftol();
                  if (((iVar5 >> 7 < 0x11) || (iVar5 = __ftol(), 0x13 < iVar5 >> 7)) ||
                     (iVar5 = __ftol(), iVar5 >> 7 < 0x1e)) {
                    bVar2 = false;
                  }
                  else {
                    bVar2 = true;
                  }
                  if (bVar2) {
                    iVar5 = __ftol();
                    if ((iVar5 >> 7 == 0x12) && (iVar5 = __ftol(), iVar5 >> 7 == 0x1e)) {
                      bVar2 = true;
                    }
                    else {
                      bVar2 = false;
                    }
                    if (((bVar2) ||
                        (((uint)*(ushort *)(param_1 + 0xc) ==
                          *(uint *)(*(int *)(extraout_ECX + 0x4c) + 0x20) &&
                         (999 < *(uint *)(local_6cc + 0x20))))) ||
                       ((*(int *)(local_6cc + 0x20) ==
                         *(int *)(*(int *)(extraout_ECX + 0x4c) + 0x20) &&
                        (999 < *(ushort *)(param_1 + 0xc))))) goto LAB_004916dd;
                    pcVar13 = (char *)0x4916d8;
                    FUN_0058f078();
                  }
                  else {
LAB_004916dd:
                    if (((*(byte *)(param_1 + 0x1f) & 4) == 0) || (local_89c != 0)) {
                      pcVar13 = (char *)local_920;
                      local_2c = (uint *)0x49173c;
                      FUN_0058f078();
                    }
                    else if (0 < *(short *)(param_1 + 0x32)) {
                      pcVar13 = s__d____d_005b88fc;
                      local_2c = local_920;
                      local_30 = (uint *)0x49171f;
                      FUN_0058f078();
                    }
                  }
                  if (DAT_0092e660 == 0) {
                    bVar2 = true;
                  }
                  else {
                    bVar2 = local_6cc == *(int *)(extraout_ECX + 0x4c) ||
                            local_40 == *(uint **)(extraout_ECX + 0x4c);
                  }
                  if ((((*(short *)(param_1 + 0x14) == 0x469) &&
                       (*(short *)(param_1 + 0x16) == 0x6ab)) ||
                      ((*(short *)(param_1 + 0x14) == 0x45c && (*(short *)(param_1 + 0x16) == 0x6ab)
                       ))) || ((*(short *)(param_1 + 0x14) == 0x446 &&
                               (*(short *)(param_1 + 0x16) == 0x69a)))) {
                    bVar2 = false;
                  }
                  if (bVar2) {
                    if ((local_6c8 < 1) ||
                       (*(short *)(param_1 + 0x2c + local_6c8 * 4) !=
                        *(short *)(param_1 + 0x28 + local_6c8 * 4))) {
                      if ((*(byte *)(param_1 + 0x1f) & 2) == 0) {
                        if (local_6cc == *(int *)(extraout_ECX + 0x4c)) {
                          pvVar7 = operator_new(0x2a0);
                          local_8 = 0x37;
                          if (pvVar7 == (void *)0x0) {
                            local_1008 = 0;
                          }
                          else {
                            local_30 = (uint *)(local_89c * 1000 + 1000);
                            local_2c = (uint *)0x1;
                            local_34 = (uint *)0x3f800000;
                            local_38 = (uint *)0xffffffff;
                            local_3c = (uint *)0x491d28;
                            pcVar13 = (char *)local_30;
                            iVar5 = __ftol();
                            local_3c = (uint *)0x491d3b;
                            iVar10 = __ftol();
                            local_3c = (uint *)(iVar5 + iVar10);
                            local_40 = local_88c;
                            local_44 = local_920;
                            local_48 = 6.714516e-39;
                            local_1008 = FUN_00422930();
                          }
                          local_934 = local_1008;
                        }
                        else {
                          pvVar7 = operator_new(0x2a0);
                          local_8 = 0x38;
                          if (pvVar7 != (void *)0x0) {
                            lVar1 = FUN_005a2fc2();
                            (**(code **)((int)(lVar1 / (longlong)unaff_EDI) + -0x75))();
                    /* WARNING: Bad instruction - Truncating control flow here */
                            halt_baddata();
                          }
                          local_934 = 0;
                        }
                      }
                      else {
                        pvVar7 = operator_new(0x2a0);
                        local_8 = 0x36;
                        if (pvVar7 == (void *)0x0) {
                          local_1000 = 0;
                        }
                        else {
                          local_30 = (uint *)((*(byte *)(param_1 + 0x1f) & 2) * 0x226 + 1000 +
                                             local_89c * 1000);
                          local_2c = (uint *)0x9;
                          local_34 = (uint *)0x3f800000;
                          local_38 = (uint *)0xffffffff;
                          local_3c = (uint *)0x491c22;
                          pcVar13 = (char *)local_30;
                          iVar5 = __ftol();
                          local_3c = (uint *)0x491c35;
                          iVar10 = __ftol();
                          local_3c = (uint *)(iVar5 + iVar10);
                          local_40 = local_88c;
                          local_44 = local_920;
                          local_48 = 6.714149e-39;
                          local_1000 = FUN_00422930();
                        }
                        local_934 = local_1000;
                      }
                    }
                    else if ((*(byte *)(param_1 + 0x1f) & 2) == 0) {
                      if (local_6cc == *(int *)(extraout_ECX + 0x4c)) {
                        pvVar7 = operator_new(0x2a0);
                        local_8 = 0x34;
                        if (pvVar7 == (void *)0x0) {
                          local_ff0 = 0;
                        }
                        else {
                          pcVar13 = (char *)0x4b0;
                          local_2c = (uint *)0x1;
                          local_30 = (uint *)0x4b0;
                          local_34 = (uint *)0x3f800000;
                          local_38 = (uint *)0xffffffff;
                          local_3c = (uint *)0x491a0b;
                          iVar5 = __ftol();
                          local_3c = (uint *)0x491a1e;
                          iVar10 = __ftol();
                          local_3c = (uint *)(iVar5 + iVar10);
                          local_40 = (uint *)((int)local_88c + local_89c * -10 + 0x14);
                          local_44 = local_920;
                          local_48 = 6.713419e-39;
                          local_ff0 = FUN_00422930();
                        }
                        local_934 = local_ff0;
                      }
                      else {
                        pvVar7 = operator_new(0x2a0);
                        local_8 = 0x35;
                        if (pvVar7 == (void *)0x0) {
                          local_ff8 = 0;
                        }
                        else {
                          lVar1 = FUN_005a2fc2();
                          uVar8 = (**(code **)((int)(lVar1 % (longlong)unaff_EDI) + 0x68))();
                          local_30 = (uint *)pcVar13;
                          pcVar13 = (char *)CONCAT31((int3)((uint)uVar8 >> 8),4);
                          *pcVar13 = *pcVar13 + '\x04';
                          local_2c = (uint *)0x1;
                          local_34 = (uint *)0x3f800000;
                          local_38 = (uint *)0xffffffff;
                          local_3c = (uint *)0x491afd;
                          pcVar13 = (char *)local_30;
                          iVar5 = __ftol();
                          local_3c = (uint *)0x491b10;
                          iVar10 = __ftol();
                          local_3c = (uint *)(iVar5 + iVar10);
                          local_40 = (uint *)((int)local_88c + local_89c * -10 + 0x14);
                          local_44 = local_920;
                          local_48 = 6.713758e-39;
                          local_ff8 = FUN_00422930();
                        }
                        local_934 = local_ff8;
                      }
                    }
                    else {
                      pvVar7 = operator_new(0x2a0);
                      local_8 = 0x33;
                      if (pvVar7 == (void *)0x0) {
                        local_fe8 = 0;
                      }
                      else {
                        local_30 = (uint *)((*(byte *)(param_1 + 0x1f) & 2) * 300 + 0x4b0);
                        local_2c = (uint *)0x9;
                        local_34 = (uint *)0x3f800000;
                        local_38 = (uint *)0xffffffff;
                        local_3c = (uint *)0x491904;
                        pcVar13 = (char *)local_30;
                        iVar5 = __ftol();
                        local_3c = (uint *)0x491917;
                        iVar10 = __ftol();
                        local_3c = (uint *)(iVar5 + iVar10);
                        local_40 = (uint *)((int)local_88c + local_89c * -10 + 0x14);
                        local_44 = local_920;
                        local_48 = 6.71305e-39;
                        local_fe8 = FUN_00422930();
                      }
                      local_934 = local_fe8;
                    }
                    local_8 = 0xffffffff;
                    if ((((*(int *)(local_6cc + 0x24c) == 0x38) &&
                         (*(short *)(local_6cc + 0x1d2) == 0)) &&
                        (local_40 != *(uint **)(extraout_ECX + 0x4c))) &&
                       (*(undefined4 *)(local_934 + 0xd4) = 0x3f000000,
                       *(int *)(local_934 + 0xac) == 5)) {
                      *(undefined4 *)(local_934 + 0xac) = 6;
                    }
                    if (local_934 != 0) {
                      FUN_0054ac09();
                    }
                  }
                  if ((local_6cc == *(int *)(extraout_ECX + 0x4c)) && (local_40 != (uint *)0x0)) {
                    pcVar13 = (char *)(extraout_ECX + 0x275e8);
                    local_2c = (uint *)0x491f2f;
                    FUN_0058f078();
                  }
                  if (*(char *)(local_6cc + 0x798) == '\x01') {
                    pvVar7 = operator_new(0xcc);
                    local_8 = 0x39;
                    if (pvVar7 == (void *)0x0) {
                      local_1014 = 0;
                    }
                    else {
                      pcVar13 = (char *)0xff999999;
                      local_2c = (uint *)0xb16;
                      local_30 = (uint *)0x491f8c;
                      local_1014 = FUN_004d2230();
                    }
                    local_8 = 0xffffffff;
                    if (local_1014 != 0) {
                      *(undefined4 *)(local_1014 + 0xa4) = 0x19d;
                      *(undefined4 *)(local_1014 + 0xb0) = 500;
                      *(undefined4 *)(local_1014 + 0xb8) = 500;
                      if (*(char *)(local_6cc + 0x79c) == '\x01') {
                        pcVar13 = *(char **)(local_6cc + 0x50);
                        local_2c = (uint *)0x49205d;
                        FUN_00430f50();
                        FUN_004310a0();
                      }
                      else {
                        pcVar13 = *(char **)(local_6cc + 0x28);
                        local_2c = (uint *)0x4920bf;
                        FUN_00430f50();
                        FUN_004310a0();
                      }
                      *(undefined4 *)(local_1014 + 0x98) = 0x40200000;
                      *(undefined4 *)(local_1014 + 0x9c) = 0x40200000;
                      *(undefined4 *)(local_1014 + 0x6c) = 1;
                      *(undefined1 *)(local_1014 + 0xac) = 0;
                      FUN_0054ac09();
                    }
                  }
                }
              }
            }
          }
          else {
            if (*(char *)(param_1 + 0x1e) == '\0') {
              if (*(int *)(local_6cc + 0x21c) == 0) {
                iVar5 = (int)*(short *)(param_1 + 0x2e + local_6c8 * 4) / (int)local_764;
                if (*(short *)(local_6cc + 0x450) == iVar5 ||
                    *(short *)(local_6cc + 0x450) - iVar5 < 0) {
                  *(undefined2 *)(local_6cc + 0x450) = 0;
                }
                else {
                  *(short *)(local_6cc + 0x450) =
                       *(short *)(local_6cc + 0x450) -
                       (short)((int)*(short *)(param_1 + 0x2e + local_6c8 * 4) / (int)local_764);
                }
              }
              else {
                if (*(int *)(local_6cc + 0x218) == (int)*(short *)(param_1 + 0x2e + local_6c8 * 4))
                {
                  *(undefined4 *)(local_6cc + 0x218) = 0;
                }
                else {
                  *(int *)(local_6cc + 0x218) =
                       *(int *)(local_6cc + 0x218) - (int)*(short *)(param_1 + 0x2e + local_6c8 * 4)
                  ;
                }
                if (local_764 == 0) {
                  local_764 = 1;
                }
                *(short *)(local_6cc + 0x450) = (short)(*(uint *)(local_6cc + 0x218) / local_764);
              }
              if (local_6cc == *(int *)(extraout_ECX + 0x4c)) {
                iVar5 = (int)*(short *)(param_1 + 0x2e + local_6c8 * 4);
                if (*(short *)(extraout_ECX + 0x26e80) == iVar5 ||
                    *(short *)(extraout_ECX + 0x26e80) - iVar5 < 0) {
                  *(undefined2 *)(extraout_ECX + 0x26e80) = 0;
                }
                else {
                  *(short *)(extraout_ECX + 0x26e80) =
                       *(short *)(extraout_ECX + 0x26e80) -
                       (short)((int)*(short *)(param_1 + 0x2e + local_6c8 * 4) / (int)local_764);
                }
                local_2c = (uint *)0x491015;
                FUN_0058f220();
              }
            }
            local_2c = (uint *)(*(float *)(local_6cc + 0x30) + _DAT_005a3660);
            local_30 = *(uint **)(local_6cc + 0x28);
            local_34 = (uint *)0x491052;
            iVar5 = FUN_00430d79();
            if (iVar5 != 0) {
              iVar5 = __ftol();
              if (((iVar5 >> 7 < 0x11) || (iVar5 = __ftol(), 0x13 < iVar5 >> 7)) ||
                 (iVar5 = __ftol(), iVar5 >> 7 < 0x1e)) {
                bVar2 = false;
              }
              else {
                bVar2 = true;
              }
              if (((bVar2) &&
                  (((uint)*(ushort *)(param_1 + 0xc) !=
                    *(uint *)(*(int *)(extraout_ECX + 0x4c) + 0x20) ||
                   (*(uint *)(local_6cc + 0x20) < 1000)))) &&
                 ((*(int *)(local_6cc + 0x20) != *(int *)(*(int *)(extraout_ECX + 0x4c) + 0x20) ||
                  (*(ushort *)(param_1 + 0xc) < 1000)))) {
                FUN_0058f078();
              }
              else {
                local_2c = (uint *)0x49115c;
                FUN_0058f078();
              }
              if (DAT_0092e660 == 0) {
                local_804 = 1;
              }
              else {
                local_804 = (uint)(local_40 == *(uint **)(extraout_ECX + 0x4c));
                if (local_6cc == *(int *)(extraout_ECX + 0x4c)) {
                  local_804 = 1;
                }
              }
              if (local_804 != 0) {
                pvVar7 = operator_new(0x2a0);
                local_8 = 0x32;
                if (pvVar7 == (void *)0x0) {
                  local_fd8 = 0;
                }
                else {
                  local_2c = (uint *)0x1;
                  local_30 = (uint *)0x5dc;
                  local_34 = (uint *)0x40000000;
                  local_38 = (uint *)0xff5555ff;
                  local_3c = (uint *)0x49120b;
                  iVar5 = __ftol();
                  local_3c = (uint *)(local_800 + iVar5);
                  local_40 = local_7fc;
                  local_44 = local_884;
                  local_48 = 6.710511e-39;
                  local_fd8 = FUN_00422930();
                }
                local_8 = 0xffffffff;
                local_888 = local_fd8;
                if (local_fd8 != 0) {
                  FUN_0054ac09();
                }
              }
            }
            if ((uint)*(ushort *)(param_1 + 0xc) == *(uint *)(*(int *)(extraout_ECX + 0x4c) + 0x20))
            {
              FUN_0048150a();
            }
          }
          if ((*(short *)(local_6cc + 0x450) < 0) && (*(char *)(param_1 + 0x1e) == '\0')) {
            *(undefined2 *)(local_6cc + 0x450) = 0;
            *(undefined2 *)(extraout_ECX + 0x26e80) = 0;
          }
          if ((*(short *)(local_6cc + 0x44c) < *(short *)(local_6cc + 0x450)) &&
             (*(char *)(param_1 + 0x1e) == '\0')) {
            *(undefined2 *)(local_6cc + 0x450) = *(undefined2 *)(local_6cc + 0x44c);
          }
          if ((local_6cc == *(int *)(extraout_ECX + 0x4c)) && (*(char *)(param_1 + 0x1e) == '\0')) {
            *(undefined2 *)(DAT_013b71e8 + 0x1ce) = *(undefined2 *)(local_6cc + 0x450);
          }
          FUN_004fff0b();
          local_768 = *(int *)(extraout_ECX + 0x2872c);
          if (*(int *)(extraout_ECX + 0x2872c) != 0) {
            for (local_93c = 0; local_93c < *(int *)(local_768 + 0x210); local_93c = local_93c + 1)
            {
              iVar5 = *(int *)(local_768 + 0x214 + local_93c * 4);
              if (*(int *)(iVar5 + 0xe50) == *(int *)(local_6cc + 0x20)) {
                (**(code **)(**(int **)(iVar5 + 0xe68) + 0x8c))();
                (**(code **)(**(int **)(iVar5 + 0xe68) + 0x88))();
                break;
              }
            }
          }
        }
        if ((((-1 < *(short *)(param_1 + 0x18)) && (*(short *)(param_1 + 0x18) < 0x68)) &&
            (*(short *)(param_1 + 0x18) != 0x1d)) &&
           (*(int *)(&DAT_0092ff0c + *(short *)(param_1 + 0x18) * 0x60) == 8)) {
          ExceptionList = local_10;
          return 1;
        }
        if ((local_40 != (uint *)0x0) && (local_40[0x17] == 0xb)) {
          local_30 = (uint *)0x4;
        }
        local_760[0] = 0;
        local_760[1] = 0;
        local_760[2] = 0;
        local_760[3] = 0;
        local_760[4] = 0;
        local_760[5] = 0;
        local_760[6] = 0;
        local_760[7] = 0;
        local_760[8] = 0;
        local_760[9] = 1000;
        local_760[10] = 1000;
        local_760[0xb] = 1000;
        local_760[0xc] = 0;
        local_760[0xd] = 0;
        local_760[0xe] = 0;
        local_760[0xf] = 0;
        local_760[0x10] = 0;
        local_760[0x11] = 0;
        local_760[0x12] = 0;
        local_760[0x13] = 0;
        local_760[0x14] = 0;
        local_760[0x15] = 0;
        local_760[0x16] = 0;
        local_760[0x17] = 0;
        local_760[0x18] = 700;
        local_760[0x19] = 700;
        local_760[0x1a] = 700;
        local_760[0x1b] = 1000;
        local_760[0x1c] = 1000;
        local_760[0x1d] = 1000;
        local_760[0x1e] = 1000;
        local_760[0x1f] = 1000;
        local_760[0x20] = 1000;
        local_760[0x21] = 1000;
        local_760[0x22] = 1000;
        local_760[0x23] = 1000;
        if (9 < *(byte *)(param_1 + 0x1c)) {
          *(undefined1 *)(param_1 + 0x1c) = 9;
        }
        local_760[0x24] = *(byte *)(param_1 + 0x1c) - 4;
        if (*(short *)(param_1 + 0x18) == 0x97) {
          local_30 = (uint *)0x5;
        }
        if ((local_40 != *(uint **)(extraout_ECX + 0x4c)) ||
           ((*(char *)(param_1 + 0x1e) == '\0' && (local_40 == *(uint **)(extraout_ECX + 0x4c))))) {
          *(int *)(local_6cc + 0x4b0) =
               (int)*(short *)(param_1 + 0x2e + local_6c8 * 4) / (int)local_764;
          FUN_0040bd30();
          iVar5 = (**(code **)(*DAT_0092e654 + 8))();
          *(int *)(local_6cc + 0x4ac) =
               (int)local_44 + iVar5 + local_760[(int)local_30 * 6 + local_760[0x24]];
        }
      }
    }
  }
  if (*(char *)(param_1 + 0x1e) == '\0') {
    if (local_40 == *(uint **)(extraout_ECX + 0x4c)) {
      *(undefined2 *)(extraout_ECX + 0x26e82) = *(undefined2 *)(param_1 + 0x28);
      if (*(short *)(extraout_ECX + 0x26e80) < *(short *)(*(int *)(extraout_ECX + 0x4c) + 0x450)) {
        *(undefined2 *)(extraout_ECX + 0x26e80) =
             *(undefined2 *)(*(int *)(extraout_ECX + 0x4c) + 0x450);
      }
      if (*(short *)(extraout_ECX + 0x26e82) < *(short *)(*(int *)(extraout_ECX + 0x4c) + 0x452)) {
        *(undefined2 *)(extraout_ECX + 0x26e82) =
             *(undefined2 *)(*(int *)(extraout_ECX + 0x4c) + 0x452);
      }
      if (*(short *)(*(int *)(extraout_ECX + 0x4c) + 0x44c) < *(short *)(extraout_ECX + 0x26e80)) {
        *(undefined2 *)(extraout_ECX + 0x26e80) =
             *(undefined2 *)(*(int *)(extraout_ECX + 0x4c) + 0x44c);
      }
      if (*(short *)(*(int *)(extraout_ECX + 0x4c) + 0x44e) < *(short *)(extraout_ECX + 0x26e82)) {
        *(undefined2 *)(extraout_ECX + 0x26e82) =
             *(undefined2 *)(*(int *)(extraout_ECX + 0x4c) + 0x44e);
      }
      FUN_004431e4();
    }
    else if (local_2c == *(uint **)(extraout_ECX + 0x4c)) {
      if (*(short *)(extraout_ECX + 0x26e80) < *(short *)(*(int *)(extraout_ECX + 0x4c) + 0x450)) {
        *(undefined2 *)(extraout_ECX + 0x26e80) =
             *(undefined2 *)(*(int *)(extraout_ECX + 0x4c) + 0x450);
      }
      if (*(short *)(extraout_ECX + 0x26e82) < *(short *)(*(int *)(extraout_ECX + 0x4c) + 0x452)) {
        *(undefined2 *)(extraout_ECX + 0x26e82) =
             *(undefined2 *)(*(int *)(extraout_ECX + 0x4c) + 0x452);
      }
      if (*(short *)(*(int *)(extraout_ECX + 0x4c) + 0x44c) < *(short *)(extraout_ECX + 0x26e80)) {
        *(undefined2 *)(extraout_ECX + 0x26e80) =
             *(undefined2 *)(*(int *)(extraout_ECX + 0x4c) + 0x44c);
      }
      if (*(short *)(*(int *)(extraout_ECX + 0x4c) + 0x44e) < *(short *)(extraout_ECX + 0x26e82)) {
        *(undefined2 *)(extraout_ECX + 0x26e82) =
             *(undefined2 *)(*(int *)(extraout_ECX + 0x4c) + 0x44e);
      }
      FUN_004431e4();
    }
  }
  ExceptionList = local_10;
  return 1;
}

