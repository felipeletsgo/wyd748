// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00500ebe | Name: FUN_00500ebe


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00500ebe(int param_1)

{
  void *pvVar1;
  undefined4 uVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  float10 fVar6;
  float fVar7;
  float fVar8;
  int *local_2f0;
  int *local_2e0;
  int *local_2d4;
  int *local_2c8;
  int *local_2bc;
  int *local_2b0;
  int *local_2ac;
  int *local_2a0;
  int *local_29c;
  int local_298;
  int *local_28c;
  int *local_280;
  int *local_274;
  int *local_268;
  int *local_264;
  int local_148;
  int local_144;
  int local_138 [5];
  int *local_124;
  int local_120;
  int local_11c;
  int local_118;
  int local_114;
  int local_110;
  int *local_10c;
  int local_108;
  int local_104;
  int *local_100;
  int local_fc;
  int local_f8;
  undefined4 local_f4;
  int *local_f0;
  int local_ec;
  int local_e8;
  int local_e4;
  int *local_e0;
  int local_dc;
  int *local_d8;
  uint local_d4;
  undefined1 local_d0 [24];
  int local_b8;
  int *local_b4;
  int local_b0;
  int local_ac;
  uint local_a8;
  undefined4 local_a4;
  undefined4 local_a0;
  int local_9c;
  int *local_98;
  int local_94;
  int local_90;
  int local_8c;
  int local_88;
  float local_84;
  undefined4 local_80;
  uint local_7c;
  int *local_78;
  int local_74;
  int local_70;
  int local_6c;
  int *local_68;
  int *local_64;
  int local_60;
  int *local_5c;
  int local_58;
  int local_54;
  int *local_50;
  int local_4c;
  undefined4 local_48;
  undefined4 local_44;
  float local_40;
  uint local_3c;
  int local_38;
  int local_34 [9];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a1ab7;
  local_10 = ExceptionList;
  if (*(int *)(param_1 + 0x214) == 0) {
    ExceptionList = &local_10;
    local_34[8] = (**(code **)(*DAT_0092e654 + 8))();
    if ((*(int *)(param_1 + 0x56c) == 0) ||
       (((*(int *)(param_1 + 0x24c) != 0x24 && (*(int *)(param_1 + 0x24c) != 0x25)) &&
        (*(char *)(param_1 + 0x796) != '\x01')))) {
      if (*(int *)(param_1 + 0x24c) == 0x20) {
        if (*(short *)(param_1 + 0x1d2) == 2) {
          for (local_38 = 0; local_38 < 6; local_38 = local_38 + 1) {
            if (*(int *)(param_1 + 0x56c + local_38 * 4) != 0) {
              FUN_004310a0(param_1 + 100 + local_38 * 0xc);
              *(float *)(*(int *)(param_1 + 0x56c + local_38 * 4) + 0x78) =
                   _DAT_005a34a0 * *(float *)(param_1 + 0x38) +
                   *(float *)(*(int *)(param_1 + 0x56c + local_38 * 4) + 0x78);
              (**(code **)(**(int **)(param_1 + 0x56c + local_38 * 4) + 0x20))(0);
            }
          }
        }
        else if ((*(short *)(param_1 + 0x1d2) == 1) && (*(short *)(param_1 + 0x1d4) == 0)) {
          local_3c = 500;
          local_48 = 0x38d1b717;
          local_40 = 1.0;
          local_44 = 0x77;
          if (*(short *)(param_1 + 0x1d4) == 2) {
            local_44 = 0x98;
            local_48 = 0x3951b717;
            local_40 = 0.3;
            local_3c = 600;
          }
          if (local_3c < (uint)(local_34[8] - *(int *)(param_1 + 0x640))) {
            for (local_4c = 0; local_4c < 6; local_4c = local_4c + 1) {
              pvVar1 = operator_new(0x180);
              local_8 = 0;
              if (pvVar1 == (void *)0x0) {
                local_264 = (int *)0x0;
              }
              else {
                local_264 = (int *)FUN_004c8f70(local_44,3000,
                                                local_40 * *(float *)(param_1 + 0x38) +
                                                _DAT_005a4214,
                                                local_40 * *(float *)(param_1 + 0x38) +
                                                _DAT_005a40ec,
                                                local_40 * *(float *)(param_1 + 0x38) +
                                                _DAT_005a4214,local_48,1,0x50);
              }
              local_8 = 0xffffffff;
              local_50 = local_264;
              if (local_264 != (int *)0x0) {
                if (*(short *)(param_1 + 0x1d4) == 2) {
                  (**(code **)(*local_264 + 0x3c))(0xaaffffff);
                  local_50[0x52] = 1;
                  local_50[0x50] = -0x40800000;
                }
                local_50[0x41] = 3;
                uVar2 = FUN_004310a0(param_1 + 100 + local_4c * 0xc);
                FUN_004310a0(uVar2);
                FUN_0054ac09(local_50);
              }
            }
            *(int *)(param_1 + 0x640) = local_34[8];
          }
        }
      }
      else if (((*(int *)(param_1 + 0x24c) == 0x22) || (*(int *)(param_1 + 0x24c) == 0x17)) ||
              ((*(int *)(param_1 + 0x24c) == 0x15 && (*(short *)(param_1 + 0x1d2) == 10)))) {
        for (local_54 = 0; local_54 < 2; local_54 = local_54 + 1) {
          if (*(int *)(param_1 + 0x56c + local_54 * 4) != 0) {
            FUN_004310a0(param_1 + 100 + local_54 * 0xc);
            (**(code **)(**(int **)(param_1 + 0x56c + local_54 * 4) + 0x20))(0);
          }
        }
      }
      else if ((*(int *)(param_1 + 0x24c) == 0x10) && (*(short *)(param_1 + 0x1d2) == 6)) {
        if (300 < (uint)(local_34[8] - *(int *)(param_1 + 0x640))) {
          for (local_58 = 1; local_58 < 8; local_58 = local_58 + 1) {
            pvVar1 = operator_new(0x180);
            local_8 = 1;
            if (pvVar1 == (void *)0x0) {
              local_268 = (int *)0x0;
            }
            else {
              local_268 = (int *)FUN_004c8f70(0,0x9c4,_DAT_005a430c * *(float *)(param_1 + 0x38),
                                              _DAT_005a430c * *(float *)(param_1 + 0x38),
                                              _DAT_005a430c * *(float *)(param_1 + 0x38),0x3a83126f,
                                              1,0x50);
            }
            local_8 = 0xffffffff;
            local_5c = local_268;
            if (local_268 != (int *)0x0) {
              iVar5 = local_58 * 0xc;
              iVar3 = _rand();
              fVar7 = (float)(iVar3 % 10 + -5) * _DAT_005a439c + *(float *)(param_1 + 0x6c + iVar5);
              uVar2 = *(undefined4 *)(param_1 + 0x68 + local_58 * 0xc);
              iVar5 = local_58 * 0xc;
              iVar3 = _rand();
              uVar2 = FUN_00430f50((float)(iVar3 % 10 + -5) * _DAT_005a439c +
                                   *(float *)(param_1 + 100 + iVar5),uVar2,fVar7);
              FUN_004310a0(uVar2);
              local_5c[0x1b] = 1;
              (**(code **)(*local_5c + 0x3c))(0xff00aa66);
              FUN_0054ac09(local_5c);
            }
          }
          *(int *)(param_1 + 0x640) = local_34[8];
        }
      }
      else if ((*(int *)(param_1 + 0x24c) == 0x10) &&
              ((*(short *)(param_1 + 0x1d2) == 0 && (*(short *)(param_1 + 0x1d4) == 1)))) {
        if (300 < (uint)(local_34[8] - *(int *)(param_1 + 0x640))) {
          pvVar1 = operator_new(0x180);
          local_8 = 2;
          if (pvVar1 == (void *)0x0) {
            local_274 = (int *)0x0;
          }
          else {
            local_274 = (int *)FUN_004c8f70(0,0x9c4,_DAT_005a4214 * *(float *)(param_1 + 0x38),
                                            _DAT_005a4214 * *(float *)(param_1 + 0x38),
                                            _DAT_005a4214 * *(float *)(param_1 + 0x38),0x3a83126f,1,
                                            0x50);
          }
          local_8 = 0xffffffff;
          local_64 = local_274;
          if (local_274 != (int *)0x0) {
            iVar3 = _rand();
            fVar7 = (float)(iVar3 % 10 + -5) * _DAT_005a439c + *(float *)(param_1 + 0x6c);
            uVar2 = *(undefined4 *)(param_1 + 0x68);
            iVar3 = _rand();
            uVar2 = FUN_00430f50((float)(iVar3 % 10 + -5) * _DAT_005a439c +
                                 *(float *)(param_1 + 100),uVar2,fVar7);
            FUN_004310a0(uVar2);
            local_64[0x1b] = 1;
            (**(code **)(*local_64 + 0x3c))(0xffaaaaaa);
            FUN_0054ac09(local_64);
          }
          *(int *)(param_1 + 0x640) = local_34[8];
        }
        for (local_60 = 8; local_60 < 10; local_60 = local_60 + 1) {
          if (*(int *)(param_1 + 0x56c + local_60 * 4) != 0) {
            FUN_004310a0(param_1 + 100 + local_60 * 0xc);
            (**(code **)(**(int **)(param_1 + 0x56c + local_60 * 4) + 0x20))(0);
          }
        }
      }
      else if ((*(int *)(param_1 + 0x24c) == 0x1e) &&
              (((*(short *)(param_1 + 0x1d2) == 0 || (*(short *)(param_1 + 0x1d2) == 1)) ||
               (*(short *)(param_1 + 0x1d2) == 2)))) {
        if (300 < (uint)(local_34[8] - *(int *)(param_1 + 0x640))) {
          pvVar1 = operator_new(0x180);
          local_8 = 3;
          if (pvVar1 == (void *)0x0) {
            local_280 = (int *)0x0;
          }
          else {
            local_280 = (int *)FUN_004c8f70(0,0x9c4,_DAT_005a43e8 * *(float *)(param_1 + 0x38),
                                            _DAT_005a43e8 * *(float *)(param_1 + 0x38),
                                            _DAT_005a43e8 * *(float *)(param_1 + 0x38),0x3a83126f,1,
                                            0x50);
          }
          local_8 = 0xffffffff;
          local_68 = local_280;
          if (local_280 != (int *)0x0) {
            iVar3 = _rand();
            fVar7 = (float)(iVar3 % 10 + -5) * _DAT_005a439c + *(float *)(param_1 + 0x6c);
            uVar2 = *(undefined4 *)(param_1 + 0x68);
            iVar3 = _rand();
            uVar2 = FUN_00430f50((float)(iVar3 % 10 + -5) * _DAT_005a439c +
                                 *(float *)(param_1 + 100),uVar2,fVar7);
            FUN_004310a0(uVar2);
            local_68[0x1b] = 1;
            (**(code **)(*local_68 + 0x3c))(0xffaaaaaa);
            FUN_0054ac09(local_68);
          }
          *(int *)(param_1 + 0x640) = local_34[8];
        }
      }
      else if (((*(int *)(param_1 + 0x24c) == 0x1e) && (*(short *)(param_1 + 0x1d2) == 4)) ||
              ((*(int *)(param_1 + 0x24c) == 0x26 &&
               ((*(short *)(param_1 + 0x1da) == 0xe && (*(char *)(param_1 + 0x79b) == '\0')))))) {
        if (100 < (uint)(local_34[8] - *(int *)(param_1 + 0x640))) {
          local_6c = 1;
          if (*(int *)(param_1 + 0x24c) == 0x26) {
            local_6c = 2;
          }
          for (local_70 = 0; local_70 < 2; local_70 = local_70 + 1) {
            for (local_74 = 0; local_74 < 1; local_74 = local_74 + 1) {
              pvVar1 = operator_new(0x180);
              local_8 = 4;
              if (pvVar1 == (void *)0x0) {
                local_28c = (int *)0x0;
              }
              else {
                local_28c = (int *)FUN_004c8f70(0,local_70 * 400 + 0x5dc,0x3dcccccd,0x3dcccccd,
                                                0x3dcccccd,0x3a83126f,1,0x50);
              }
              local_8 = 0xffffffff;
              local_78 = local_28c;
              if (local_28c != (int *)0x0) {
                iVar5 = local_6c + local_70;
                iVar3 = _rand();
                fVar7 = (float)(iVar3 % 10 + -5) * _DAT_005a439c +
                        *(float *)(param_1 + 0x6c + iVar5 * 0xc);
                uVar2 = *(undefined4 *)(param_1 + 0x68 + (local_6c + local_70) * 0xc);
                iVar5 = local_6c + local_70;
                iVar3 = _rand();
                uVar2 = FUN_00430f50((float)(iVar3 % 10 + -5) * _DAT_005a439c +
                                     *(float *)(param_1 + 100 + iVar5 * 0xc),uVar2,fVar7);
                FUN_004310a0(uVar2);
                local_78[0x1b] = 1;
                (**(code **)(*local_78 + 0x3c))(0xffff6666);
                FUN_0054ac09(local_78);
              }
            }
          }
          *(int *)(param_1 + 0x640) = local_34[8];
        }
      }
      else if ((((*(int *)(param_1 + 0x24c) == 0x15) && (*(short *)(param_1 + 0x1d2) == 10)) ||
               ((*(int *)(param_1 + 0x24c) == 0x19 &&
                ((*(short *)(param_1 + 0x1d2) == 3 && (*(short *)(param_1 + 0x1d4) == 8)))))) ||
              ((*(int *)(param_1 + 0x24c) == 0x19 && (*(short *)(param_1 + 0x1d2) == 0xc)))) {
        local_7c = 400;
        if (*(int *)(param_1 + 0x24c) == 0x19) {
          local_7c = 1000;
        }
        if (local_7c < (uint)(local_34[8] - *(int *)(param_1 + 0x644))) {
          local_8c = 3;
          local_84 = 1.0;
          local_80 = 0x77;
          if (*(int *)(param_1 + 0x24c) == 0x19) {
            local_8c = 2;
            local_80 = 0x59;
            local_84 = 1.6;
          }
          for (local_88 = 0; local_88 < local_8c; local_88 = local_88 + 1) {
            local_94 = _rand();
            local_94 = local_94 % 3;
            pvVar1 = operator_new(0x180);
            local_8 = 5;
            if (pvVar1 == (void *)0x0) {
              local_298 = 0;
            }
            else {
              local_298 = FUN_004c8f70(local_80,local_88 * 600 + 0x960,
                                       _DAT_005a4678 * *(float *)(param_1 + 0x38) +
                                       (float)local_94 * _DAT_005a4214,
                                       _DAT_005a3660 * *(float *)(param_1 + 0x38) + _DAT_005a40ec,
                                       _DAT_005a4678 * *(float *)(param_1 + 0x38) +
                                       (float)local_94 * _DAT_005a4214,0x3a03126f,1,0x50);
            }
            local_8 = 0xffffffff;
            local_90 = local_298;
            if (local_298 != 0) {
              uVar2 = FUN_00430f50((float)local_88 * _DAT_005a40ec * (float)local_94 +
                                   *(float *)(param_1 + 0x28),
                                   (local_84 * *(float *)(param_1 + 0x38) +
                                   *(float *)(param_1 + 0x30)) -
                                   (float)local_88 * _DAT_005a40ec * (float)local_94,
                                   (float)local_88 * _DAT_005a40ec * (float)local_94 +
                                   *(float *)(param_1 + 0x2c));
              uVar2 = FUN_004310a0(uVar2);
              FUN_004310a0(uVar2);
              *(undefined4 *)(local_90 + 0x148) = 1;
              *(undefined4 *)(local_90 + 0x140) = 0xc0000000;
              FUN_0054ac09(local_90);
            }
          }
          if (*(int *)(param_1 + 0x24c) == 0x19) {
            local_9c = _rand();
            local_9c = local_9c % 3;
            pvVar1 = operator_new(0xc0);
            local_8 = 6;
            if (pvVar1 == (void *)0x0) {
              local_29c = (int *)0x0;
            }
            else {
              local_29c = (int *)FUN_004df250(2,local_80,0x3f800000);
            }
            local_8 = 0xffffffff;
            local_98 = local_29c;
            if (local_29c != (int *)0x0) {
              local_29c[0x2d] = 0;
              local_29c[0x28] = 3000;
              local_29c[0x26] = (int)(((float)local_9c * _DAT_005a4148) / _DAT_005a3794);
              (**(code **)(*local_29c + 0x44))(0xcccccccc);
              fVar6 = (float10)FUN_00423b00(*(float *)(param_1 + 0x34) - _DAT_005a4148);
              fVar7 = (float)((float10)local_9c * (float10)_DAT_005a4158 +
                             ((float10)*(float *)(param_1 + 0x2c) - fVar6 * (float10)_DAT_005a34a0))
              ;
              fVar6 = (float10)FUN_00493bf0(*(float *)(param_1 + 0x34) - _DAT_005a4148,fVar7);
              FUN_00430f20((float)((float10)local_9c * (float10)_DAT_005a4158 +
                                  fVar6 * (float10)_DAT_005a34a0 +
                                  (float10)*(float *)(param_1 + 0x28)),fVar7);
              (**(code **)(*local_98 + 0x3c))(local_a4,local_a0);
              FUN_0054ac09(local_98);
            }
          }
          *(int *)(param_1 + 0x644) = local_34[8];
        }
      }
      else if (*(int *)(param_1 + 0x24c) == 0x17) {
        local_a8 = 100;
        if (_DAT_005a3670 <= *(float *)(DAT_005ccf9c + 0x2b040)) {
          if (_DAT_005a4314 <= *(float *)(DAT_005ccf9c + 0x2b040)) {
            if (*(float *)(DAT_005ccf9c + 0x2b040) < _DAT_005a40fc) {
              local_a8 = 300;
            }
          }
          else {
            local_a8 = 600;
          }
        }
        else {
          local_a8 = 1000;
        }
        if (local_a8 < (uint)(local_34[8] - *(int *)(param_1 + 0x640))) {
          for (local_ac = 0; local_ac < 2; local_ac = local_ac + 1) {
            for (local_b0 = 0; local_b0 < 1; local_b0 = local_b0 + 1) {
              pvVar1 = operator_new(0x180);
              local_8 = 7;
              if (pvVar1 == (void *)0x0) {
                local_2a0 = (int *)0x0;
              }
              else {
                local_2a0 = (int *)FUN_004c8f70(0,local_ac * 400 + 0x5dc,0x3dcccccd,0x3dcccccd,
                                                0x3dcccccd,0x3a83126f,1,0x50);
              }
              local_8 = 0xffffffff;
              local_b4 = local_2a0;
              if (local_2a0 != (int *)0x0) {
                iVar5 = local_ac * 0xc;
                iVar3 = _rand();
                fVar7 = (float)(iVar3 % 10 + -5) * _DAT_005a439c +
                        *(float *)(param_1 + 0x6c + iVar5);
                uVar2 = *(undefined4 *)(param_1 + 0x68 + local_ac * 0xc);
                iVar5 = local_ac * 0xc;
                iVar3 = _rand();
                uVar2 = FUN_00430f50((float)(iVar3 % 10 + -5) * _DAT_005a439c +
                                     *(float *)(param_1 + 100 + iVar5),uVar2,fVar7);
                FUN_004310a0(uVar2);
                local_b4[0x1b] = 1;
                (**(code **)(*local_b4 + 0x3c))(0xff33ff66);
                FUN_0054ac09(local_b4);
              }
            }
          }
          *(int *)(param_1 + 0x640) = local_34[8];
        }
      }
      else if ((*(int *)(param_1 + 0x24c) == 0x1c) && (*(short *)(param_1 + 0x1d2) == 2)) {
        local_b8 = (**(code **)(*DAT_0092e654 + 8))();
        if (1000 < (uint)(local_b8 - *(int *)(param_1 + 0x640))) {
          FUN_00421910(local_d0,0xc,2,CSimpleArray<>);
          uVar2 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                               *(float *)(param_1 + 0x30) + _DAT_005a3660,
                               *(undefined4 *)(param_1 + 0x2c));
          FUN_004310a0(uVar2);
          FUN_004310a0(param_1 + 100);
          for (local_d4 = 0; (int)local_d4 < 6; local_d4 = local_d4 + 1) {
            local_dc = _rand();
            local_dc = local_dc % 5;
            pvVar1 = operator_new(0x180);
            local_8 = 8;
            if (pvVar1 == (void *)0x0) {
              local_2ac = (int *)0x0;
            }
            else {
              local_2ac = (int *)FUN_004c8f70(0,local_d4 * 400 + 0x5dc,
                                              _DAT_005a4158 * *(float *)(param_1 + 0x38) +
                                              (float)local_dc * _DAT_005a4214,
                                              _DAT_005a4158 * *(float *)(param_1 + 0x38) +
                                              (float)local_dc * _DAT_005a4214,
                                              _DAT_005a4158 * *(float *)(param_1 + 0x38) +
                                              (float)local_dc * _DAT_005a4214,0x3a83126f,1,0x50);
            }
            local_8 = 0xffffffff;
            local_d8 = local_2ac;
            if (local_2ac != (int *)0x0) {
              uVar4 = local_d4 & 0x80000001;
              if ((int)uVar4 < 0) {
                uVar4 = (uVar4 - 1 | 0xfffffffe) + 1;
              }
              FUN_004310a0(local_d0 + uVar4 * 0xc);
              local_d8[0x1b] = 0;
              local_d8[0x50] = -0x40800000;
              (**(code **)(*local_d8 + 0x3c))(0xff00ff00);
              FUN_0054ac09(local_d8);
            }
          }
          *(int *)(param_1 + 0x640) = local_b8;
        }
      }
      else if ((*(int *)(param_1 + 0x24c) == 0x16) || (*(int *)(param_1 + 0x24c) == 0x1b)) {
        local_e4 = _rand();
        local_e4 = local_e4 % 5;
        pvVar1 = operator_new(0x180);
        local_8 = 9;
        if (pvVar1 == (void *)0x0) {
          local_2b0 = (int *)0x0;
        }
        else {
          local_2b0 = (int *)FUN_004c8f70(0,0x5dc,(float)local_e4 * _DAT_005a45d4 + _DAT_005a45d4,
                                          (float)local_e4 * _DAT_005a4ae0 + _DAT_005a45d4,
                                          (float)local_e4 * _DAT_005a45d4 + _DAT_005a45d4,0x38d1b717
                                          ,1,0x50);
        }
        local_8 = 0xffffffff;
        local_e0 = local_2b0;
        if (local_2b0 != (int *)0x0) {
          iVar3 = _rand();
          fVar7 = (float)(iVar3 % 10 + -5) * _DAT_005a41e8 + *(float *)(param_1 + 0x6c);
          uVar2 = *(undefined4 *)(param_1 + 0x68);
          iVar3 = _rand();
          uVar2 = FUN_00430f50((float)(iVar3 % 10 + -5) * _DAT_005a41e8 + *(float *)(param_1 + 100),
                               uVar2,fVar7);
          uVar2 = FUN_004310a0(uVar2);
          FUN_004310a0(uVar2);
          local_e0[0x1b] = 1;
          local_e0[0x40] = 0;
          local_e0[0x52] = 1;
          local_e0[0x50] = -0x40800000;
          (**(code **)(*local_e0 + 0x3c))(0xffffffcc);
          if (*(int *)(param_1 + 0x24c) == 0x1b) {
            (**(code **)(*local_e0 + 0x3c))(0xffffdd88);
          }
          FUN_0054ac09(local_e0);
        }
      }
      else if ((*(int *)(param_1 + 0x24c) == 0x12) && (*(int *)(param_1 + 0x740) == 5)) {
        for (local_e8 = 0; local_e8 < 2; local_e8 = local_e8 + 1) {
          for (local_ec = 0; local_ec < 1; local_ec = local_ec + 1) {
            pvVar1 = operator_new(0x180);
            local_8 = 10;
            if (pvVar1 == (void *)0x0) {
              local_2bc = (int *)0x0;
            }
            else {
              local_2bc = (int *)FUN_004c8f70(0,local_e8 * 400 + 0x5dc,0x3dcccccd,0x3dcccccd,
                                              0x3dcccccd,0x3a83126f,1,0x50);
            }
            local_8 = 0xffffffff;
            local_f0 = local_2bc;
            if (local_2bc != (int *)0x0) {
              iVar5 = local_e8 * 0xc;
              iVar3 = _rand();
              fVar7 = (float)(iVar3 % 10 + -5) * _DAT_005a439c + *(float *)(param_1 + 0x6c + iVar5);
              uVar2 = *(undefined4 *)(param_1 + 0x68 + local_e8 * 0xc);
              iVar5 = local_e8 * 0xc;
              iVar3 = _rand();
              uVar2 = FUN_00430f50((float)(iVar3 % 10 + -5) * _DAT_005a439c +
                                   *(float *)(param_1 + 100 + iVar5),uVar2,fVar7);
              FUN_004310a0(uVar2);
              local_f0[0x1b] = 1;
              (**(code **)(*local_f0 + 0x3c))(0xffff6666);
              FUN_0054ac09(local_f0);
            }
          }
        }
      }
      else if (((*(int *)(param_1 + 0x24c) == 0x19) &&
               ((*(short *)(param_1 + 0x1d2) == 3 && (_DAT_005a55f8 < *(float *)(param_1 + 0x38)))))
              || (*(int *)(param_1 + 0x24c) == 0x1e)) {
        local_f4 = 0xffff6666;
        if (*(int *)(param_1 + 0x24c) == 0x1e) {
          local_f4 = 0xff66ff66;
        }
        if (100 < (uint)(local_34[8] - *(int *)(param_1 + 0x640))) {
          for (local_f8 = 0; local_f8 < 2; local_f8 = local_f8 + 1) {
            for (local_fc = 0; local_fc < 1; local_fc = local_fc + 1) {
              pvVar1 = operator_new(0x180);
              local_8 = 0xb;
              if (pvVar1 == (void *)0x0) {
                local_2c8 = (int *)0x0;
              }
              else {
                local_2c8 = (int *)FUN_004c8f70(0,local_f8 * 400 + 0x5dc,0x3dcccccd,0x3dcccccd,
                                                0x3dcccccd,0x3a83126f,1,0x50);
              }
              local_8 = 0xffffffff;
              local_100 = local_2c8;
              if (local_2c8 != (int *)0x0) {
                iVar5 = local_f8 + 1;
                iVar3 = _rand();
                fVar7 = (float)(iVar3 % 10 + -5) * _DAT_005a439c +
                        *(float *)(param_1 + 0x6c + iVar5 * 0xc);
                uVar2 = *(undefined4 *)(param_1 + 0x68 + (local_f8 + 1) * 0xc);
                iVar5 = local_f8 + 1;
                iVar3 = _rand();
                uVar2 = FUN_00430f50((float)(iVar3 % 10 + -5) * _DAT_005a439c +
                                     *(float *)(param_1 + 100 + iVar5 * 0xc),uVar2,fVar7);
                FUN_004310a0(uVar2);
                local_100[0x1b] = 1;
                (**(code **)(*local_100 + 0x3c))(local_f4);
                FUN_0054ac09(local_100);
              }
            }
          }
          *(int *)(param_1 + 0x640) = local_34[8];
        }
      }
      else if ((*(int *)(param_1 + 0x24c) == 0x15) && (*(short *)(param_1 + 0x1d2) == 4)) {
        if (100 < (uint)(local_34[8] - *(int *)(param_1 + 0x640))) {
          for (local_104 = 0; local_104 < 2; local_104 = local_104 + 1) {
            for (local_108 = 0; local_108 < 2; local_108 = local_108 + 1) {
              pvVar1 = operator_new(0x180);
              local_8 = 0xc;
              if (pvVar1 == (void *)0x0) {
                local_2d4 = (int *)0x0;
              }
              else {
                local_2d4 = (int *)FUN_004c8f70(0,local_104 * 400 + 0x5dc,0x3dcccccd,0x3dcccccd,
                                                0x3dcccccd,0x3a83126f,1,0x50);
              }
              local_8 = 0xffffffff;
              local_10c = local_2d4;
              if (local_2d4 != (int *)0x0) {
                iVar5 = local_108 + 1;
                iVar3 = _rand();
                fVar7 = (float)(iVar3 % 10 + -5) * _DAT_005a439c +
                        *(float *)(param_1 + 0x6c + iVar5 * 0xc);
                uVar2 = *(undefined4 *)(param_1 + 0x68 + (local_108 + 1) * 0xc);
                iVar5 = local_108 + 1;
                iVar3 = _rand();
                uVar2 = FUN_00430f50((float)(iVar3 % 10 + -5) * _DAT_005a439c +
                                     *(float *)(param_1 + 100 + iVar5 * 0xc),uVar2,fVar7);
                FUN_004310a0(uVar2);
                local_10c[0x1b] = 1;
                (**(code **)(*local_10c + 0x3c))(0xffffaa66);
                FUN_0054ac09(local_10c);
              }
            }
          }
          *(int *)(param_1 + 0x640) = local_34[8];
        }
      }
      else if ((*(int *)(param_1 + 0x24c) == 0x1d) && (*(short *)(param_1 + 0x1d2) == 1)) {
        for (local_110 = 0; local_110 < 1; local_110 = local_110 + 1) {
          if (*(int *)(param_1 + 0x56c + local_110 * 4) != 0) {
            FUN_004310a0(param_1 + 100 + local_110 * 0xc);
            (**(code **)(**(int **)(param_1 + 0x56c + local_110 * 4) + 0x20))(0);
          }
        }
      }
      else if ((*(int *)(param_1 + 0x24c) == 0x26) && ('\0' < *(char *)(param_1 + 0x79b))) {
        for (local_114 = 1; local_114 < 7; local_114 = local_114 + 1) {
          if (*(int *)(param_1 + 0x56c + local_114 * 4) != 0) {
            FUN_004310a0(param_1 + 100 + local_114 * 0xc);
            (**(code **)(**(int **)(param_1 + 0x56c + local_114 * 4) + 0x20))(0);
          }
        }
      }
      else if ((*(int *)(param_1 + 0x24c) == 0x21) &&
              ((*(short *)(param_1 + 0x1d2) == 1 && (DAT_005ccfa0 == 2)))) {
        for (local_118 = 0; local_118 < 7; local_118 = local_118 + 1) {
          if (*(int *)(param_1 + 0x56c + local_118 * 4) != 0) {
            (**(code **)(**(int **)(param_1 + 0x56c + local_118 * 4) + 0x3c))(0xffff5500);
            FUN_004310a0(param_1 + 100 + local_118 * 0xc);
            *(float *)(*(int *)(param_1 + 0x56c + local_118 * 4) + 0x78) =
                 _DAT_005a40ec * *(float *)(param_1 + 0x38) +
                 *(float *)(*(int *)(param_1 + 0x56c + local_118 * 4) + 0x78);
            (**(code **)(**(int **)(param_1 + 0x56c + local_118 * 4) + 0x20))(0);
          }
        }
      }
      else if ((*(int *)(param_1 + 0x24c) == 0x10) && (*(short *)(param_1 + 0x1d2) == 7)) {
        if (100 < (uint)(local_34[8] - *(int *)(param_1 + 0x640))) {
          for (local_11c = 0; local_11c < 2; local_11c = local_11c + 1) {
            for (local_120 = 0; local_120 < 1; local_120 = local_120 + 1) {
              pvVar1 = operator_new(0x180);
              local_8 = 0xd;
              if (pvVar1 == (void *)0x0) {
                local_2e0 = (int *)0x0;
              }
              else {
                local_2e0 = (int *)FUN_004c8f70(0,local_11c * 400 + 0x5dc,0x3dcccccd,0x3dcccccd,
                                                0x3dcccccd,0x3a83126f,1,0x50);
              }
              local_8 = 0xffffffff;
              local_124 = local_2e0;
              if (local_2e0 != (int *)0x0) {
                iVar3 = _rand();
                fVar7 = (float)(iVar3 % 10 + -5) * _DAT_005a439c + *(float *)(param_1 + 0x2c);
                fVar8 = *(float *)(param_1 + 0x30) + _DAT_005a4158;
                iVar3 = _rand();
                uVar2 = FUN_00430f50((float)(iVar3 % 10 + -5) * _DAT_005a439c +
                                     *(float *)(param_1 + 0x28),fVar8,fVar7);
                FUN_004310a0(uVar2);
                local_124[0x1b] = 1;
                (**(code **)(*local_124 + 0x3c))(0xffff8800);
                FUN_0054ac09(local_124);
              }
            }
          }
          *(int *)(param_1 + 0x640) = local_34[8];
        }
      }
      else if (*(int *)(param_1 + 0x24c) == 0x27) {
        local_138[0] = 6;
        local_138[1] = 7;
        local_138[2] = 2;
        local_138[3] = 3;
        for (local_138[4] = 0; local_138[4] < 4; local_138[4] = local_138[4] + 1) {
          if (*(int *)(param_1 + 0x56c + local_138[4] * 4) != 0) {
            FUN_004310a0(param_1 + 100 + local_138[local_138[4]] * 0xc);
            (**(code **)(**(int **)(param_1 + 0x56c + local_138[4] * 4) + 0x20))(0);
          }
        }
      }
      else if ((*(int *)(param_1 + 0x24c) == 0x38) && (*(short *)(param_1 + 0x1d2) == 0)) {
        if ((*(int *)(param_1 + 0x524) != 0) && (*(char *)(param_1 + 0x788) == '\0')) {
          *(undefined1 *)(param_1 + 0x788) = 1;
          *(undefined4 *)(*(int *)(param_1 + 0x524) + 0x98) = 3000;
          iVar3 = _rand();
          *(float *)(*(int *)(param_1 + 0x524) + 0xac) = (float)-(iVar3 % 0x32) / _DAT_005a40f8;
          (**(code **)(**(int **)(param_1 + 0x524) + 0x3c))(0xffffffff);
        }
        iVar3 = _rand();
        iVar3 = iVar3 % 5;
        pvVar1 = operator_new(0x180);
        local_8 = 0xe;
        if (pvVar1 == (void *)0x0) {
          local_2f0 = (int *)0x0;
        }
        else {
          local_2f0 = (int *)FUN_004c8f70(0,0x5dc,(float)iVar3 * _DAT_005a4154 + _DAT_005a4158,
                                          (float)iVar3 * _DAT_005a4160 + _DAT_005a40f4,
                                          (float)iVar3 * _DAT_005a34a0 + _DAT_005a34a0,0x38d1b717,1,
                                          0x50);
        }
        local_8 = 0xffffffff;
        if (local_2f0 != (int *)0x0) {
          iVar3 = _rand();
          fVar7 = (float)(iVar3 % 10 + -5) * _DAT_005a41e8 + *(float *)(param_1 + 0x6c);
          uVar2 = *(undefined4 *)(param_1 + 0x68);
          iVar3 = _rand();
          uVar2 = FUN_00430f50((float)(iVar3 % 0x28 + -5) * _DAT_005a41e8 +
                               *(float *)(param_1 + 100),uVar2,fVar7);
          uVar2 = FUN_004310a0(uVar2);
          FUN_004310a0(uVar2);
          local_2f0[0x1b] = 1;
          local_2f0[0x40] = 0;
          local_2f0[0x52] = 1;
          local_2f0[0x50] = -0x3fc00000;
          (**(code **)(*local_2f0 + 0x3c))(0xffff7777);
          FUN_0054ac09(local_2f0);
        }
      }
    }
    else {
      local_34[0] = 8;
      local_34[1] = 9;
      local_34[2] = 1;
      local_34[3] = 6;
      local_34[4] = 7;
      local_34[5] = 2;
      local_34[6] = 3;
      for (local_34[7] = 0; local_34[7] < 7; local_34[7] = local_34[7] + 1) {
        if (*(int *)(param_1 + 0x56c + local_34[7] * 4) != 0) {
          FUN_004310a0(param_1 + 100 + local_34[local_34[7]] * 0xc);
          if ((2 < local_34[7]) && (local_34[7] < 5)) {
            *(float *)(*(int *)(param_1 + 0x56c + local_34[7] * 4) + 0x78) =
                 _DAT_005a4214 * *(float *)(param_1 + 0x38) +
                 *(float *)(*(int *)(param_1 + 0x56c + local_34[7] * 4) + 0x78);
          }
          if (4 < local_34[7]) {
            *(float *)(*(int *)(param_1 + 0x56c + local_34[7] * 4) + 0x78) =
                 _DAT_005a40ec * *(float *)(param_1 + 0x38) +
                 *(float *)(*(int *)(param_1 + 0x56c + local_34[7] * 4) + 0x78);
          }
          (**(code **)(**(int **)(param_1 + 0x56c + local_34[7] * 4) + 0x20))(0);
        }
      }
    }
    if (*(int *)(param_1 + 0x5c) == 0x26) {
      for (local_144 = 0; local_144 < 7; local_144 = local_144 + 1) {
        if (*(int *)(param_1 + 0x56c + local_144 * 4) != 0) {
          FUN_004310a0(param_1 + 100 + local_144 * 0xc);
          if ((0 < local_144) && (local_144 < 5)) {
            *(float *)(*(int *)(param_1 + 0x56c + local_144 * 4) + 0x78) =
                 _DAT_005a34a0 * *(float *)(param_1 + 0x38) +
                 *(float *)(*(int *)(param_1 + 0x56c + local_144 * 4) + 0x78);
          }
          (**(code **)(**(int **)(param_1 + 0x56c + local_144 * 4) + 0x20))(0);
        }
      }
    }
    if (*(int *)(param_1 + 0x7a0) == 0x26) {
      for (local_148 = 0; local_148 < 7; local_148 = local_148 + 1) {
        if (*(int *)(param_1 + 0x588 + local_148 * 4) != 0) {
          FUN_004310a0(param_1 + 0xdc + local_148 * 0xc);
          if ((0 < local_148) && (local_148 < 5)) {
            *(float *)(*(int *)(param_1 + 0x588 + local_148 * 4) + 0x78) =
                 _DAT_005a34a0 * *(float *)(param_1 + 0x38) +
                 *(float *)(*(int *)(param_1 + 0x588 + local_148 * 4) + 0x78);
          }
          (**(code **)(**(int **)(param_1 + 0x588 + local_148 * 4) + 0x20))(0);
        }
      }
    }
    if (*(char *)(param_1 + 0x790) != '\0') {
      if ((*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) &&
         (*(int *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x88) != 0)) {
        FUN_004310a0(param_1 + 0xac);
        (**(code **)(**(int **)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x88) + 0x20))(0);
      }
      if ((*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0) &&
         (*(int *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x88) != 0)) {
        FUN_004310a0(param_1 + 0xb8);
        (**(code **)(**(int **)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x88) + 0x20))(0);
      }
    }
  }
  ExceptionList = local_10;
  return;
}

