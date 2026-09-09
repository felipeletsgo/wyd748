// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00517231 | Name: FUN_00517231


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int * __thiscall FUN_00517231(int *param_1,int param_2)

{
  float fVar1;
  float fVar2;
  bool bVar3;
  int iVar4;
  int *piVar5;
  uint uVar6;
  undefined4 *puVar7;
  void *pvVar8;
  float10 fVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined *puVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  undefined4 uVar15;
  undefined4 uVar16;
  undefined4 uVar17;
  undefined4 uVar18;
  undefined4 uVar19;
  int local_314;
  int *local_310;
  int *local_30c;
  int local_308;
  int *local_304;
  int local_300;
  int *local_2f8;
  int *local_2f4;
  int *local_2f0;
  int *local_2ec;
  int local_2e8;
  int local_2e4;
  int *local_2e0;
  int local_2dc;
  int local_2c4;
  undefined1 local_2b4 [12];
  undefined1 local_2a8 [12];
  void *local_29c;
  int *local_298;
  undefined1 local_294 [12];
  undefined1 local_288 [12];
  void *local_27c;
  int *local_278;
  undefined1 local_274 [12];
  undefined1 local_268 [12];
  void *local_25c;
  int local_258;
  void *local_24c;
  int *local_248;
  void *local_244;
  int local_240;
  undefined1 local_23c [12];
  undefined1 local_230 [12];
  void *local_224;
  int *local_220;
  void *local_21c;
  int *local_218;
  undefined1 local_214 [12];
  undefined1 local_208 [12];
  void *local_1fc;
  int *local_1f8;
  void *local_1f4;
  int *local_1f0;
  undefined1 local_1ec [12];
  undefined1 local_1e0 [12];
  void *local_1d4;
  int local_1d0;
  undefined1 local_1cc [12];
  undefined1 local_1c0 [12];
  void *local_1b4;
  int local_1b0;
  void *local_1a4;
  int *local_1a0;
  void *local_19c;
  int local_198;
  void *local_194;
  int local_190;
  int local_18c;
  int *local_188;
  int local_184;
  int *local_180;
  undefined4 local_17c;
  undefined4 local_178;
  undefined4 local_174;
  int *local_170;
  int local_16c;
  float local_168;
  undefined4 local_164;
  float local_160;
  int *local_15c;
  float local_158;
  float local_154 [2];
  float local_14c;
  float local_148;
  int local_144;
  undefined1 local_140 [12];
  undefined1 local_134 [4];
  float local_130;
  undefined4 local_128;
  undefined *local_124;
  undefined4 local_120;
  float local_11c;
  float local_118;
  undefined4 local_114;
  float local_110;
  undefined *local_10c;
  undefined2 local_108 [4];
  int local_100;
  int *local_fc;
  int local_f8;
  undefined2 local_f4 [4];
  int *local_ec;
  int *local_e8;
  int local_e4;
  int local_e0;
  float local_dc;
  undefined4 local_d8;
  float local_d4;
  int *local_d0;
  float local_cc;
  undefined *local_c8;
  undefined4 local_c4;
  undefined4 local_c0;
  undefined4 local_bc;
  undefined4 local_b8;
  int local_b4;
  float local_b0 [2];
  float local_a8;
  float local_a4;
  int local_a0;
  undefined *local_9c;
  undefined1 local_98 [12];
  undefined1 local_8c [4];
  float local_88;
  undefined4 local_80;
  float local_7c;
  float local_78;
  undefined4 local_74;
  float local_70;
  undefined2 local_6c [4];
  int local_64;
  int local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  float local_30;
  int local_2c;
  uint local_28;
  int local_24;
  uint local_20;
  uint local_1c;
  float local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  pvVar8 = ExceptionList;
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a210e;
  local_10 = ExceptionList;
  if (param_1[0x85] != 0) {
    return param_1;
  }
  if (param_1[0x18] == 0) {
    return param_1;
  }
  piVar5 = param_1;
  ExceptionList = &local_10;
  if (param_1[0x14c] != 0) {
    ExceptionList = &local_10;
    if (*(char *)((int)param_1 + 0x795) == '\x01') {
      piVar5 = (int *)param_1[0x14c];
      ExceptionList = &local_10;
      piVar5[9] = 0;
    }
    if (param_1[0x93] == 0x2d) {
      piVar5 = (int *)param_1[0x14c];
      piVar5[9] = 0;
    }
  }
  if ((param_1[0x93] == 0x2d) || ((param_1[0x1d0] != 2 && (piVar5 = param_1, param_1[0x1d0] != 3))))
  {
    if ((3 < param_1[0x1d0]) && (piVar5 = param_1, param_1[0x1d0] < 10)) {
      if ((5 < *(uint *)(param_1[0x18] + 0x2e0)) && (*(uint *)(param_1[0x18] + 0x2e0) < 8)) {
        if ((param_1[0x17] == 4) || (param_1[0x17] == 2)) {
          FUN_0051d270(param_1[0x1d0],0);
        }
        if ((param_1[0x17] == 0) || (param_1[0x17] == 1)) {
          if (param_1[0x93] == 0x3c) {
            local_5c = 0x16b;
            if ((DAT_005ccf98 != 0) && (iVar4 = FUN_00429a6d(0x16b), iVar4 != 0)) {
              FUN_00429a6d(local_5c);
              iVar4 = FUN_0042afd0();
              if (iVar4 == 0) {
                uVar18 = 0;
                uVar17 = 0;
                FUN_00429a6d(local_5c);
                FUN_0042ad2b(uVar17,uVar18);
              }
            }
          }
          else {
            FUN_0051d270(param_1[0x1d0],0);
          }
        }
        else {
          local_60 = *(int *)(&DAT_00672570 + param_1[0x1d0] * 4 + param_1[0x17] * 0x2a0);
          if ((param_1[0x17] == 2) && (param_1[0x93] == 0x19)) {
            local_60 = local_60 + 10;
          }
          else if ((param_1[0x17] == 2) && (*(short *)((int)param_1 + 0x1d2) == 4)) {
            local_60 = 0xe2;
          }
          else if ((param_1[0x17] == 2) &&
                  ((*(short *)((int)param_1 + 0x1d2) == 5 || (*(short *)((int)param_1 + 0x1d2) == 6)
                   ))) {
            local_60 = 0xde;
          }
          else if ((param_1[0x17] == 4) && (*(short *)((int)param_1 + 0x1d2) == 3)) {
            local_60 = local_60 + 4;
          }
          else if ((param_1[0x17] == 0x15) && (1 < *(ushort *)((int)param_1 + 0x1d2))) {
            local_60 = local_60 + 0xab;
          }
          else if ((param_1[0x17] == 3) && (*(short *)((int)param_1 + 0x1d2) == 1)) {
            local_60 = local_60 + 4;
          }
          else if ((param_1[0x17] != 0x14) ||
                  (((*(short *)((int)param_1 + 0x1d2) != 4 &&
                    (*(short *)((int)param_1 + 0x1d2) != 7)) ||
                   (_DAT_005a414c <= (float)param_1[0xe])))) {
            if ((param_1[0x17] == 2) &&
               ((*(short *)((int)param_1 + 0x1d2) == 7 || (*(short *)((int)param_1 + 0x1d2) == 9))))
            {
              local_60 = 0xc9;
            }
            else if ((param_1[0x17] == 2) && (*(short *)((int)param_1 + 0x1d2) == 8)) {
              local_60 = 0xc9;
            }
            else if (param_1[0x93] == 0x23) {
              local_60 = local_60 + -0x59;
            }
            else if (((param_1[0x93] == 0x1e) && (*(short *)((int)param_1 + 0x1d2) == 4)) ||
                    ((param_1[0x93] == 0x21 && (*(short *)((int)param_1 + 0x1d2) == 1)))) {
              local_60 = 0x109;
            }
          }
          else {
            local_60 = 0xd0;
          }
          if ((DAT_005ccf98 != 0) && (iVar4 = FUN_00429a6d(local_60), iVar4 != 0)) {
            FUN_00429a6d(local_60);
            iVar4 = FUN_0042afd0();
            if (iVar4 == 0) {
              uVar18 = 0;
              uVar17 = 0;
              FUN_00429a6d(local_60);
              FUN_0042ad2b(uVar17,uVar18);
            }
          }
        }
      }
      if (((0xc < *(uint *)(param_1[0x18] + 0x2e0)) && (*(uint *)(param_1[0x18] + 0x2e0) < 0xf)) &&
         ((param_1[0x17] == 0 ||
          ((((param_1[0x17] == 1 || (param_1[0x17] == 3)) || (param_1[0x17] == 4)) ||
           (param_1[0x17] == 2)))))) {
        FUN_0051d270(param_1[0x1d0],1);
      }
      if (((7 < *(uint *)(param_1[0x18] + 0x2e0)) && (*(uint *)(param_1[0x18] + 0x2e0) < 0xb)) &&
         ((param_1[0x93] == 2 || (param_1[0x1d0] < 7)))) {
        _memset(local_6c,0,8);
        local_6c[0] = (undefined2)param_1[0x19b];
        local_64 = param_1[0x58];
        if ((local_64 == 1) && (0x5a < *(short *)(&DAT_00d44996 + (short)param_1[0x19b] * 0x8c))) {
          local_64 = 2;
        }
        FUN_0051d8bf(local_64,0);
        if (((local_64 != 0x65) && (local_64 != 0x66)) && (local_64 != 0x67)) {
          local_80 = 0x3f800000;
          local_7c = (float)(int)(short)param_1[0x111] / _DAT_005a43c4;
          if (_DAT_005a3660 < local_7c) {
            local_7c = 1.0;
          }
          FUN_00430f50(param_1[0x127],_DAT_005a40ec * local_7c + (float)param_1[0xc] + _DAT_005a40f4
                       ,param_1[0x128]);
          if ((param_1[0x1f1] != 0) &&
             (uVar6 = (**(code **)(*DAT_0092e654 + 8))(), param_1[0x1f1] + 100U < uVar6)) {
            FUN_00430f50(0,0,0xbf800000);
            FUN_00430f50((float)param_1[0x127] - (float)param_1[10],0,
                         (float)param_1[0x128] - (float)param_1[0xb]);
            thunk_FUN_005611dd(local_b0,local_b0);
            FUN_00430f10();
            FUN_0052fb40(local_8c,local_98,local_b0);
            fVar9 = (float10)FUN_004d3cf0(local_98,local_b0);
            local_a4 = (float)-fVar9;
            local_cc = (float)((-fVar9 * (float10)_DAT_005a34a0 + (float10)_DAT_005a34a0) *
                              (float10)_DAT_005a4148);
            if (local_88 < _DAT_005a3534) {
              local_cc = -local_cc;
            }
            local_b8 = 0x80ffffff;
            local_c8 = (undefined *)0x52a9e5;
            local_9c = (undefined *)0x334388;
            if (*(char *)((int)param_1 + 0x201) == '\b') {
              local_b8 = 0x80ffcccc;
              local_c8 = &DAT_00e57777;
              local_9c = &DAT_00883333;
            }
            else if (*(char *)((int)param_1 + 0x201) == '\a') {
              local_b8 = 0x80ffccff;
              local_c8 = &DAT_00cc88cc;
              local_9c = &DAT_00884388;
            }
            else if (*(char *)((int)param_1 + 0x201) == '\x06') {
              local_b8 = 0x80ccffcc;
              local_c8 = &DAT_0088e588;
              local_9c = (undefined *)0x338843;
            }
            else if (*(char *)((int)param_1 + 0x201) == '\x05') {
              local_b8 = 0x80ccccff;
              local_c8 = (undefined *)0x5253e5;
              local_9c = (undefined *)0x222288;
            }
            local_a0 = 0;
            local_b4 = 0;
            if (*(int *)(DAT_0067cf38 + 0x24) == 30000) {
              local_b4 = DAT_0067cf38;
            }
            local_19c = operator_new(0xcc);
            local_8 = 1;
            if (local_19c == (void *)0x0) {
              local_2dc = 0;
            }
            else {
              local_2dc = FUN_004d2230(0x213,local_b8,param_1[0xd],4);
            }
            local_198 = local_2dc;
            local_8 = 0xffffffff;
            local_a0 = local_2dc;
            if (local_2dc != 0) {
              if ((((float)param_1[0x1f2] <= _DAT_005a430c) || (local_b4 == 0)) ||
                 (param_1 == *(int **)(local_b4 + 0x4c))) {
                *(float *)(local_2dc + 0x98) = _DAT_005a3f2c * (float)param_1[0x1f2];
                *(float *)(local_2dc + 0x9c) = _DAT_005a3f2c * (float)param_1[0x1f2];
                *(undefined4 *)(local_2dc + 0xb0) = 500;
              }
              else {
                *(float *)(local_2dc + 0x98) = _DAT_005a4154 * (float)param_1[0x1f2];
                *(float *)(local_2dc + 0x9c) = _DAT_005a4154 * (float)param_1[0x1f2];
                *(undefined4 *)(local_2dc + 0xb0) = 500;
              }
              *(undefined4 *)(local_2dc + 0xa4) = 0xe5;
              *(undefined4 *)(local_2dc + 0xb8) = 500;
              FUN_004310a0(&local_78);
              *(undefined4 *)(local_a0 + 0x6c) = 1;
              *(undefined1 *)(local_a0 + 0xac) = 0;
              FUN_0054ac09(local_a0);
            }
            if (((float)param_1[0x1f2] <= _DAT_005a430c) || (param_1[0x93] != 0x38)) {
              local_d0 = (int *)0x0;
              local_1a4 = operator_new(0xc0);
              local_8 = 2;
              if (local_1a4 == (void *)0x0) {
                local_2e0 = (int *)0x0;
              }
              else {
                local_2e0 = (int *)FUN_004df250(7,0x76,0x3f800000);
              }
              local_1a0 = local_2e0;
              local_8 = 0xffffffff;
              local_d0 = local_2e0;
              if (local_2e0 != (int *)0x0) {
                (**(code **)(*local_2e0 + 0x44))(local_9c);
                local_d0[0x1b] = 1;
                puVar7 = (undefined4 *)
                         FUN_00430f20(local_b0[0] * _DAT_005a34a0 + local_78,
                                      local_a8 * _DAT_005a34a0 + local_70);
                (**(code **)(*local_d0 + 0x3c))(*puVar7,puVar7[1]);
                local_d0[0x28] = 300;
                FUN_0054ac09(local_d0);
              }
            }
            CSimpleArray<>();
            FUN_0058f220(&local_c4,local_b0,0xc);
            if (DAT_0092e660 == 0) {
              local_e4 = (**(code **)(*DAT_013b71e8 + 0x34))(param_1[0x1f5]);
              if (local_e4 != 0) {
                *(undefined1 *)(local_e4 + 0x7cc) = 1;
                uVar17 = (**(code **)(*DAT_0092e654 + 8))();
                *(undefined4 *)(local_e4 + 2000) = uVar17;
              }
              local_e0 = 0;
              if (((float)param_1[0x1f2] <= _DAT_005a430c) || (param_1[0x93] != 0x38)) {
                if (*(char *)((int)param_1 + 0x79a) == '\x01') {
                  local_1b4 = operator_new(0x90);
                  local_8 = 3;
                  if (local_1b4 == (void *)0x0) {
                    local_2e4 = 0;
                  }
                  else {
                    uVar19 = 800;
                    uVar16 = 1;
                    uVar15 = 0x3f800000;
                    uVar14 = 0xe7;
                    uVar13 = 0;
                    puVar12 = &DAT_00ff3366;
                    uVar11 = 0x3e99999a;
                    uVar10 = 0xf;
                    uVar18 = 5;
                    uVar17 = FUN_004c8c60(local_1c0,0x3e99999a);
                    puVar7 = (undefined4 *)FUN_00493d50(local_1cc,uVar17);
                    local_2e4 = FUN_004d4de0(*puVar7,puVar7[1],puVar7[2],uVar18,uVar10,uVar11,
                                             puVar12,uVar13,uVar14,uVar15,uVar16,local_c4,local_c0,
                                             local_bc,uVar19);
                  }
                  local_1b0 = local_2e4;
                  local_e0 = local_2e4;
                }
                else {
                  local_1d4 = operator_new(0x90);
                  local_8 = 4;
                  if (local_1d4 == (void *)0x0) {
                    local_2e8 = 0;
                  }
                  else {
                    uVar19 = 800;
                    uVar16 = 1;
                    uVar15 = 0x3f800000;
                    uVar14 = 0xe7;
                    uVar13 = 0;
                    puVar12 = &DAT_00ffeeaa;
                    uVar11 = 0x3dcccccd;
                    uVar10 = 10;
                    uVar18 = 5;
                    uVar17 = FUN_004c8c60(local_1e0,0x3e99999a);
                    puVar7 = (undefined4 *)FUN_00493d50(local_1ec,uVar17);
                    local_2e8 = FUN_004d4de0(*puVar7,puVar7[1],puVar7[2],uVar18,uVar10,uVar11,
                                             puVar12,uVar13,uVar14,uVar15,uVar16,local_c4,local_c0,
                                             local_bc,uVar19);
                  }
                  local_1d0 = local_2e8;
                  local_e0 = local_2e8;
                }
                local_8 = 0xffffffff;
                if (local_e0 != 0) {
                  FUN_0054ac09(local_e0);
                }
              }
              local_ec = (int *)0x0;
              local_dc = local_78;
              local_d8 = local_74;
              local_d4 = local_70;
              if ((float)param_1[0x1f2] <= _DAT_005a430c) {
                local_1fc = operator_new(0x180);
                local_8 = 6;
                if (local_1fc == (void *)0x0) {
                  local_2f0 = (int *)0x0;
                }
                else {
                  local_2f0 = (int *)FUN_004c8f70(0xe6,600,_DAT_005a3834 * (float)param_1[0x1f2],
                                                  _DAT_005a3834 * (float)param_1[0x1f2],
                                                  _DAT_005a3834 * (float)param_1[0x1f2],0x3a03126f,1
                                                  ,0x50);
                }
                local_1f8 = local_2f0;
                local_ec = local_2f0;
              }
              else {
                local_1f4 = operator_new(0x180);
                local_8 = 5;
                if (local_1f4 == (void *)0x0) {
                  local_2ec = (int *)0x0;
                }
                else {
                  local_2ec = (int *)FUN_004c8f70(0xe6,600,_DAT_005a3f2c * (float)param_1[0x1f2],
                                                  _DAT_005a3f2c * (float)param_1[0x1f2],
                                                  _DAT_005a3f2c * (float)param_1[0x1f2],0x3a03126f,1
                                                  ,0x50);
                }
                local_1f0 = local_2ec;
                local_ec = local_2ec;
              }
              local_8 = 0xffffffff;
              if (local_ec != (int *)0x0) {
                local_ec[0x42] = 0;
                uVar17 = FUN_004c8c60(local_208,0x3f000000);
                uVar17 = FUN_00493d50(local_214,uVar17);
                FUN_004310a0(uVar17);
                FUN_004310a0(&local_c4);
                local_ec[0x45] = (int)((float)param_1[0xd] + _DAT_005a4380 + _DAT_005a439c);
                local_ec[0x1b] = 1;
                local_ec[0x52] = 0xe;
                local_ec[0x3b] = 0x3a83126f;
                local_ec[0x3c] = 0x3a83126f;
                local_ec[0x3d] = 0x3a83126f;
                (**(code **)(*local_ec + 0x3c))(local_c8);
                FUN_0054ac09(local_ec);
              }
              local_e8 = (int *)0x0;
              if ((float)param_1[0x1f2] <= _DAT_005a430c) {
                local_224 = operator_new(0x180);
                local_8 = 8;
                if (local_224 == (void *)0x0) {
                  local_2f8 = (int *)0x0;
                }
                else {
                  local_2f8 = (int *)FUN_004c8f70(0xe6,600,_DAT_005a3834 * (float)param_1[0x1f2],
                                                  _DAT_005a3834 * (float)param_1[0x1f2],
                                                  _DAT_005a3834 * (float)param_1[0x1f2],0x3a03126f,1
                                                  ,0x50);
                }
                local_220 = local_2f8;
                local_e8 = local_2f8;
              }
              else {
                local_21c = operator_new(0x180);
                local_8 = 7;
                if (local_21c == (void *)0x0) {
                  local_2f4 = (int *)0x0;
                }
                else {
                  local_2f4 = (int *)FUN_004c8f70(0xe6,600,_DAT_005a3f2c * (float)param_1[0x1f2],
                                                  _DAT_005a3f2c * (float)param_1[0x1f2],
                                                  _DAT_005a3f2c * (float)param_1[0x1f2],0x3a03126f,1
                                                  ,0x50);
                }
                local_218 = local_2f4;
                local_e8 = local_2f4;
              }
              local_8 = 0xffffffff;
              if (local_e8 != (int *)0x0) {
                local_e8[0x42] = 0;
                uVar17 = FUN_004c8c60(local_230,0x3f000000);
                uVar17 = FUN_00493d50(local_23c,uVar17);
                FUN_004310a0(uVar17);
                FUN_004310a0(&local_c4);
                local_e8[0x45] = (int)(((float)param_1[0xd] + _DAT_005a4380) - _DAT_005a439c);
                local_e8[0x1b] = 1;
                local_e8[0x52] = 0xf;
                local_e8[0x3b] = 0x3a83126f;
                local_e8[0x3c] = 0x3a83126f;
                local_e8[0x3d] = 0x3a83126f;
                (**(code **)(*local_e8 + 0x3c))(local_c8);
                FUN_0054ac09(local_e8);
              }
            }
          }
          iVar4 = (**(code **)(*DAT_0092e654 + 8))();
          param_1[0x1f1] = iVar4;
        }
      }
      piVar5 = (int *)param_1[0x18];
      if (((0xe < (uint)piVar5[0xb8]) && (*(uint *)(param_1[0x18] + 0x2e0) < 0x11)) &&
         ((param_1[0x93] == 2 || (piVar5 = param_1, param_1[0x1d0] < 7)))) {
        _memset(local_108,0,8);
        _memset(local_f4,0,8);
        local_108[0] = (undefined2)param_1[0x19b];
        local_f4[0] = *(undefined2 *)((int)param_1 + 0x66e);
        local_100 = param_1[0x59];
        local_f8 = FUN_0054cd07(local_108,0x11);
        piVar5 = (int *)FUN_0054cd07(local_f4,0x11);
        local_fc = piVar5;
        if (((piVar5 != (int *)0x0) && (piVar5 != (int *)0x80)) ||
           ((piVar5 == (int *)0x0 && (local_f8 == 0)))) {
          if ((local_100 == 1) &&
             (0x5a < *(short *)(&DAT_00d44996 + *(short *)((int)param_1 + 0x66e) * 0x8c))) {
            local_100 = 2;
          }
          FUN_0051d8bf(local_100,1);
          local_128 = 0x3f800000;
          local_11c = (float)(int)(short)param_1[0x111] / _DAT_005a43c4;
          if (_DAT_005a3660 < local_11c) {
            local_11c = 1.0;
          }
          local_120 = 0x80ffffff;
          local_124 = (undefined *)0x52a9e5;
          local_10c = (undefined *)0x334388;
          if (*(char *)((int)param_1 + 0x201) == '\b') {
            local_120 = 0x80ffcccc;
            local_124 = &DAT_00e57777;
            local_10c = &DAT_00883333;
          }
          else if (*(char *)((int)param_1 + 0x201) == '\a') {
            local_120 = 0x80ffccff;
            local_124 = &DAT_00cc88cc;
            local_10c = &DAT_00884388;
          }
          else if (*(char *)((int)param_1 + 0x201) == '\x06') {
            local_120 = 0x80ccffcc;
            local_124 = &DAT_0088e588;
            local_10c = (undefined *)0x338843;
          }
          else if (*(char *)((int)param_1 + 0x201) == '\x05') {
            local_120 = 0x80ccccff;
            local_124 = (undefined *)0x5253e5;
            local_10c = (undefined *)0x222288;
          }
          FUN_00430f50(param_1[0x127],
                       _DAT_005a40ec * local_11c + (float)param_1[0xc] + _DAT_005a40f4,
                       param_1[0x128]);
          if ((param_1[0x1f1] != 0) &&
             (uVar6 = (**(code **)(*DAT_0092e654 + 8))(), param_1[0x1f1] + 100U < uVar6)) {
            FUN_00430f50(0,0,0xbf800000);
            FUN_00430f50((float)param_1[0x127] - (float)param_1[10],0,
                         (float)param_1[0x128] - (float)param_1[0xb]);
            thunk_FUN_005611dd(local_154,local_154);
            FUN_00430f10();
            FUN_0052fb40(local_134,local_140,local_154);
            fVar9 = (float10)FUN_004d3cf0(local_140,local_154);
            local_148 = (float)-fVar9;
            local_158 = (float)((-fVar9 * (float10)_DAT_005a34a0 + (float10)_DAT_005a34a0) *
                               (float10)_DAT_005a4148);
            if (local_130 < _DAT_005a3534) {
              local_158 = -local_158;
            }
            local_144 = 0;
            local_244 = operator_new(0xcc);
            local_8 = 9;
            if (local_244 == (void *)0x0) {
              local_300 = 0;
            }
            else {
              local_300 = FUN_004d2230(0x213,local_120,param_1[0xd],4);
            }
            local_240 = local_300;
            local_8 = 0xffffffff;
            local_144 = local_300;
            if (local_300 != 0) {
              *(undefined4 *)(local_300 + 0xa4) = 0xe5;
              *(undefined4 *)(local_300 + 0xb0) = 500;
              *(undefined4 *)(local_300 + 0xb8) = 500;
              FUN_004310a0(&local_118);
              *(float *)(local_144 + 0x98) = _DAT_005a3f2c * (float)param_1[0x1f2];
              *(float *)(local_144 + 0x9c) = _DAT_005a3f2c * (float)param_1[0x1f2];
              *(undefined4 *)(local_144 + 0x6c) = 1;
              *(undefined1 *)(local_144 + 0xac) = 0;
              FUN_0054ac09(local_144);
              local_15c = (int *)0x0;
              local_24c = operator_new(0xc0);
              local_8 = 10;
              if (local_24c == (void *)0x0) {
                local_304 = (int *)0x0;
              }
              else {
                local_304 = (int *)FUN_004df250(7,0x76,0x3f800000);
              }
              local_248 = local_304;
              local_8 = 0xffffffff;
              local_15c = local_304;
              if (local_304 != (int *)0x0) {
                (**(code **)(*local_304 + 0x44))(local_10c);
                local_15c[0x1b] = 1;
                puVar7 = (undefined4 *)
                         FUN_00430f20(local_154[0] * _DAT_005a34a0 + local_118,
                                      local_14c * _DAT_005a34a0 + local_110);
                (**(code **)(*local_15c + 0x3c))(*puVar7,puVar7[1]);
                local_15c[0x28] = 300;
                FUN_0054ac09(local_15c);
              }
            }
            if (DAT_0092e660 == 0) {
              local_16c = 0;
              CSimpleArray<>();
              FUN_0058f220(&local_17c,local_154,0xc);
              local_25c = operator_new(0x90);
              local_8 = 0xb;
              if (local_25c == (void *)0x0) {
                local_308 = 0;
              }
              else {
                uVar19 = 800;
                uVar16 = 1;
                uVar15 = 0x3f800000;
                uVar14 = 0xe7;
                uVar13 = 0;
                puVar12 = &DAT_00ffeeaa;
                uVar11 = 0x3dcccccd;
                uVar10 = 10;
                uVar18 = 5;
                uVar17 = FUN_004c8c60(local_268,0x3e99999a);
                puVar7 = (undefined4 *)FUN_00493d50(local_274,uVar17);
                local_308 = FUN_004d4de0(*puVar7,puVar7[1],puVar7[2],uVar18,uVar10,uVar11,puVar12,
                                         uVar13,uVar14,uVar15,uVar16,local_17c,local_178,local_174,
                                         uVar19);
              }
              local_258 = local_308;
              local_8 = 0xffffffff;
              local_16c = local_308;
              if (local_308 != 0) {
                FUN_0054ac09(local_308);
              }
              local_180 = (int *)0x0;
              local_168 = local_118;
              local_164 = local_114;
              local_160 = local_110;
              local_27c = operator_new(0x180);
              local_8 = 0xc;
              if (local_27c == (void *)0x0) {
                local_30c = (int *)0x0;
              }
              else {
                local_30c = (int *)FUN_004c8f70(0xe6,600,_DAT_005a3834 * (float)param_1[0x1f2],
                                                _DAT_005a3834 * (float)param_1[0x1f2],
                                                _DAT_005a3834 * (float)param_1[0x1f2],0x3a03126f,1,
                                                0x50);
              }
              local_278 = local_30c;
              local_8 = 0xffffffff;
              local_180 = local_30c;
              if (local_30c != (int *)0x0) {
                local_30c[0x42] = 0;
                uVar17 = FUN_004c8c60(local_288,0x3f000000);
                uVar17 = FUN_00493d50(local_294,uVar17);
                FUN_004310a0(uVar17);
                FUN_004310a0(&local_17c);
                local_180[0x45] = (int)((float)param_1[0xd] + _DAT_005a4380 + _DAT_005a439c);
                local_180[0x1b] = 1;
                local_180[0x52] = 0xe;
                local_180[0x3b] = 0x3a83126f;
                local_180[0x3c] = 0x3a83126f;
                local_180[0x3d] = 0x3a83126f;
                (**(code **)(*local_180 + 0x3c))(local_124);
                FUN_0054ac09(local_180);
              }
              local_170 = (int *)0x0;
              local_29c = operator_new(0x180);
              local_8 = 0xd;
              if (local_29c == (void *)0x0) {
                local_310 = (int *)0x0;
              }
              else {
                local_310 = (int *)FUN_004c8f70(0xe6,600,_DAT_005a3834 * (float)param_1[0x1f2],
                                                _DAT_005a3834 * (float)param_1[0x1f2],
                                                _DAT_005a3834 * (float)param_1[0x1f2],0x3a03126f,1,
                                                0x50);
              }
              local_298 = local_310;
              local_8 = 0xffffffff;
              local_170 = local_310;
              if (local_310 != (int *)0x0) {
                local_310[0x42] = 0;
                uVar17 = FUN_004c8c60(local_2a8,0x3f000000);
                uVar17 = FUN_00493d50(local_2b4,uVar17);
                FUN_004310a0(uVar17);
                FUN_004310a0(&local_17c);
                local_170[0x45] = (int)(((float)param_1[0xd] + _DAT_005a4380) - _DAT_005a439c);
                local_170[0x1b] = 1;
                local_170[0x52] = 0xf;
                local_170[0x3b] = 0x3a83126f;
                local_170[0x3c] = 0x3a83126f;
                local_170[0x3d] = 0x3a83126f;
                (**(code **)(*local_170 + 0x3c))(local_124);
                FUN_0054ac09(local_170);
              }
            }
          }
          piVar5 = (int *)(**(code **)(*DAT_0092e654 + 8))();
          param_1[0x1f1] = (int)piVar5;
        }
      }
      if (param_1[0x17] != 0x14) {
        ExceptionList = local_10;
        return piVar5;
      }
      if ((*(short *)((int)param_1 + 0x1d6) != 0) &&
         ((int *)(uint)*(ushort *)((int)param_1 + 0x1d6) != (int *)0x2)) {
        ExceptionList = local_10;
        return (int *)(uint)*(ushort *)((int)param_1 + 0x1d6);
      }
      piVar5 = (int *)(*(uint *)(param_1[0x18] + 0x2e0) / 5);
      if (*(uint *)(param_1[0x18] + 0x2e0) % 5 != 0) {
        ExceptionList = local_10;
        return piVar5;
      }
      if (param_1[0x1d0] != 5) {
        ExceptionList = local_10;
        return piVar5;
      }
      if (((float)param_1[0x118] == (float)param_1[0x19]) &&
         ((float)param_1[0x119] == (float)param_1[0x1a])) {
        fVar1 = (float)param_1[0x11a];
        fVar2 = (float)param_1[0x1b];
        if ((fVar1 == fVar2) != 0) {
          ExceptionList = local_10;
          return (int *)(uint)(ushort)((ushort)(fVar1 < fVar2) << 8 |
                                       (ushort)(NAN(fVar1) || NAN(fVar2)) << 10 |
                                      (ushort)(fVar1 == fVar2) << 0xe);
        }
      }
      pvVar8 = operator_new(0x180);
      local_8 = 0xe;
      if (pvVar8 == (void *)0x0) {
        local_314 = 0;
      }
      else {
        local_314 = FUN_004c8f70(0x2c,2000,0x3f000000,0x3f000000,0x3f000000,0x3a03126f,1,0x50);
      }
      local_8 = 0xffffffff;
      local_184 = local_314;
      if (local_314 != 0) {
        *(undefined4 *)(local_314 + 0x6c) = 1;
        FUN_004310a0(param_1 + 0x19);
        FUN_0054ac09(local_184);
      }
      piVar5 = (int *)FUN_004310a0(param_1 + 0x19);
      ExceptionList = local_10;
      return piVar5;
    }
    if (param_1[0x1d0] != 10) {
      if (param_1[0x1d0] != 0xb) {
        ExceptionList = pvVar8;
        return piVar5;
      }
      if (*(uint *)(param_1[0x18] + 0x2e0) < 5) {
        ExceptionList = pvVar8;
        return param_1;
      }
      if (6 < (uint)((int *)param_1[0x18])[0xb8]) {
        ExceptionList = pvVar8;
        return (int *)param_1[0x18];
      }
      local_18c = *(int *)(&DAT_0067259c + param_1[0x17] * 0x2a0);
      if ((param_1[0x17] == 2) && (param_1[0x93] == 0x19)) {
        local_18c = local_18c + 10;
      }
      else if ((param_1[0x17] == 2) &&
              ((*(short *)((int)param_1 + 0x1d2) == 5 || (*(short *)((int)param_1 + 0x1d2) == 6))))
      {
        local_18c = 0xe0;
      }
      else if ((param_1[0x17] == 4) && (*(short *)((int)param_1 + 0x1d2) == 3)) {
        local_18c = local_18c + 4;
      }
      else if ((param_1[0x17] == 0x15) && (1 < *(ushort *)((int)param_1 + 0x1d2))) {
        local_18c = local_18c + 0xab;
      }
      else if ((param_1[0x17] == 3) && (*(short *)((int)param_1 + 0x1d2) == 1)) {
        local_18c = local_18c + 4;
      }
      else if ((param_1[0x17] != 0x14) ||
              (((*(short *)((int)param_1 + 0x1d2) != 4 && (*(short *)((int)param_1 + 0x1d2) != 7))
               || (_DAT_005a414c <= (float)param_1[0xe])))) {
        if ((param_1[0x17] == 2) &&
           ((*(short *)((int)param_1 + 0x1d2) == 7 || (*(short *)((int)param_1 + 0x1d2) == 9)))) {
          local_18c = 0x106;
        }
        else if ((param_1[0x17] == 2) && (*(short *)((int)param_1 + 0x1d2) == 8)) {
          local_18c = 0x107;
        }
        else if (param_1[0x93] == 0x22) {
          local_18c = 0x103;
        }
        else if (param_1[0x93] == 0x23) {
          local_18c = local_18c + -0x59;
        }
        else if ((param_1[0x93] == 0x24) || (param_1[0x93] == 0x25)) {
          local_18c = 0x108;
        }
        else if (((param_1[0x93] == 0x27) ||
                 ((param_1[0x93] == 0x1e && (*(short *)((int)param_1 + 0x1d2) == 4)))) ||
                ((param_1[0x93] == 0x21 && (*(short *)((int)param_1 + 0x1d2) == 1)))) {
          local_18c = 0x10c;
        }
        else if (param_1[0x93] == 0x3c) {
          local_18c = 0x16d;
        }
      }
      else {
        local_18c = 0xd1;
      }
      if ((param_1[0x93] == 4) && (*(short *)((int)param_1 + 0x1d2) == 0xf)) {
        local_18c = 299;
      }
      if (DAT_005ccf98 == 0) {
        ExceptionList = pvVar8;
        return param_1;
      }
      iVar4 = FUN_00429a6d(local_18c);
      if (iVar4 == 0) {
        ExceptionList = local_10;
        return (int *)0x0;
      }
      FUN_00429a6d(local_18c);
      piVar5 = (int *)FUN_0042afd0();
      if (piVar5 != (int *)0x0) {
        ExceptionList = local_10;
        return piVar5;
      }
      uVar18 = 0;
      uVar17 = 0;
      FUN_00429a6d(local_18c);
      piVar5 = (int *)FUN_0042ad2b(uVar17,uVar18);
      ExceptionList = local_10;
      return piVar5;
    }
    if (1 < *(uint *)(param_1[0x18] + 0x2e0)) {
      ExceptionList = pvVar8;
      return (int *)param_1[0x18];
    }
    local_188 = *(int **)(&DAT_00672598 + param_1[0x17] * 0x2a0);
    piVar5 = param_1;
    if ((param_1[0x17] == 2) && (param_1[0x93] == 0x19)) {
      local_188 = (int *)((int)local_188 + 10);
    }
    else if ((param_1[0x17] == 2) &&
            ((*(short *)((int)param_1 + 0x1d2) == 5 || (*(short *)((int)param_1 + 0x1d2) == 6)))) {
      local_188 = (int *)0xdf;
    }
    else if ((param_1[0x17] == 4) && (*(short *)((int)param_1 + 0x1d2) == 3)) {
      local_188 = local_188 + 1;
    }
    else if ((param_1[0x17] == 0x15) && (1 < *(ushort *)((int)param_1 + 0x1d2))) {
      local_188 = (int *)((int)local_188 + 0xab);
      piVar5 = local_188;
    }
    else if ((param_1[0x17] == 3) &&
            (piVar5 = (int *)(uint)*(ushort *)((int)param_1 + 0x1d2), piVar5 == (int *)0x1)) {
      local_188 = local_188 + 1;
    }
    else {
      if ((param_1[0x17] == 0x14) &&
         ((*(short *)((int)param_1 + 0x1d2) == 4 ||
          (piVar5 = (int *)(uint)*(ushort *)((int)param_1 + 0x1d2), piVar5 == (int *)0x7)))) {
        fVar1 = (float)param_1[0xe];
        piVar5 = (int *)(uint)(ushort)((ushort)(fVar1 < _DAT_005a414c) << 8 |
                                       (ushort)(NAN(fVar1) || NAN(_DAT_005a414c)) << 10 |
                                      (ushort)(fVar1 == _DAT_005a414c) << 0xe);
        if (fVar1 < _DAT_005a414c) {
          local_188 = (int *)0xd0;
          goto LAB_0051a130;
        }
      }
      if ((param_1[0x17] == 2) &&
         ((piVar5 = param_1, *(short *)((int)param_1 + 0x1d2) == 7 ||
          (piVar5 = (int *)(uint)*(ushort *)((int)param_1 + 0x1d2), piVar5 == (int *)0x9)))) {
        local_188 = (int *)0x105;
      }
      else if ((param_1[0x17] == 2) &&
              (piVar5 = (int *)(uint)*(ushort *)((int)param_1 + 0x1d2), piVar5 == (int *)0x8)) {
        local_188 = (int *)0x105;
      }
      else if (param_1[0x93] == 0x23) {
        local_188 = (int *)((int)local_188 + -0x59);
      }
      else if (param_1[0x93] == 0x22) {
        local_188 = (int *)0x102;
        piVar5 = param_1;
      }
      else {
        piVar5 = param_1;
        if (((param_1[0x93] == 0x1e) &&
            (piVar5 = (int *)(uint)*(ushort *)((int)param_1 + 0x1d2), piVar5 == (int *)0x4)) ||
           ((param_1[0x93] == 0x21 &&
            (piVar5 = (int *)(uint)*(ushort *)((int)param_1 + 0x1d2), piVar5 == (int *)0x1)))) {
          local_188 = (int *)0x109;
        }
        else if (param_1[0x93] == 0x3c) {
          local_188 = (int *)0x16c;
        }
      }
    }
LAB_0051a130:
    if ((param_1[0x93] == 4) && (piVar5 = param_1, *(short *)((int)param_1 + 0x1d2) == 0xf)) {
      local_188 = (int *)0x138;
    }
    if (DAT_005ccf98 == 0) {
      ExceptionList = pvVar8;
      return piVar5;
    }
    iVar4 = FUN_00429a6d(local_188);
    if (iVar4 == 0) {
      ExceptionList = local_10;
      return (int *)0x0;
    }
    FUN_00429a6d(local_188);
    piVar5 = (int *)FUN_0042afd0();
    if (piVar5 != (int *)0x0) {
      ExceptionList = local_10;
      return piVar5;
    }
    uVar18 = 0;
    uVar17 = 0;
    FUN_00429a6d(local_188);
    piVar5 = (int *)FUN_0042ad2b(uVar17,uVar18);
    ExceptionList = local_10;
    return piVar5;
  }
  local_18 = 0.0;
  iVar4 = FUN_0049cf38(param_1[10],param_1[0xb],param_1[0xc],&local_18);
  if (iVar4 == 1) {
    param_2 = 4;
    local_24 = (**(code **)(*DAT_0092e654 + 8))();
    local_28 = 0x50;
    if (param_1[0x1d0] == 2) {
      local_28 = 0x78;
    }
    if (local_28 < (uint)(local_24 - param_1[0x181])) {
      local_30 = (float)param_1[0xe] * *(float *)(&DAT_0092e7d0 + param_1[0x17] * 8) * 0.002;
      local_194 = operator_new(0x124);
      local_8 = 0;
      if (local_194 == (void *)0x0) {
        local_2c4 = 0;
      }
      else {
        local_2c4 = FUN_004cb900(10,700,0x3f000000,0x3f000000,0x3f000000,local_30,0);
      }
      local_190 = local_2c4;
      local_8 = 0xffffffff;
      local_2c = local_2c4;
      if (local_2c4 != 0) {
        *(undefined4 *)(local_2c4 + 0x6c) = 1;
        *(int *)(local_2c4 + 0x74) = param_1[10];
        *(int *)(local_2c4 + 0x7c) = param_1[0xb];
        *(float *)(local_2c4 + 0x78) = local_18 + _DAT_005a4150;
        FUN_0054ac09(local_2c4);
      }
      param_1[0x181] = local_24;
    }
  }
  local_1c = *(uint *)(param_1[0x18] + 0x2e0);
  local_14 = param_1[0x17];
  local_20 = (uint)*(ushort *)((int)param_1 + 0x1d6);
  if (((char)param_1[0x1e7] == '\x01') && (param_1[0x67] != 0)) {
    local_1c = *(uint *)(param_1[0x67] + 0x2e0);
    local_14 = param_1[0x1e8];
    local_20 = (uint)*(ushort *)((int)param_1 + 0x1a2);
  }
  if (param_2 == 8) {
    iVar4 = __ftol();
    if ((((iVar4 >> 7 < 0x1b) || (iVar4 = __ftol(), 0x1e < iVar4 >> 7)) ||
        (iVar4 = __ftol(), iVar4 >> 7 < 0x15)) || (iVar4 = __ftol(), 0x18 < iVar4 >> 7)) {
      bVar3 = false;
    }
    else {
      bVar3 = true;
    }
    if (!bVar3) goto LAB_0051759b;
    bVar3 = true;
  }
  else {
LAB_0051759b:
    bVar3 = false;
  }
  if (bVar3) {
    param_2 = 0x53;
  }
  if (param_2 == 9) {
    iVar4 = __ftol();
    if (((iVar4 >> 7 < 0x1b) || (iVar4 = __ftol(), 0x1e < iVar4 >> 7)) ||
       ((iVar4 = __ftol(), iVar4 >> 7 < 0x15 || (iVar4 = __ftol(), 0x18 < iVar4 >> 7)))) {
      bVar3 = false;
    }
    else {
      bVar3 = true;
    }
    if (bVar3) {
      bVar3 = true;
      goto LAB_0051764c;
    }
  }
  bVar3 = false;
LAB_0051764c:
  if (bVar3) {
    param_2 = 0x52;
  }
  if (((*(char *)((int)param_1 + 0x795) == '\0') && (param_2 != 4)) &&
     ((local_14 == 0x1f && (param_1 == *(int **)(DAT_013b71e8[0x6c21] + 0x54))))) {
    piVar5 = DAT_013b71e8;
    if (local_1c < 2) {
      local_34 = 0xc0;
      if (param_2 == 1) {
        local_34 = 0xb8;
      }
      else if (param_2 == 8) {
        local_34 = 0xc6;
      }
      else if (param_2 == 0x52) {
        local_34 = 0x162;
      }
      else if (param_2 == 0x53) {
        local_34 = 0x15e;
      }
      if (DAT_005ccf98 != 0) {
        iVar4 = FUN_00429a6d(local_34);
        piVar5 = (int *)0x0;
        if (iVar4 != 0) {
          FUN_00429a6d(local_34);
          piVar5 = (int *)FUN_0042afd0();
          if (piVar5 == (int *)0x0) {
            uVar18 = 0;
            uVar17 = 0;
            FUN_00429a6d(local_34);
            piVar5 = (int *)FUN_0042ad2b(uVar17,uVar18);
          }
        }
      }
    }
    if ((4 < local_1c) && (local_1c < 7)) {
      local_38 = 0xc1;
      if (param_2 == 1) {
        local_38 = 0xb9;
      }
      else if (param_2 == 8) {
        local_38 = 199;
      }
      else if (param_2 == 0x52) {
        local_38 = 0x163;
      }
      else if (param_2 == 0x53) {
        local_38 = 0x15f;
      }
      if (DAT_005ccf98 != 0) {
        iVar4 = FUN_00429a6d(local_38);
        piVar5 = (int *)0x0;
        if (iVar4 != 0) {
          FUN_00429a6d(local_38);
          piVar5 = (int *)FUN_0042afd0();
          if (piVar5 == (int *)0x0) {
            uVar18 = 0;
            uVar17 = 0;
            FUN_00429a6d(local_38);
            piVar5 = (int *)FUN_0042ad2b(uVar17,uVar18);
          }
        }
      }
    }
    if ((10 < local_1c) && (local_1c < 0xd)) {
      local_3c = 0xc2;
      if (param_2 == 1) {
        local_3c = 0xc4;
      }
      else if (param_2 == 8) {
        local_3c = 0xb4;
      }
      else if (param_2 == 0x52) {
        local_3c = 0x164;
      }
      else if (param_2 == 0x53) {
        local_3c = 0x160;
      }
      if (DAT_005ccf98 != 0) {
        iVar4 = FUN_00429a6d(local_3c);
        piVar5 = (int *)0x0;
        if (iVar4 != 0) {
          FUN_00429a6d(local_3c);
          piVar5 = (int *)FUN_0042afd0();
          if (piVar5 == (int *)0x0) {
            uVar18 = 0;
            uVar17 = 0;
            FUN_00429a6d(local_3c);
            piVar5 = (int *)FUN_0042ad2b(uVar17,uVar18);
          }
        }
      }
    }
    if ((0xc < local_1c) && (local_1c < 0xf)) {
      local_40 = 0xc3;
      if (param_2 == 1) {
        local_40 = 0xc5;
      }
      else if (param_2 == 8) {
        local_40 = 0xb5;
      }
      else if (param_2 == 0x52) {
        local_40 = 0x165;
      }
      else if (param_2 == 0x53) {
        local_40 = 0x161;
      }
      if (DAT_005ccf98 != 0) {
        iVar4 = FUN_00429a6d(local_40);
        piVar5 = (int *)0x0;
        if (iVar4 != 0) {
          FUN_00429a6d(local_40);
          piVar5 = (int *)FUN_0042afd0();
          if (piVar5 == (int *)0x0) {
            uVar18 = 0;
            uVar17 = 0;
            FUN_00429a6d(local_40);
            piVar5 = (int *)FUN_0042ad2b(uVar17,uVar18);
          }
        }
      }
    }
  }
  else {
    piVar5 = param_1;
    if (*(char *)((int)param_1 + 0x795) == '\0') {
      if (local_1c < 2) {
        if (DAT_005ccf98 != 0) {
          iVar4 = FUN_00429a6d(param_2 * 2 + 0xb6);
          piVar5 = (int *)0x0;
          if (iVar4 != 0) {
            FUN_00429a6d(param_2 * 2 + 0xb6);
            piVar5 = (int *)FUN_0042afd0();
            if (piVar5 == (int *)0x0) {
              if ((local_14 == 0x14) &&
                 (((local_20 == 0 || (local_20 == 2)) && (param_1[0x1d0] == 3)))) {
                local_44 = uRam006759fc;
                piVar5 = param_1;
                if (DAT_005ccf98 != 0) {
                  iVar4 = FUN_00429a6d(uRam006759fc);
                  piVar5 = (int *)0x0;
                  if (iVar4 != 0) {
                    FUN_00429a6d(local_44);
                    piVar5 = (int *)FUN_0042afd0();
                    if (piVar5 == (int *)0x0) {
                      uVar18 = 0;
                      uVar17 = 0;
                      FUN_00429a6d(local_44);
                      piVar5 = (int *)FUN_0042ad2b(uVar17,uVar18);
                    }
                  }
                }
              }
              else if ((local_14 == 0x27) && (param_1[0x1d0] == 3)) {
                local_48 = uRam00678bdc;
                piVar5 = param_1;
                if (DAT_005ccf98 != 0) {
                  iVar4 = FUN_00429a6d(uRam00678bdc);
                  piVar5 = (int *)0x0;
                  if (iVar4 != 0) {
                    FUN_00429a6d(local_48);
                    piVar5 = (int *)FUN_0042afd0();
                    if (piVar5 == (int *)0x0) {
                      uVar18 = 0;
                      uVar17 = 0;
                      FUN_00429a6d(local_48);
                      piVar5 = (int *)FUN_0042ad2b(uVar17,uVar18);
                    }
                  }
                }
              }
              else if (param_1[0x93] == 0x21) {
                piVar5 = param_1;
                if (DAT_005ccf98 != 0) {
                  iVar4 = FUN_00429a6d(0x94);
                  piVar5 = (int *)0x0;
                  if (iVar4 != 0) {
                    FUN_00429a6d(0x94);
                    piVar5 = (int *)FUN_0042afd0();
                    if (piVar5 == (int *)0x0) {
                      uVar18 = 0;
                      uVar17 = 0;
                      FUN_00429a6d(0x94);
                      piVar5 = (int *)FUN_0042ad2b(uVar17,uVar18);
                    }
                  }
                }
              }
              else if (local_14 == 0x28) {
                local_4c = uRam00678e7c;
                piVar5 = param_1;
                if (DAT_005ccf98 != 0) {
                  iVar4 = FUN_00429a6d(uRam00678e7c);
                  piVar5 = (int *)0x0;
                  if (iVar4 != 0) {
                    FUN_00429a6d(local_4c);
                    piVar5 = (int *)FUN_0042afd0();
                    if (piVar5 == (int *)0x0) {
                      uVar18 = 0;
                      uVar17 = 0;
                      FUN_00429a6d(local_4c);
                      piVar5 = (int *)FUN_0042ad2b(uVar17,uVar18);
                    }
                  }
                }
              }
              else {
                piVar5 = DAT_013b71e8;
                if (param_1 == *(int **)(DAT_013b71e8[0x6c21] + 0x54)) {
                  if (param_2 == 9) {
                    param_2 = 8;
                  }
                  uVar18 = 0;
                  uVar17 = 0;
                  FUN_00429a6d(param_2 * 2 + 0xb6);
                  piVar5 = (int *)FUN_0042ad2b(uVar17,uVar18);
                }
              }
            }
          }
        }
      }
      else if (((6 < local_1c) && (local_1c < 9)) && (DAT_005ccf98 != 0)) {
        iVar4 = FUN_00429a6d(param_2 * 2 + 0xb7);
        piVar5 = (int *)0x0;
        if (iVar4 != 0) {
          FUN_00429a6d(param_2 * 2 + 0xb7);
          piVar5 = (int *)FUN_0042afd0();
          if (piVar5 == (int *)0x0) {
            if (((local_14 == 0x14) && ((local_20 == 0 || (local_20 == 2)))) &&
               (param_1[0x1d0] == 3)) {
              local_50 = uRam006759fc;
              piVar5 = param_1;
              if (DAT_005ccf98 != 0) {
                iVar4 = FUN_00429a6d(uRam006759fc);
                piVar5 = (int *)0x0;
                if (iVar4 != 0) {
                  FUN_00429a6d(local_50);
                  piVar5 = (int *)FUN_0042afd0();
                  if (piVar5 == (int *)0x0) {
                    uVar18 = 0;
                    uVar17 = 0;
                    FUN_00429a6d(local_50);
                    piVar5 = (int *)FUN_0042ad2b(uVar17,uVar18);
                  }
                }
              }
            }
            else if ((local_14 == 0x27) && (param_1[0x1d0] == 3)) {
              local_54 = uRam00678bdc;
              piVar5 = param_1;
              if (DAT_005ccf98 != 0) {
                iVar4 = FUN_00429a6d(uRam00678bdc);
                piVar5 = (int *)0x0;
                if (iVar4 != 0) {
                  FUN_00429a6d(local_54);
                  piVar5 = (int *)FUN_0042afd0();
                  if (piVar5 == (int *)0x0) {
                    uVar18 = 0;
                    uVar17 = 0;
                    FUN_00429a6d(local_54);
                    piVar5 = (int *)FUN_0042ad2b(uVar17,uVar18);
                  }
                }
              }
            }
            else if (param_1[0x93] == 0x21) {
              piVar5 = param_1;
              if (DAT_005ccf98 != 0) {
                iVar4 = FUN_00429a6d(0x94);
                piVar5 = (int *)0x0;
                if (iVar4 != 0) {
                  FUN_00429a6d(0x94);
                  piVar5 = (int *)FUN_0042afd0();
                  if (piVar5 == (int *)0x0) {
                    uVar18 = 0;
                    uVar17 = 0;
                    FUN_00429a6d(0x94);
                    piVar5 = (int *)FUN_0042ad2b(uVar17,uVar18);
                  }
                }
              }
            }
            else if (local_14 == 0x28) {
              local_58 = uRam00678e7c;
              piVar5 = param_1;
              if (DAT_005ccf98 != 0) {
                iVar4 = FUN_00429a6d(uRam00678e7c);
                piVar5 = (int *)0x0;
                if (iVar4 != 0) {
                  FUN_00429a6d(local_58);
                  piVar5 = (int *)FUN_0042afd0();
                  if (piVar5 == (int *)0x0) {
                    uVar18 = 0;
                    uVar17 = 0;
                    FUN_00429a6d(local_58);
                    piVar5 = (int *)FUN_0042ad2b(uVar17,uVar18);
                  }
                }
              }
            }
            else {
              piVar5 = DAT_013b71e8;
              if (param_1 == *(int **)(DAT_013b71e8[0x6c21] + 0x54)) {
                if (param_2 == 9) {
                  param_2 = 8;
                }
                uVar18 = 0;
                uVar17 = 0;
                FUN_00429a6d(param_2 * 2 + 0xb7);
                piVar5 = (int *)FUN_0042ad2b(uVar17,uVar18);
              }
            }
          }
        }
      }
    }
  }
  ExceptionList = local_10;
  return piVar5;
}

