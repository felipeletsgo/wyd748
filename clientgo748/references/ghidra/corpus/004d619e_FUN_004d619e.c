// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d619e | Name: FUN_004d619e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004d619e(int *param_1)

{
  float fVar1;
  uint uVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  int iVar5;
  void *pvVar6;
  int iVar7;
  float10 fVar8;
  float10 fVar9;
  undefined4 uVar10;
  float *pfVar11;
  undefined1 *puVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  int *piVar15;
  float fVar16;
  int *local_288;
  int *local_26c;
  int *local_260;
  int *local_25c;
  int *local_258;
  int local_248;
  int local_23c;
  int local_230;
  undefined1 local_1d8 [12];
  undefined1 local_1cc [12];
  undefined1 local_1c0 [36];
  void *local_19c;
  int *local_198;
  void *local_188;
  int *local_184;
  void *local_180;
  int *local_17c;
  void *local_178;
  int *local_174;
  void *local_15c;
  int local_158;
  undefined1 local_14c [12];
  undefined1 local_140 [12];
  undefined1 local_134 [36];
  void *local_110;
  int local_10c;
  void *local_fc;
  int local_f8;
  undefined1 local_ec [12];
  undefined1 local_e0 [12];
  undefined1 local_d4 [24];
  float local_bc;
  float local_b8;
  float local_b4;
  int local_b0;
  int *local_ac;
  undefined4 local_a8;
  int local_a4;
  undefined4 local_a0;
  float local_9c;
  undefined4 local_98 [2];
  undefined4 local_90;
  int *local_8c;
  int *local_88;
  float local_84;
  float local_80;
  int local_7c;
  float local_78;
  float local_74;
  float local_70;
  float local_6c;
  float local_68;
  int local_64;
  float local_60;
  int local_5c;
  float local_58;
  undefined4 local_54 [2];
  undefined4 local_4c;
  float local_48;
  int local_44;
  int local_40;
  undefined4 local_3c;
  int local_38;
  undefined4 local_34;
  float local_30;
  undefined4 local_2c [2];
  undefined4 local_24;
  float local_20;
  void *local_1c [4];
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0b37;
  local_1c[3] = ExceptionList;
  ExceptionList = local_1c + 3;
  uVar2 = (**(code **)(*DAT_0092e654 + 8))();
  if (param_1[100] == 0) {
    param_1[100] = uVar2;
  }
  if ((uint)param_1[100] <= uVar2) {
    if ((param_1[0x65] == 0) || (uVar2 - param_1[100] <= (uint)param_1[0x65])) {
      if ((param_1[0x65] == 0) || (uVar2 - param_1[100] < 0x2711)) {
        local_1c[0] = (void *)0xffffaa00;
        local_1c[1] = (void *)0xffff0000;
        local_1c[2] = (void *)0xffffaaff;
        local_20 = (float)(uVar2 - param_1[100]) / (float)param_1[0x65];
        if (_DAT_005a45d4 <= local_20) {
          if ((param_1[0x75] == 1) && (param_1[0x65] != 0)) {
            fVar8 = (float10)FUN_00493bf0((local_20 * _DAT_005a4148) / _DAT_005a365c);
            param_1[0x71] = (int)(float)(fVar8 * (float10)(float)param_1[0x69]);
            fVar8 = (float10)FUN_00493bf0((local_20 * _DAT_005a4148) / _DAT_005a365c);
            param_1[0x72] = (int)(float)(fVar8 * (float10)(float)param_1[0x6a]);
            fVar8 = (float10)FUN_00493bf0((local_20 * _DAT_005a4148) / _DAT_005a365c);
            param_1[0x73] = (int)(float)(fVar8 * (float10)(float)param_1[0x6b]);
          }
          else if ((param_1[0x75] == 2) && (param_1[0x65] != 0)) {
            fVar8 = (float10)FUN_00493bf0((local_20 * _DAT_005a4148) / _DAT_005a365c);
            fVar16 = (float)param_1[0x69];
            fVar9 = (float10)FUN_00493bf0(((_DAT_005a3660 - local_20) * _DAT_005a4148) /
                                          _DAT_005a365c);
            param_1[0x71] =
                 (int)(float)(fVar9 * (float10)(float)param_1[0x6d] +
                             (float10)(float)(fVar8 * (float10)fVar16));
            fVar8 = (float10)FUN_00493bf0((local_20 * _DAT_005a4148) / _DAT_005a365c);
            fVar16 = (float)param_1[0x6a];
            fVar9 = (float10)FUN_00493bf0(((_DAT_005a3660 - local_20) * _DAT_005a4148) /
                                          _DAT_005a365c);
            param_1[0x72] =
                 (int)(float)(fVar9 * (float10)(float)param_1[0x6e] +
                             (float10)(float)(fVar8 * (float10)fVar16));
            fVar8 = (float10)FUN_00493bf0((local_20 * _DAT_005a4148) / _DAT_005a365c);
            fVar16 = (float)param_1[0x6b];
            fVar9 = (float10)FUN_00493bf0(((_DAT_005a3660 - local_20) * _DAT_005a4148) /
                                          _DAT_005a365c);
            param_1[0x73] =
                 (int)(float)(fVar9 * (float10)(float)param_1[0x6f] +
                             (float10)(float)(fVar8 * (float10)fVar16));
          }
          if ((local_20 < _DAT_005a3660) && (param_1[7] != 0)) {
            param_1[0x79] = *(int *)(param_1[7] + 0x28);
            param_1[0x7b] = *(int *)(param_1[7] + 0x2c);
            param_1[0x7a] = *(int *)(param_1[7] + 0x30);
          }
          if (param_1[0x67] == 1) {
            param_1[0xc] = (int)((float)(uVar2 - param_1[100]) / _DAT_005a43c0 +
                                (float)param_1[0x77]);
          }
          else if (param_1[0x67] == 2) {
            FUN_00493db0(local_2c,param_1 + 0x76);
            if (param_1[7] != 0) {
              piVar15 = param_1 + 0x76;
              puVar12 = local_d4;
              FUN_00430f50(*(undefined4 *)(param_1[7] + 0x28),*(undefined4 *)(param_1[7] + 0x30),
                           *(undefined4 *)(param_1[7] + 0x2c));
              uVar3 = FUN_00493db0(puVar12,piVar15);
              FUN_004310a0(uVar3);
            }
            fVar8 = (float10)FUN_00493c30(local_2c[0],local_24);
            param_1[0x7d] = (int)(float)(fVar8 + (float10)_DAT_005a4380);
            local_30 = (float)(uVar2 - param_1[100]) / (float)param_1[0x65];
            if (local_30 < _DAT_005a3660) {
              uVar3 = FUN_004c8c60(local_ec,local_30);
              puVar4 = &local_3c;
              FUN_004c8c60(local_e0,_DAT_005a3660 - local_30);
              FUN_00493d50(puVar4,uVar3);
              uVar3 = FUN_00430f20(local_3c,local_34);
              FUN_0040bd30(uVar3);
              param_1[0xc] = local_38;
              local_fc = operator_new(0x90);
              local_8 = 0;
              if (local_fc == (void *)0x0) {
                local_230 = 0;
              }
              else {
                uVar13 = 0;
                uVar10 = 1;
                uVar3 = 2;
                pvVar6 = local_1c[param_1[0x7c]];
                puVar4 = (undefined4 *)FUN_00430f50(param_1[10],param_1[0xc],param_1[0xb]);
                local_230 = FUN_004ec9a0(*puVar4,puVar4[1],puVar4[2],pvVar6,uVar3,uVar10,uVar13);
              }
              local_f8 = local_230;
              local_8 = 0xffffffff;
              local_40 = local_230;
              if (local_230 != 0) {
                FUN_0054ac09(local_230);
              }
            }
            if (param_1[0x7c] == 0) {
              *(undefined4 *)(param_1[0x18] + 0x2e4) = 5;
              FUN_004bd906(*(undefined4 *)(&DAT_00672498 + param_1[0x17] * 0x2a0));
            }
            else {
              *(undefined4 *)(param_1[0x18] + 0x2e4) = 4;
              FUN_004bd906(*(undefined4 *)(&DAT_0067249c + param_1[0x17] * 0x2a0));
            }
          }
          else if (param_1[0x67] == 3) {
            local_48 = ((float)((uVar2 - param_1[100]) % 1000) * _DAT_005a4290) / _DAT_005a43c0 +
                       (float)param_1[0x68];
            param_1[0x7d] = (int)local_48;
            if (param_1[7] == 0) {
              fVar8 = (float10)FUN_00493bf0(local_48);
              param_1[10] = (int)(float)(fVar8 * (float10)_DAT_005a3660 +
                                        (float10)(float)param_1[0x76]);
              fVar8 = (float10)FUN_00423b00(local_48);
              param_1[0xb] = (int)(float)(fVar8 * (float10)_DAT_005a3660 +
                                         (float10)(float)param_1[0x78]);
              param_1[0xc] = (int)(((float)param_1[0x77] + (float)param_1[0x7a]) / _DAT_005a365c);
            }
            else {
              fVar8 = (float10)FUN_00493bf0(local_48);
              param_1[10] = (int)(float)(fVar8 * (float10)_DAT_005a3660 +
                                        (float10)*(float *)(param_1[7] + 0x28));
              fVar8 = (float10)FUN_00423b00(local_48);
              param_1[0xb] = (int)(float)(fVar8 * (float10)_DAT_005a3660 +
                                         (float10)*(float *)(param_1[7] + 0x2c));
              param_1[0xc] = (int)(*(float *)(param_1[7] + 0x30) + _DAT_005a34a0);
            }
            local_110 = operator_new(0x90);
            local_8 = 1;
            if (local_110 == (void *)0x0) {
              local_23c = 0;
            }
            else {
              uVar14 = 0;
              uVar13 = 1;
              uVar10 = 2;
              uVar3 = 0xffff0000;
              puVar4 = (undefined4 *)FUN_00430f50(param_1[10],param_1[0xc],param_1[0xb]);
              local_23c = FUN_004ec9a0(*puVar4,puVar4[1],puVar4[2],uVar3,uVar10,uVar13,uVar14);
            }
            local_10c = local_23c;
            local_8 = 0xffffffff;
            local_44 = local_23c;
            if (local_23c != 0) {
              FUN_0054ac09(local_23c);
            }
            *(undefined4 *)(param_1[0x18] + 0x2e4) = 5;
            FUN_004bd906(*(undefined4 *)(&DAT_0067249c + param_1[0x17] * 0x2a0));
          }
          else if (param_1[0x67] == 4) {
            FUN_00493db0(local_54,param_1 + 0x76);
            if (param_1[7] != 0) {
              piVar15 = param_1 + 0x76;
              puVar12 = local_134;
              FUN_00430f50(*(undefined4 *)(param_1[7] + 0x28),*(undefined4 *)(param_1[7] + 0x30),
                           *(undefined4 *)(param_1[7] + 0x2c));
              uVar3 = FUN_00493db0(puVar12,piVar15);
              FUN_004310a0(uVar3);
            }
            fVar8 = (float10)FUN_00493c30(local_54[0],local_4c);
            param_1[0x7d] = (int)(float)(fVar8 + (float10)_DAT_005a4380);
            if (local_20 < _DAT_005a3660) {
              iVar7 = param_1[100];
              iVar5 = __ftol();
              local_68 = ((float)((iVar5 + (uVar2 - iVar7)) % 1000) * _DAT_005a4290) / _DAT_005a43c0
                         + (float)param_1[0x68];
              uVar3 = FUN_004c8c60(local_14c,local_20);
              pfVar11 = &local_60;
              FUN_004c8c60(local_140,_DAT_005a3660 - local_20);
              FUN_00493d50(pfVar11,uVar3);
              fVar8 = (float10)FUN_00423b00(local_68);
              fVar16 = (float)(fVar8 * (float10)_DAT_005a4158 + (float10)local_58);
              fVar8 = (float10)FUN_00493bf0(local_68,fVar16);
              uVar3 = FUN_00430f20((float)(fVar8 * (float10)_DAT_005a4158 + (float10)local_60),
                                   fVar16);
              FUN_0040bd30(uVar3);
              param_1[0xc] = local_5c;
              local_15c = operator_new(0x90);
              local_8 = 2;
              if (local_15c == (void *)0x0) {
                local_248 = 0;
              }
              else {
                uVar13 = 0;
                uVar10 = 1;
                uVar3 = 2;
                pvVar6 = local_1c[param_1[0x7c]];
                puVar4 = (undefined4 *)FUN_00430f50(param_1[10],param_1[0xc],param_1[0xb]);
                local_248 = FUN_004ec9a0(*puVar4,puVar4[1],puVar4[2],pvVar6,uVar3,uVar10,uVar13);
              }
              local_158 = local_248;
              local_8 = 0xffffffff;
              local_64 = local_248;
              if (local_248 != 0) {
                FUN_0054ac09(local_248);
              }
            }
            *(undefined4 *)(param_1[0x18] + 0x2e4) = 5;
            FUN_004bd906(*(undefined4 *)(&DAT_00672498 + param_1[0x17] * 0x2a0));
          }
          else if ((param_1[0x67] == 5) || (param_1[0x67] == 7)) {
            if (param_1[7] != 0) {
              local_78 = 2.0;
              local_74 = 0.0;
              iVar7 = param_1[100];
              iVar5 = __ftol();
              local_80 = ((float)((iVar5 + (uVar2 - iVar7)) % 1000) * _DAT_005a4290) / _DAT_005a43c0
              ;
              if (param_1[0x67] == 7) {
                local_78 = 1.9;
                local_80 = local_80 + _DAT_005a46d0;
                local_74 = 0.7853982;
              }
              iVar7 = param_1[7];
              fVar8 = (float10)FUN_00423b00((*(float *)(param_1[7] + 0x34) - _DAT_005a4148) -
                                            local_74);
              fVar16 = (float)(fVar8 * (float10)_DAT_005a40ec + (float10)*(float *)(iVar7 + 0x2c));
              iVar7 = param_1[7];
              fVar8 = (float10)FUN_00493bf0((*(float *)(param_1[7] + 0x34) - _DAT_005a4148) -
                                            local_74,fVar16);
              FUN_00430f20((float)(fVar8 * (float10)_DAT_005a40ec +
                                  (float10)*(float *)(iVar7 + 0x28)),fVar16);
              fVar8 = (float10)FUN_00423b00(local_80);
              fVar16 = (float)(fVar8 * (float10)_DAT_005a4214 + (float10)local_6c);
              fVar8 = (float10)FUN_00493bf0(local_80,fVar16);
              uVar3 = FUN_00430f20((float)(fVar8 * (float10)_DAT_005a4214 + (float10)local_70),
                                   fVar16);
              FUN_0040bd30(uVar3);
              fVar1 = local_78 * *(float *)(param_1[7] + 0x38);
              fVar16 = *(float *)(param_1[7] + 0x30);
              fVar8 = (float10)FUN_00423b00(local_80 + local_74);
              param_1[0xc] = (int)(float)(fVar8 * (float10)_DAT_005a439c + (float10)(fVar1 + fVar16)
                                         );
              param_1[0x7d] = *(int *)(param_1[7] + 0x34);
              local_7c = _rand();
              local_7c = local_7c % 5;
              if (param_1[0x67] == 5) {
                local_84 = 1.0;
                if (param_1[0x7c] == 3) {
                  *(undefined4 *)(param_1[0x18] + 0x2e4) = 5;
                }
                if (((param_1[0x7c] == 4) || (param_1[0x7c] == 5)) || (param_1[0x7c] == 6)) {
                  *(undefined4 *)(param_1[0x18] + 0x2e4) = 10;
                  local_84 = 2.0;
                }
                FUN_004bd906(*(undefined4 *)(&DAT_00672498 + param_1[0x17] * 0x2a0));
                if (DAT_0092e660 == 0) {
                  local_88 = (int *)0x0;
                  if (param_1[0x7c] == 5) {
                    local_178 = operator_new(0x180);
                    local_8 = 3;
                    if (local_178 == (void *)0x0) {
                      local_258 = (int *)0x0;
                    }
                    else {
                      local_258 = (int *)FUN_004c8f70(2,0x9c4,_DAT_005a4ae0 * local_84,
                                                      _DAT_005a4ae0 * local_84,
                                                      _DAT_005a4ae0 * local_84,0,1,0x50);
                    }
                    local_174 = local_258;
                    local_88 = local_258;
                  }
                  else if (param_1[0x7c] == 6) {
                    local_180 = operator_new(0x180);
                    local_8 = 4;
                    if (local_180 == (void *)0x0) {
                      local_25c = (int *)0x0;
                    }
                    else {
                      local_25c = (int *)FUN_004c8f70(200,0x5dc,_DAT_005a41e8 * local_84,
                                                      _DAT_005a41e8 * local_84,
                                                      _DAT_005a41e8 * local_84,0,1,0x50);
                    }
                    local_17c = local_25c;
                    local_88 = local_25c;
                  }
                  else {
                    local_188 = operator_new(0x180);
                    local_8 = 5;
                    if (local_188 == (void *)0x0) {
                      local_260 = (int *)0x0;
                    }
                    else {
                      local_260 = (int *)FUN_004c8f70(0,0x5dc,_DAT_005a45d4 * local_84 +
                                                              (float)local_7c * _DAT_005a45d4,
                                                      _DAT_005a45d4 * local_84 +
                                                      (float)local_7c * _DAT_005a4214,
                                                      _DAT_005a45d4 * local_84 +
                                                      (float)local_7c * _DAT_005a45d4,0x38d1b717,1,
                                                      0x50);
                    }
                    local_184 = local_260;
                    local_88 = local_260;
                  }
                  local_8 = 0xffffffff;
                  if (local_88 != (int *)0x0) {
                    iVar7 = _rand();
                    fVar16 = (float)(iVar7 % 10 + -5) * _DAT_005a41e8 + (float)param_1[0xb];
                    iVar7 = param_1[0xc];
                    iVar5 = _rand();
                    uVar3 = FUN_00430f50((float)(iVar5 % 10 + -5) * _DAT_005a41e8 +
                                         (float)param_1[10],iVar7,fVar16);
                    uVar3 = FUN_004310a0(uVar3);
                    FUN_004310a0(uVar3);
                    local_88[0x1b] = 1;
                    local_88[0x40] = 0;
                    local_88[0x52] = 1;
                    if (param_1[0x7c] == 5) {
                      local_88[0x50] = -0x40400000;
                    }
                    else {
                      local_88[0x50] = -0x41000000;
                    }
                    if (param_1[0x7c] == 3) {
                      (**(code **)(*local_88 + 0x3c))(0xffffaaff);
                    }
                    if (param_1[0x7c] == 4) {
                      (**(code **)(*local_88 + 0x3c))(0xffaaffee);
                    }
                    if (param_1[0x7c] == 5) {
                      (**(code **)(*local_88 + 0x3c))(0xffeeeeff);
                    }
                    FUN_0054ac09(local_88);
                  }
                }
              }
              else {
                *(undefined4 *)(param_1[0x18] + 0x2e4) = 4;
                FUN_004bd906(*(undefined4 *)(&DAT_0067249c + param_1[0x17] * 0x2a0));
                if (DAT_0092e660 == 0) {
                  local_19c = operator_new(0x180);
                  local_8 = 6;
                  if (local_19c == (void *)0x0) {
                    local_26c = (int *)0x0;
                  }
                  else {
                    local_26c = (int *)FUN_004c8f70(0,0x5dc,(float)local_7c * _DAT_005a45d4 +
                                                            _DAT_005a41e8,
                                                    (float)local_7c * _DAT_005a4214 + _DAT_005a41e8,
                                                    (float)local_7c * _DAT_005a45d4 + _DAT_005a41e8,
                                                    0x38d1b717,1,0x50);
                  }
                  local_198 = local_26c;
                  local_8 = 0xffffffff;
                  local_8c = local_26c;
                  if (local_26c != (int *)0x0) {
                    iVar7 = _rand();
                    fVar16 = (float)(iVar7 % 10 + -5) * _DAT_005a41e8 + (float)param_1[0xb];
                    iVar7 = param_1[0xc];
                    iVar5 = _rand();
                    uVar3 = FUN_00430f50((float)(iVar5 % 10 + -5) * _DAT_005a41e8 +
                                         (float)param_1[10],iVar7,fVar16);
                    uVar3 = FUN_004310a0(uVar3);
                    FUN_004310a0(uVar3);
                    local_8c[0x1b] = 1;
                    local_8c[0x40] = 0;
                    local_8c[0x52] = 1;
                    local_8c[0x50] = 0x3f000000;
                    if (param_1[0x7c] == 3) {
                      (**(code **)(*local_8c + 0x3c))(0xffff0000);
                    }
                    FUN_0054ac09(local_8c);
                  }
                }
              }
            }
          }
          else if (param_1[0x67] == 6) {
            FUN_00493db0(local_98,param_1 + 0x76);
            if (param_1[7] != 0) {
              piVar15 = param_1 + 0x76;
              puVar12 = local_1c0;
              FUN_00430f50(*(undefined4 *)(param_1[7] + 0x28),*(undefined4 *)(param_1[7] + 0x30),
                           *(undefined4 *)(param_1[7] + 0x2c));
              uVar3 = FUN_00493db0(puVar12,piVar15);
              FUN_004310a0(uVar3);
            }
            fVar8 = (float10)FUN_00493c30(local_98[0],local_90);
            param_1[0x7d] = (int)(float)(fVar8 + (float10)_DAT_005a4380);
            local_9c = (float)(uVar2 - param_1[100]) / (float)param_1[0x65];
            if (local_9c < _DAT_005a3660) {
              uVar3 = FUN_004c8c60(local_1d8,local_9c);
              puVar4 = &local_a8;
              FUN_004c8c60(local_1cc,_DAT_005a3660 - local_9c);
              FUN_00493d50(puVar4,uVar3);
              uVar3 = FUN_00430f20(local_a8,local_a0);
              FUN_0040bd30(uVar3);
              param_1[0xc] = local_a4;
            }
            if (DAT_0092e660 == 0) {
              local_b0 = _rand();
              local_b0 = local_b0 % 5;
              pvVar6 = operator_new(0x180);
              local_8 = 7;
              if (pvVar6 == (void *)0x0) {
                local_288 = (int *)0x0;
              }
              else {
                local_288 = (int *)FUN_004c8f70(0,0x5dc,(float)local_b0 * _DAT_005a439c +
                                                        _DAT_005a41e8,
                                                (float)local_b0 * _DAT_005a4214 + _DAT_005a41e8,
                                                (float)local_b0 * _DAT_005a439c + _DAT_005a41e8,
                                                0x3a03126f,1,0x50);
              }
              local_8 = 0xffffffff;
              local_ac = local_288;
              if (local_288 != (int *)0x0) {
                iVar7 = _rand();
                fVar16 = (float)(iVar7 % 10 + -5) * _DAT_005a41e8 + (float)param_1[0xb];
                iVar7 = param_1[0xc];
                iVar5 = _rand();
                uVar3 = FUN_00430f50((float)(iVar5 % 10 + -5) * _DAT_005a41e8 + (float)param_1[10],
                                     iVar7,fVar16);
                uVar3 = FUN_004310a0(uVar3);
                FUN_004310a0(uVar3);
                local_ac[0x1b] = 1;
                local_ac[0x40] = 0;
                local_ac[0x52] = 1;
                local_ac[0x50] = 0x3f800000;
                (**(code **)(*local_ac + 0x3c))(0xffaa8877);
                FUN_0054ac09(local_ac);
              }
            }
          }
          else if (param_1[0x67] == 8) {
            param_1[0xc] = (int)(float)((float10)(float)param_1[0x77] -
                                       ((float10)(uVar2 - param_1[100]) +
                                       (float10)(uVar2 - param_1[100])) / (float10)_DAT_005a43c0);
          }
          else if (param_1[0x67] == 9) {
            local_b4 = (float)(uVar2 - param_1[100]) / (float)param_1[0x65];
            if (local_b4 < _DAT_005a3660) {
              param_1[0xc] = (int)((_DAT_005a3660 - local_b4) * _DAT_005a40f0 + (float)param_1[0x77]
                                  );
            }
          }
          else if (param_1[0x67] == 10) {
            if (param_1[7] != 0) {
              param_1[10] = (int)((*(float *)(param_1[7] + 0x70) + *(float *)(param_1[7] + 0x7c)) /
                                 _DAT_005a365c);
              param_1[0xb] = (int)((*(float *)(param_1[7] + 0x78) + *(float *)(param_1[7] + 0x84)) /
                                  _DAT_005a365c);
              param_1[0xc] = (int)((*(float *)(param_1[7] + 0x74) + *(float *)(param_1[7] + 0x80)) /
                                  _DAT_005a365c);
              param_1[0x7d] = *(int *)(param_1[7] + 0x34);
            }
            if (param_1[0x18] != 0) {
              local_bc = (float)(uVar2 - param_1[100]) / (float)param_1[0x65];
              local_b8 = 0.57;
              if (0.57 < local_bc) {
                fVar8 = (float10)FUN_00493bf0(((local_bc - 0.57) * _DAT_005a4148) /
                                              ((_DAT_005a3660 - 0.57) + (_DAT_005a3660 - 0.57)));
                param_1[0x71] = (int)(float)(fVar8 * (float10)(float)param_1[0x71]);
                fVar8 = (float10)FUN_00493bf0(((local_bc - local_b8) * _DAT_005a4148) /
                                              ((_DAT_005a3660 - local_b8) +
                                              (_DAT_005a3660 - local_b8)));
                param_1[0x72] = (int)(float)(fVar8 * (float10)(float)param_1[0x72]);
                fVar8 = (float10)FUN_00493bf0(((local_bc - local_b8) * _DAT_005a4148) /
                                              ((_DAT_005a3660 - local_b8) +
                                              (_DAT_005a3660 - local_b8)));
                param_1[0x73] = (int)(float)(fVar8 * (float10)(float)param_1[0x73]);
                *(float *)(param_1[0x18] + 0x24) =
                     (local_bc - local_b8) * _DAT_005a3660 + _DAT_005a3660;
                *(float *)(param_1[0x18] + 0x28) =
                     (local_bc - local_b8) * _DAT_005a3660 + _DAT_005a3660;
                *(float *)(param_1[0x18] + 0x2c) =
                     (local_bc - local_b8) * _DAT_005a3660 + _DAT_005a3660;
              }
              *(uint *)(param_1[0x18] + 0x2e4) = (uint)(param_1[0x65] * 0xf) / 1000;
              FUN_004bd906(*(undefined4 *)(&DAT_00672498 + param_1[0x17] * 0x2a0));
            }
          }
          (**(code **)(*param_1 + 0x44))(param_1[10],param_1[0xc],param_1[0xb]);
          (**(code **)(*param_1 + 0x48))(0,param_1[0x7d],0);
          if (((param_1[0x18] != 0) && (iVar7 = (**(code **)(*param_1 + 0x4c))(), iVar7 == 1)) &&
             (FUN_004bda22(uVar2), param_1[0x87] != 0)) {
            FUN_004bda22(uVar2);
          }
        }
      }
      else {
        (**(code **)(*DAT_013b71e8 + 100))(param_1);
      }
    }
    else {
      (**(code **)(*DAT_013b71e8 + 100))(param_1);
    }
  }
  ExceptionList = local_1c[3];
  return 1;
}

