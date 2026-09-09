// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00541065 | Name: FUN_00541065


/* WARNING: Removing unreachable block (ram,0x00544f92) */
/* WARNING: Removing unreachable block (ram,0x00544fe1) */
/* WARNING: Removing unreachable block (ram,0x00544fb5) */
/* WARNING: Removing unreachable block (ram,0x00544feb) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_00541065(int param_1,undefined4 param_2)

{
  bool bVar1;
  undefined4 uVar2;
  void *pvVar3;
  uint uVar4;
  uint uVar5;
  undefined4 *puVar6;
  uint uVar7;
  uint uVar8;
  int iVar9;
  float10 fVar10;
  float fVar11;
  float fVar12;
  float fVar13;
  undefined4 local_624;
  int *local_60c;
  int *local_600;
  int *local_5f4;
  int local_5f0;
  int local_5ec;
  int local_5e8;
  int local_5e4;
  undefined4 local_5dc;
  undefined4 local_5d8;
  int local_5d4;
  undefined4 local_5d0;
  undefined4 local_5cc;
  int local_5c8;
  undefined4 local_5c4;
  undefined4 local_5c0;
  undefined4 local_5bc;
  int local_5b8;
  int local_5b4;
  undefined4 local_5b0;
  undefined4 local_5ac;
  int *local_5a8;
  int *local_5a4;
  int *local_59c;
  int local_598;
  int *local_590;
  int *local_58c;
  int *local_588;
  int *local_584;
  int *local_580;
  int *local_57c;
  int *local_578;
  int *local_574;
  int *local_570;
  int *local_56c;
  undefined4 local_568;
  undefined4 local_564;
  undefined4 local_560;
  undefined4 local_55c;
  undefined4 local_558;
  int *local_550;
  int local_538;
  int local_520;
  int local_504;
  int local_4e8;
  int local_4cc;
  int local_4b0;
  undefined4 local_4a8;
  int local_1f0;
  undefined4 local_1e4;
  undefined4 local_1e0;
  undefined4 local_1cc;
  undefined4 local_1c8;
  undefined4 local_1bc;
  undefined4 local_1b0;
  undefined4 local_1ac;
  undefined4 local_18c;
  undefined4 local_188;
  undefined4 local_174;
  undefined1 local_144 [16];
  uint local_134;
  undefined *local_130 [5];
  undefined4 local_11c;
  int *local_118;
  int *local_114;
  int *local_110;
  int *local_10c;
  int *local_108;
  undefined1 local_104;
  int *local_100;
  int aiStack_fc [5];
  int local_e8;
  int aiStack_e4 [5];
  int local_d0;
  int aiStack_cc [5];
  int local_b8;
  int aiStack_b4 [5];
  int local_a0;
  int aiStack_9c [5];
  int local_88;
  int aiStack_84 [5];
  int local_70;
  int local_6c;
  int local_68;
  int local_64;
  int local_60;
  float local_5c;
  float local_58;
  float local_54;
  int local_50;
  float local_4c;
  size_t local_48;
  uint local_44;
  uint local_40;
  int local_3c;
  int local_38;
  void *local_34;
  uint *local_30;
  int local_2c;
  float local_28;
  uint local_24;
  int local_20;
  float local_1c;
  float local_18;
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a29b5;
  local_10 = ExceptionList;
  local_44 = 0xffffffff;
  ExceptionList = &local_10;
  FUN_00430f10();
  local_58 = 0.0;
  local_4c = 0.0;
  local_34 = (void *)0x0;
  local_30 = (uint *)0x0;
  local_50 = 0;
  if (*(int *)(param_1 + 0x24) != 0) {
    local_58 = (float)(*(int *)(*(int *)(param_1 + 0x24) + 0x20a20) << 6);
    local_58 = local_58 + local_58;
    local_4c = (float)(*(int *)(*(int *)(param_1 + 0x24) + 0x20a24) << 6);
    local_4c = local_4c + local_4c;
  }
  local_20 = FUN_0058fb07(param_2,0x8000,0);
  if (local_20 == -1) {
    uVar2 = 0;
  }
  else {
    local_48 = FUN_0058f83f(local_20);
    local_34 = _malloc(local_48);
    if (local_34 == (void *)0x0) {
      MessageBoxA((HWND)0x0,s_Not_Enought_Memory_005bdf8c,s_Memory_allocation_fail_005bdf74,0x1000);
      uVar2 = 0;
    }
    else {
      FUN_0058f8c9(local_20,local_34,local_48);
      local_2c = 0;
      local_38 = 0x1c;
      local_3c = 8;
      *(undefined4 *)(param_1 + 0x48) = 0;
      while ((*(int *)(param_1 + 0x48) < 0x1000 && (local_50 < (int)local_48))) {
        local_30 = (uint *)((int)local_34 + local_50);
        local_44 = *local_30;
        FUN_0040bd30(local_30 + 1);
        local_28 = (float)local_30[3];
        local_5c = (float)local_30[4];
        local_24 = local_30[5];
        local_40 = local_30[6];
        local_14 = (float)local_30[7];
        local_54 = (float)local_30[8];
        local_50 = local_50 + local_38;
        local_64 = __ftol();
        local_68 = __ftol();
        local_60 = (local_64 >> 5) + (local_68 >> 5) * 0x10000;
        local_2c = local_2c + local_44;
        iVar9 = __ftol();
        local_2c = local_2c + iVar9;
        iVar9 = __ftol();
        local_2c = local_2c + iVar9;
        iVar9 = __ftol();
        local_2c = local_2c + iVar9 + local_40;
        if (local_44 == 2) {
          local_6c = *(int *)(param_1 + 0x24);
          if (local_6c != 0) {
            pvVar3 = operator_new(0x1d0);
            local_8 = 0;
            if (pvVar3 == (void *)0x0) {
              local_4a8 = 0;
            }
            else {
              local_4a8 = FUN_00545730();
            }
            local_8 = 0xffffffff;
            *(undefined4 *)(local_6c + 0x20a34 + *(int *)(local_6c + 0x20a5c) * 4) = local_4a8;
            if (*(int *)(local_6c + 0x20a34 + *(int *)(local_6c + 0x20a5c) * 4) != 0) {
              if (((*(int *)(DAT_0067cf38 + 0x24) == 30000) && (iVar9 = __ftol(), 0x1a < iVar9 >> 7)
                  ) && ((iVar9 = __ftol(), iVar9 >> 7 < 0x1f &&
                        ((iVar9 = __ftol(), 0x14 < iVar9 >> 7 &&
                         (iVar9 = __ftol(), iVar9 >> 7 < 0x19)))))) {
                bVar1 = true;
              }
              else {
                bVar1 = false;
              }
              if (bVar1) {
                *(undefined4 *)
                 (*(int *)(local_6c + 0x20a34 + *(int *)(local_6c + 0x20a5c) * 4) + 0x1cc) = 0;
              }
              *(uint *)(*(int *)(local_6c + 0x20a34 + *(int *)(local_6c + 0x20a5c) * 4) + 0x3c) =
                   local_24;
              *(uint *)(*(int *)(local_6c + 0x20a34 + *(int *)(local_6c + 0x20a5c) * 4) + 0x24) =
                   local_44;
              *(uint *)(*(int *)(local_6c + 0x20a34 + *(int *)(local_6c + 0x20a5c) * 4) + 0x40) =
                   local_40;
              (**(code **)(**(int **)(local_6c + 0x20a34 + *(int *)(local_6c + 0x20a5c) * 4) + 0x38)
              )();
              (**(code **)(**(int **)(local_6c + 0x20a34 + *(int *)(local_6c + 0x20a5c) * 4) + 0x40)
              )(0,local_5c,0);
              (**(code **)(**(int **)(local_6c + 0x20a34 + *(int *)(local_6c + 0x20a5c) * 4) + 0x3c)
              )(local_58 + local_1c,local_28,local_4c + local_18);
              FUN_0054ac09(*(undefined4 *)(local_6c + 0x20a34 + *(int *)(local_6c + 0x20a5c) * 4));
            }
            *(int *)(local_6c + 0x20a5c) = *(int *)(local_6c + 0x20a5c) + 1;
          }
        }
        else if (local_44 == 0x157) {
          for (local_70 = 0; local_70 < 5; local_70 = local_70 + 1) {
            pvVar3 = operator_new(0x1b4);
            local_8 = 1;
            if (pvVar3 == (void *)0x0) {
              local_4b0 = 0;
            }
            else {
              local_4b0 = FUN_0052ff60(local_44,0,0);
            }
            local_8 = 0xffffffff;
            aiStack_84[local_70] = local_4b0;
            if (aiStack_84[local_70] != 0) {
              (**(code **)(*(int *)aiStack_84[local_70] + 0x38))();
              uVar2 = 0;
              uVar4 = _rand();
              uVar4 = uVar4 & 0x80000003;
              if ((int)uVar4 < 0) {
                uVar4 = (uVar4 - 1 | 0xfffffffc) + 1;
              }
              (**(code **)(*(int *)aiStack_84[local_70] + 0x40))
                        (0,((float)(int)uVar4 * _DAT_005a4148) / _DAT_005a3e9c,uVar2);
              fVar12 = local_4c + local_18;
              iVar9 = _rand();
              fVar12 = (float)(iVar9 % 5) * _DAT_005a4214 + fVar12;
              iVar9 = _rand();
              fVar11 = (float)(iVar9 % 10) * _DAT_005a4158 + local_28;
              fVar13 = local_58 + local_1c;
              iVar9 = _rand();
              (**(code **)(*(int *)aiStack_84[local_70] + 0x3c))
                        ((float)(iVar9 % 5) * _DAT_005a4214 + fVar13,fVar11,fVar12);
              FUN_0054adbc(aiStack_84[local_70],local_60);
            }
          }
        }
        else if (local_44 == 4) {
          for (local_88 = 0; local_88 < 5; local_88 = local_88 + 1) {
            pvVar3 = operator_new(0x1b4);
            local_8 = 2;
            if (pvVar3 == (void *)0x0) {
              local_4cc = 0;
            }
            else {
              local_4cc = FUN_0052ff60(local_44,1,0);
            }
            local_8 = 0xffffffff;
            aiStack_9c[local_88] = local_4cc;
            if (aiStack_9c[local_88] != 0) {
              (**(code **)(*(int *)aiStack_9c[local_88] + 0x38))();
              uVar2 = 0;
              uVar4 = _rand();
              uVar4 = uVar4 & 0x80000003;
              if ((int)uVar4 < 0) {
                uVar4 = (uVar4 - 1 | 0xfffffffc) + 1;
              }
              (**(code **)(*(int *)aiStack_9c[local_88] + 0x40))
                        (0,((float)(int)uVar4 * _DAT_005a4148) / _DAT_005a3e9c,uVar2);
              fVar12 = local_4c + local_18;
              iVar9 = _rand();
              fVar12 = (float)(iVar9 % 5) * _DAT_005a4214 + fVar12;
              iVar9 = _rand();
              fVar11 = (float)(iVar9 % 10) * _DAT_005a4158 + local_28;
              fVar13 = local_58 + local_1c;
              iVar9 = _rand();
              (**(code **)(*(int *)aiStack_9c[local_88] + 0x3c))
                        ((float)(iVar9 % 5) * _DAT_005a4214 + fVar13,fVar11,fVar12);
              FUN_0054adbc(aiStack_9c[local_88],local_60);
            }
          }
        }
        else if (local_44 == 6) {
          for (local_a0 = 0; local_a0 < 5; local_a0 = local_a0 + 1) {
            pvVar3 = operator_new(0x1b4);
            local_8 = 3;
            if (pvVar3 == (void *)0x0) {
              local_4e8 = 0;
            }
            else {
              local_4e8 = FUN_0052ff60(local_44,3,0);
            }
            local_8 = 0xffffffff;
            aiStack_b4[local_a0] = local_4e8;
            if (aiStack_b4[local_a0] != 0) {
              (**(code **)(*(int *)aiStack_b4[local_a0] + 0x38))();
              uVar2 = 0;
              uVar4 = _rand();
              uVar4 = uVar4 & 0x80000003;
              if ((int)uVar4 < 0) {
                uVar4 = (uVar4 - 1 | 0xfffffffc) + 1;
              }
              (**(code **)(*(int *)aiStack_b4[local_a0] + 0x40))
                        (0,((float)(int)uVar4 * _DAT_005a4148) / _DAT_005a3e9c,uVar2);
              fVar12 = local_4c + local_18;
              iVar9 = _rand();
              fVar12 = (float)(iVar9 % 5) * _DAT_005a4214 + fVar12;
              iVar9 = _rand();
              fVar11 = (float)(iVar9 % 10) * _DAT_005a4158 + local_28;
              fVar13 = local_58 + local_1c;
              iVar9 = _rand();
              (**(code **)(*(int *)aiStack_b4[local_a0] + 0x3c))
                        ((float)(iVar9 % 5) * _DAT_005a4214 + fVar13,fVar11,fVar12);
              *(float *)(aiStack_b4[local_a0] + 0x1a0) =
                   *(float *)(aiStack_b4[local_a0] + 0x1a0) * _DAT_005a34a0;
              *(float *)(aiStack_b4[local_a0] + 0x19c) =
                   *(float *)(aiStack_b4[local_a0] + 0x19c) * _DAT_005a34a0;
              *(float *)(aiStack_b4[local_a0] + 0x198) = (float)local_a0 + _DAT_005a366c;
              FUN_0054adbc(aiStack_b4[local_a0],local_60);
            }
          }
        }
        else if (local_44 == 7) {
          for (local_b8 = 0; local_b8 < 5; local_b8 = local_b8 + 1) {
            pvVar3 = operator_new(0x1b4);
            local_8 = 4;
            if (pvVar3 == (void *)0x0) {
              local_504 = 0;
            }
            else {
              local_504 = FUN_0052ff60(local_44,2,0);
            }
            local_8 = 0xffffffff;
            aiStack_cc[local_b8] = local_504;
            if (aiStack_cc[local_b8] != 0) {
              (**(code **)(*(int *)aiStack_cc[local_b8] + 0x38))();
              (**(code **)(*(int *)aiStack_cc[local_b8] + 0x40))(0,0xbfc90fdb,0);
              fVar12 = local_4c + local_18;
              iVar9 = _rand();
              fVar12 = (float)(iVar9 % 5) * _DAT_005a4214 + fVar12;
              fVar11 = local_28 + _DAT_005a3794;
              iVar9 = _rand();
              fVar11 = (float)(iVar9 % 10) * _DAT_005a4158 + fVar11;
              fVar13 = local_58 + local_1c;
              iVar9 = _rand();
              (**(code **)(*(int *)aiStack_cc[local_b8] + 0x3c))
                        ((float)(iVar9 % 5) * _DAT_005a4214 + fVar13,fVar11,fVar12);
              *(int *)(aiStack_cc[local_b8] + 0x1b0) = local_b8 * 200;
              *(undefined4 *)(aiStack_cc[local_b8] + 0x1a0) = 0x40a00000;
              *(undefined4 *)(aiStack_cc[local_b8] + 0x19c) = 0x40a00000;
              *(float *)(aiStack_cc[local_b8] + 0x198) = (float)local_b8 + _DAT_005a366c;
              FUN_0054adbc(aiStack_cc[local_b8],local_60);
            }
          }
        }
        else if (local_44 == 0x158) {
          for (local_d0 = 0; local_d0 < 5; local_d0 = local_d0 + 1) {
            pvVar3 = operator_new(0x1b0);
            local_8 = 5;
            if (pvVar3 == (void *)0x0) {
              local_520 = 0;
            }
            else {
              local_520 = FUN_00530fa0(local_44,0);
            }
            local_8 = 0xffffffff;
            aiStack_e4[local_d0] = local_520;
            if (aiStack_e4[local_d0] != 0) {
              (**(code **)(*(int *)aiStack_e4[local_d0] + 0x38))();
              fVar12 = local_4c + local_18;
              iVar9 = _rand();
              fVar12 = (float)(iVar9 % 5) * _DAT_005a439c + fVar12;
              iVar9 = _rand();
              fVar11 = (float)(iVar9 % 10) * _DAT_005a41e8 + local_28;
              fVar13 = local_58 + local_1c;
              iVar9 = _rand();
              (**(code **)(*(int *)aiStack_e4[local_d0] + 0x3c))
                        ((float)(iVar9 % 5) * _DAT_005a439c + fVar13,fVar11,fVar12);
              FUN_0054adbc(aiStack_e4[local_d0],local_60);
            }
          }
        }
        else if (local_44 == 0xc) {
          for (local_e8 = 0; local_e8 < 5; local_e8 = local_e8 + 1) {
            pvVar3 = operator_new(0x1b0);
            local_8 = 6;
            if (pvVar3 == (void *)0x0) {
              local_538 = 0;
            }
            else {
              local_538 = FUN_00530fa0(local_44,3);
            }
            local_8 = 0xffffffff;
            aiStack_fc[local_e8] = local_538;
            if (aiStack_fc[local_e8] != 0) {
              (**(code **)(*(int *)aiStack_fc[local_e8] + 0x38))();
              fVar12 = local_4c + local_18;
              iVar9 = _rand();
              fVar12 = (float)(iVar9 % 5) * _DAT_005a439c + fVar12;
              iVar9 = _rand();
              fVar11 = (float)(iVar9 % 10) * _DAT_005a41e8 + local_28;
              fVar13 = local_58 + local_1c;
              iVar9 = _rand();
              (**(code **)(*(int *)aiStack_fc[local_e8] + 0x3c))
                        ((float)(iVar9 % 5) * _DAT_005a439c + fVar13,fVar11,fVar12);
              FUN_0054adbc(aiStack_fc[local_e8],local_60);
            }
          }
        }
        else if ((local_44 < 0x137) || (0x142 < local_44)) {
          if (((local_44 < 0x14b) || (0x156 < local_44)) &&
             ((local_44 < 0x15f || (0x17a < local_44)))) {
            if ((local_44 < 0x1e7) || (0x1e9 < local_44)) {
              if (local_44 == 3) {
                pvVar3 = operator_new(0x19c);
                local_8 = 10;
                if (pvVar3 == (void *)0x0) {
                  local_560 = 0;
                }
                else {
                  local_560 = FUN_00531970(local_44);
                }
                *(undefined4 *)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4) = local_560;
              }
              else if (local_44 == 5) {
                pvVar3 = operator_new(0x19c);
                local_8 = 0xb;
                if (pvVar3 == (void *)0x0) {
                  local_564 = 0;
                }
                else {
                  local_564 = FUN_00531970(local_44);
                }
                *(undefined4 *)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4) = local_564;
              }
              else {
                if (((((local_44 < 0xfb) || (0xfe < local_44)) && (local_44 != 0x1da)) &&
                    ((local_44 != 0x111 && (local_44 != 0x112)))) &&
                   ((((local_44 != 0x124 && ((local_44 != 0x25f && (local_44 != 0x262)))) &&
                     ((local_44 != 0x266 &&
                      ((((((local_44 != 0xc3 && (local_44 != 0x2b9)) && (local_44 != 699)) &&
                         (((local_44 != 0x1ea && (local_44 != 0x5f0)) &&
                          ((local_44 != 0x5ff && ((local_44 != 0x5f6 && (local_44 != 0x681))))))))
                        && (local_44 != 0x7c9)) &&
                       (((local_44 != 0x7d5 && (local_44 != 0x69f)) && (local_44 != 0x6a0)))))))) &&
                    (((local_44 != 0x6d6 && (local_44 != 0x6cb)) &&
                     ((local_44 != 0x6af && (local_44 != 0x73f)))))))) {
                  if (local_44 != 0x79) {
                    if ((local_44 < 0x1f5) || (0x1f9 < local_44)) {
                      if (local_44 == 0x1fa) {
                        local_50 = local_50 + local_3c;
                        local_188 = 0x44554444;
                        if (*(int *)(DAT_0067cf38 + 0x24) == 0x7531) {
                          local_188 = 0xff887777;
                        }
                        else {
                          if ((((*(int *)(DAT_0067cf38 + 0x24) == 30000) &&
                               (iVar9 = __ftol(), 0x10 < iVar9 >> 7)) &&
                              (iVar9 = __ftol(), iVar9 >> 7 < 0x14)) &&
                             (iVar9 = __ftol(), 0x1d < iVar9 >> 7)) {
                            bVar1 = true;
                          }
                          else {
                            bVar1 = false;
                          }
                          if (bVar1) {
                            local_188 = 0xff222288;
                          }
                        }
                        pvVar3 = operator_new(0xcc);
                        local_8 = 0x17;
                        if (pvVar3 == (void *)0x0) {
                          local_598 = 0;
                        }
                        else {
                          local_598 = FUN_004d2230(local_44,local_188,local_5c - _DAT_005a4380,0);
                        }
                        local_8 = 0xffffffff;
                        if (local_598 != 0) {
                          *(undefined4 *)(local_598 + 0x6c) = 1;
                          *(undefined1 *)(local_598 + 0xac) = 1;
                          *(undefined4 *)(local_598 + 0xb8) = 5000;
                          if (*(int *)(DAT_0067cf38 + 0x24) == 0x7531) {
                            *(undefined4 *)(local_598 + 0xb8) = 10000;
                          }
                          *(float *)(local_598 + 0x98) = local_14;
                          *(float *)(local_598 + 0x9c) = local_54;
                          uVar2 = FUN_00430f50(local_58 + local_1c,local_28,local_4c + local_18);
                          FUN_004310a0(uVar2);
                          if (*(int *)(DAT_0067cf38 + 0x24) == 0x7531) {
                            FUN_0054ac09(local_598);
                          }
                          else {
                            FUN_0054adbc(local_598,local_60);
                          }
                        }
                        uVar2 = 0xffffdddd;
                        puVar6 = (undefined4 *)FUN_00430f20(local_58 + local_1c,local_4c + local_18)
                        ;
                        FUN_0049da58(*puVar6,puVar6[1],uVar2);
                        local_18c = 3;
                        if (*(int *)(DAT_0067cf38 + 0x24) == 0x7531) {
                          local_18c = 5;
                        }
                        pvVar3 = operator_new(0xc0);
                        local_8 = 0x18;
                        if (pvVar3 == (void *)0x0) {
                          local_59c = (int *)0x0;
                        }
                        else {
                          local_59c = (int *)FUN_004df250(local_18c,0x76,0x3f800000);
                        }
                        local_8 = 0xffffffff;
                        if (local_59c != (int *)0x0) {
                          if (((*(int *)(DAT_0067cf38 + 0x24) == 30000) &&
                              (iVar9 = __ftol(), 0x10 < iVar9 >> 7)) &&
                             ((iVar9 = __ftol(), iVar9 >> 7 < 0x14 &&
                              (iVar9 = __ftol(), 0x1d < iVar9 >> 7)))) {
                            bVar1 = true;
                          }
                          else {
                            bVar1 = false;
                          }
                          if (bVar1) {
                            (**(code **)(*local_59c + 0x44))(0xff222288);
                          }
                          else {
                            (**(code **)(*local_59c + 0x44))(0x88998888);
                          }
                          local_59c[0x1b] = 1;
                          puVar6 = (undefined4 *)
                                   FUN_00430f20(local_58 + local_1c,local_4c + local_18);
                          (**(code **)(*local_59c + 0x3c))(*puVar6,puVar6[1]);
                          FUN_0054adbc(local_59c,local_60);
                        }
                      }
                      else {
                        if (local_44 == 0x7f3) {
                          pvVar3 = operator_new(0x180);
                          local_8 = 0x19;
                          if (pvVar3 == (void *)0x0) {
                            local_5a4 = (int *)0x0;
                          }
                          else {
                            local_5a4 = (int *)FUN_004c8f70(0x1a7,0,0x3fd9999a,0x40666666,0x3fd9999a
                                                            ,0,1,0x50);
                          }
                          local_8 = 0xffffffff;
                          if (local_5a4 != (int *)0x0) {
                            local_5a4[0x41] = 0;
                            (**(code **)(*local_5a4 + 0x3c))(0xffffffff);
                            local_5a4[0x1b] = 1;
                            uVar2 = FUN_00430f50(local_58 + local_1c,local_28 + _DAT_005a5af8,
                                                 local_4c + local_18);
                            FUN_004310a0(uVar2);
                            local_5a4[0x57] = 1;
                            FUN_0054adbc(local_5a4,local_60);
                          }
                          pvVar3 = operator_new(0x180);
                          local_8 = 0x1a;
                          if (pvVar3 == (void *)0x0) {
                            local_5a8 = (int *)0x0;
                          }
                          else {
                            local_5a8 = (int *)FUN_004c8f70(0x1a8,0,0x3f800000,0x3f800000,0x3f800000
                                                            ,0,1,0x50);
                          }
                          local_8 = 0xffffffff;
                          if (local_5a8 != (int *)0x0) {
                            local_5a8[0x41] = 0;
                            (**(code **)(*local_5a8 + 0x3c))(0xffffffff);
                            local_5a8[0x1b] = 1;
                            uVar2 = FUN_00430f50(local_58 + local_1c,local_28 + _DAT_005a34a0,
                                                 local_4c + local_18);
                            FUN_004310a0(uVar2);
                            local_5a8[0x57] = 1;
                            FUN_0054adbc(local_5a8,local_60);
                          }
                          iVar9 = DAT_0067cf38;
                          if (((local_58 + local_1c == (float)_DAT_005a5af0) &&
                              (local_4c + local_18 == (float)_DAT_005a5ae8)) && (DAT_0067cf38 != 0))
                          {
                            *(int **)(DAT_0067cf38 + 0x28e38) = local_5a4;
                            *(int **)(iVar9 + 0x28e3c) = local_5a8;
                          }
                          pvVar3 = operator_new(0x160);
                          local_8 = 0x1b;
                          if (pvVar3 == (void *)0x0) {
                            local_5ac = 0;
                          }
                          else {
                            local_5ac = FUN_0053e1d2();
                          }
                          *(undefined4 *)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4) = local_5ac
                          ;
                          goto LAB_00545171;
                        }
                        if ((local_44 < 0x1ff) || (0x206 < local_44)) {
                          if ((local_44 < 0x208) || (0x212 < local_44)) {
                            if ((((local_44 == 0x5f8) || (local_44 == 0x604)) || (local_44 == 0x605)
                                ) || (((local_44 == 0x606 || (local_44 == 0x607)) ||
                                      (local_44 == 0x63d)))) {
                              local_1bc = 0;
                              if (local_44 == 0x5f8) {
                                local_1bc = 0x613;
                              }
                              if (local_44 == 0x604) {
                                local_1bc = 0x614;
                              }
                              if (local_44 == 0x605) {
                                local_1bc = 0x615;
                              }
                              if (local_44 == 0x606) {
                                local_1bc = 0x616;
                              }
                              if (local_44 == 0x607) {
                                local_1bc = 0x617;
                              }
                              if (local_44 == 0x63d) {
                                local_1bc = 0x63e;
                              }
                              pvVar3 = operator_new(0xcc);
                              local_8 = 0x1e;
                              if (pvVar3 == (void *)0x0) {
                                local_5b8 = 0;
                              }
                              else {
                                local_5b8 = FUN_004d2230(local_1bc,0x223333,local_5c - _DAT_005a4380
                                                         ,0);
                              }
                              local_8 = 0xffffffff;
                              if (local_5b8 != 0) {
                                *(undefined4 *)(local_5b8 + 0x98) = 0x3f800000;
                                *(undefined4 *)(local_5b8 + 0x9c) = 0x3f800000;
                                *(undefined4 *)(local_5b8 + 0x6c) = 1;
                                uVar2 = FUN_00430f50(local_58 + local_1c,local_28,
                                                     local_4c + local_18);
                                FUN_004310a0(uVar2);
                                FUN_0054adbc(local_5b8,local_60);
                              }
                              pvVar3 = operator_new(0x160);
                              local_8 = 0x1f;
                              if (pvVar3 == (void *)0x0) {
                                local_5bc = 0;
                              }
                              else {
                                local_5bc = FUN_0053e1d2();
                              }
                              *(undefined4 *)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4) =
                                   local_5bc;
                            }
                            else if ((local_44 < 0x215) || (599 < local_44)) {
                              if (local_44 == 0x213) {
                                local_50 = local_50 + local_3c;
                                pvVar3 = operator_new(0x170);
                                local_8 = 0x21;
                                if (pvVar3 == (void *)0x0) {
                                  local_5c4 = 0;
                                }
                                else {
                                  local_5c4 = FUN_004f1530(local_54,0);
                                }
                                *(undefined4 *)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4) =
                                     local_5c4;
                              }
                              else {
                                if (local_44 == 0x214) {
                                  local_50 = local_50 + local_3c;
                                  pvVar3 = operator_new(0xcc);
                                  local_8 = 0x22;
                                  if (pvVar3 == (void *)0x0) {
                                    local_5c8 = 0;
                                  }
                                  else {
                                    local_5c8 = FUN_004d2230(local_44,0xaaaaaaaa,
                                                             local_5c - _DAT_005a4380,0);
                                  }
                                  local_8 = 0xffffffff;
                                  if (local_5c8 != 0) {
                                    *(undefined1 *)(local_5c8 + 0xac) = 1;
                                    *(undefined1 *)(local_5c8 + 0xad) = 1;
                                    *(undefined4 *)(local_5c8 + 0xb8) = 5000;
                                    *(float *)(local_5c8 + 0x98) = local_14;
                                    *(float *)(local_5c8 + 0x9c) = local_54;
                                    *(undefined4 *)(local_5c8 + 0x6c) = 1;
                                    uVar2 = FUN_00430f50(local_58 + local_1c,local_28,
                                                         local_4c + local_18);
                                    FUN_004310a0(uVar2);
                                    FUN_0054adbc(local_5c8,local_60);
                                  }
                                  goto LAB_005411aa;
                                }
                                if (local_44 == 8) {
                                  pvVar3 = operator_new(0x170);
                                  local_8 = 0x23;
                                  if (pvVar3 == (void *)0x0) {
                                    local_5cc = 0;
                                  }
                                  else {
                                    local_5cc = FUN_004f1530(0x3f800000,0);
                                  }
                                  *(undefined4 *)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4) =
                                       local_5cc;
                                }
                                else if (local_44 == 9) {
                                  pvVar3 = operator_new(0x170);
                                  local_8 = 0x24;
                                  if (pvVar3 == (void *)0x0) {
                                    local_5d0 = 0;
                                  }
                                  else {
                                    local_5d0 = FUN_004f1530(0x3f800000,2);
                                  }
                                  *(undefined4 *)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4) =
                                       local_5d0;
                                }
                                else {
                                  if (local_44 == 10) {
                                    FUN_00430f20(local_58 + local_1c,local_4c + local_18);
                                    pvVar3 = operator_new(0x18c);
                                    local_8 = 0x25;
                                    if (pvVar3 == (void *)0x0) {
                                      local_5d4 = 0;
                                    }
                                    else {
                                      local_5d4 = FUN_004dda51(local_1cc,local_1c8);
                                    }
                                    local_8 = 0xffffffff;
                                    if (local_5d4 != 0) {
                                      FUN_0054adbc(local_5d4,local_60);
                                    }
                                    goto LAB_005411aa;
                                  }
                                  if (local_44 == 0xd) {
                                    pvVar3 = operator_new(0x170);
                                    local_8 = 0x26;
                                    if (pvVar3 == (void *)0x0) {
                                      local_5d8 = 0;
                                    }
                                    else {
                                      local_5d8 = FUN_004f1530(0x3f800000,1);
                                    }
                                    *(undefined4 *)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4) =
                                         local_5d8;
                                  }
                                  else {
                                    if (((local_44 != 0x60d) && (local_44 != 0x60e)) &&
                                       (local_44 != 0x60f)) {
                                      if (local_44 == 0x7bc) {
                                        if (DAT_0067cf38 != 0) {
                                          FUN_00430f20(local_58 + local_1c,local_4c + local_18);
                                          FUN_0049c70f(local_1e4,local_1e0);
                                          pvVar3 = operator_new(0xcc);
                                          local_8 = 0x28;
                                          if (pvVar3 == (void *)0x0) {
                                            local_5e4 = 0;
                                          }
                                          else {
                                            local_5e4 = FUN_004d2230(local_44,0xaaaaaaaa,
                                                                     local_5c - _DAT_005a4380,0);
                                          }
                                          local_8 = 0xffffffff;
                                          if (local_5e4 != 0) {
                                            *(undefined4 *)(local_5e4 + 0x98) = 0x3f800000;
                                            *(undefined4 *)(local_5e4 + 0x9c) = 0x3f800000;
                                            *(undefined4 *)(local_5e4 + 0x6c) = 1;
                                            uVar2 = FUN_00430f50(local_58 + local_1c,local_28,
                                                                 local_4c + local_18);
                                            FUN_004310a0(uVar2);
                                            FUN_0054adbc(local_5e4,local_60);
                                          }
                                          pvVar3 = operator_new(0xcc);
                                          local_8 = 0x29;
                                          if (pvVar3 == (void *)0x0) {
                                            local_5e8 = 0;
                                          }
                                          else {
                                            local_5e8 = FUN_004d2230(local_44 - 1,0xaaaaaaaa,
                                                                     local_5c - _DAT_005a4380,0);
                                          }
                                          local_8 = 0xffffffff;
                                          if (local_5e8 != 0) {
                                            *(undefined1 *)(local_5e8 + 0xac) = 1;
                                            *(undefined1 *)(local_5e8 + 0xad) = 1;
                                            *(undefined4 *)(local_5e8 + 0xb8) = 5000;
                                            *(undefined4 *)(local_5e8 + 0x98) = 0x3f7ae148;
                                            *(float *)(local_5e8 + 0x9c) =
                                                 local_28 / _DAT_005a5ae0 + _DAT_005a5adc;
                                            *(undefined4 *)(local_5e8 + 0x6c) = 1;
                                            uVar2 = FUN_00430f50(local_58 + local_1c + _DAT_005a5ad8
                                                                 ,0x3e99999a,
                                                                 local_4c + local_18 + _DAT_005a4cec
                                                                );
                                            FUN_004310a0(uVar2);
                                            FUN_0054adbc(local_5e8,local_60);
                                          }
                                          pvVar3 = operator_new(0xcc);
                                          local_8 = 0x2a;
                                          if (pvVar3 == (void *)0x0) {
                                            local_5ec = 0;
                                          }
                                          else {
                                            local_5ec = FUN_004d2230(local_44 + 1,0x88888888,
                                                                     local_5c - _DAT_005a4380,0);
                                          }
                                          local_8 = 0xffffffff;
                                          if (local_5ec != 0) {
                                            *(undefined4 *)(local_5ec + 0x98) = 0x3f800000;
                                            *(undefined4 *)(local_5ec + 0x9c) = 0x3f800000;
                                            *(undefined4 *)(local_5ec + 0x6c) = 1;
                                            uVar2 = FUN_00430f50(local_58 + local_1c + _DAT_005a5ad8
                                                                 ,0x3e99999a,
                                                                 local_4c + local_18 + _DAT_005a4cec
                                                                );
                                            FUN_004310a0(uVar2);
                                            FUN_0054adbc(local_5ec,local_60);
                                          }
                                        }
                                      }
                                      else {
                                        if (local_44 != 0x736) {
                                          pvVar3 = operator_new(0x160);
                                          local_8 = 0x30;
                                          if (pvVar3 == (void *)0x0) {
                                            local_624 = 0;
                                          }
                                          else {
                                            local_624 = FUN_0053e1d2();
                                          }
                                          *(undefined4 *)
                                           (param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4) =
                                               local_624;
                                          goto LAB_00545171;
                                        }
                                        pvVar3 = operator_new(0xcc);
                                        local_8 = 0x2b;
                                        if (pvVar3 == (void *)0x0) {
                                          local_5f0 = 0;
                                        }
                                        else {
                                          local_5f0 = FUN_004d2230(local_44,0xffffffff,
                                                                   local_5c - _DAT_005a4380,0);
                                        }
                                        local_8 = 0xffffffff;
                                        if (local_5f0 != 0) {
                                          *(undefined4 *)(local_5f0 + 0x98) = 0x3f800000;
                                          *(undefined4 *)(local_5f0 + 0x9c) = 0x3f800000;
                                          *(undefined4 *)(local_5f0 + 0x6c) = 3;
                                          uVar2 = FUN_00430f50(local_58 + local_1c,local_28,
                                                               local_4c + local_18);
                                          FUN_004310a0(uVar2);
                                          FUN_0054adbc(local_5f0,local_60);
                                        }
                                        for (local_1f0 = 0; local_1f0 < 0xc;
                                            local_1f0 = local_1f0 + 1) {
                                          pvVar3 = operator_new(0x180);
                                          local_8 = 0x2c;
                                          if (pvVar3 == (void *)0x0) {
                                            local_5f4 = (int *)0x0;
                                          }
                                          else {
                                            local_5f4 = (int *)FUN_004c8f70(2,0,0x40333333,
                                                                            0x40333333,0x40333333,0,
                                                                            1,0x50);
                                          }
                                          local_8 = 0xffffffff;
                                          if (local_5f4 != (int *)0x0) {
                                            local_5f4[0x41] = 0;
                                            (**(code **)(*local_5f4 + 0x3c))(0x55553300);
                                            local_5f4[0x1b] = 1;
                                            fVar12 = local_4c + local_18;
                                            fVar10 = (float10)FUN_00423b00(((float)local_1f0 *
                                                                            _DAT_005a4148 +
                                                                           (float)local_1f0 *
                                                                           _DAT_005a4148) /
                                                                           _DAT_005a3e9c);
                                            fVar12 = (float)(fVar10 * (float10)_DAT_005a40f0 +
                                                            (float10)fVar12);
                                            fVar13 = local_28 + _DAT_005a5ad4;
                                            fVar11 = local_58 + local_1c + _DAT_005a34a0;
                                            fVar10 = (float10)FUN_00493bf0(((float)local_1f0 *
                                                                            _DAT_005a4148 +
                                                                           (float)local_1f0 *
                                                                           _DAT_005a4148) /
                                                                           _DAT_005a3e9c,fVar13,
                                                                           fVar12);
                                            uVar2 = FUN_00430f50((float)(fVar10 * (float10)
                                                  _DAT_005a40f0 + (float10)fVar11),fVar13,fVar12);
                                            FUN_004310a0(uVar2);
                                            FUN_0054adbc(local_5f4,local_60);
                                          }
                                          pvVar3 = operator_new(0x180);
                                          local_8 = 0x2d;
                                          if (pvVar3 == (void *)0x0) {
                                            local_600 = (int *)0x0;
                                          }
                                          else {
                                            local_600 = (int *)FUN_004c8f70(0xb,0,0x3f800000,
                                                                            0x3f800000,0x3f800000,0,
                                                                            8,0x50);
                                          }
                                          local_8 = 0xffffffff;
                                          if (local_600 != (int *)0x0) {
                                            (**(code **)(*local_600 + 0x3c))(0xeeeecc00);
                                            local_600[0x41] = 0;
                                            local_600[0x1b] = 1;
                                            fVar12 = local_4c + local_18;
                                            fVar10 = (float10)FUN_00423b00(((float)local_1f0 *
                                                                            _DAT_005a4148 +
                                                                           (float)local_1f0 *
                                                                           _DAT_005a4148) /
                                                                           _DAT_005a3e9c);
                                            fVar12 = (float)(fVar10 * (float10)_DAT_005a40f0 +
                                                            (float10)fVar12);
                                            fVar13 = local_28 + _DAT_005a5ad4;
                                            fVar11 = local_58 + local_1c + _DAT_005a34a0;
                                            fVar10 = (float10)FUN_00493bf0(((float)local_1f0 *
                                                                            _DAT_005a4148 +
                                                                           (float)local_1f0 *
                                                                           _DAT_005a4148) /
                                                                           _DAT_005a3e9c,fVar13,
                                                                           fVar12);
                                            uVar2 = FUN_00430f50((float)(fVar10 * (float10)
                                                  _DAT_005a40f0 + (float10)fVar11),fVar13,fVar12);
                                            FUN_004310a0(uVar2);
                                            FUN_0054adbc(local_600,local_60);
                                          }
                                        }
                                        for (local_1f0 = 0; local_1f0 < 8; local_1f0 = local_1f0 + 1
                                            ) {
                                          pvVar3 = operator_new(0x180);
                                          local_8 = 0x2e;
                                          if (pvVar3 == (void *)0x0) {
                                            local_60c = (int *)0x0;
                                          }
                                          else {
                                            local_60c = (int *)FUN_004c8f70(2,0,0x40333333,
                                                                            0x40333333,0x40333333,0,
                                                                            1,0x50);
                                          }
                                          local_8 = 0xffffffff;
                                          if (local_60c != (int *)0x0) {
                                            local_60c[0x41] = 0;
                                            (**(code **)(*local_60c + 0x3c))(0x55553300);
                                            local_60c[0x1b] = 1;
                                            fVar12 = local_4c + local_18;
                                            fVar10 = (float10)FUN_00423b00(((float)local_1f0 *
                                                                            _DAT_005a4148 +
                                                                           (float)local_1f0 *
                                                                           _DAT_005a4148) /
                                                                           _DAT_005a366c);
                                            fVar12 = (float)(fVar10 * (float10)_DAT_005a5ad0 +
                                                            (float10)fVar12);
                                            fVar13 = local_28 + _DAT_005a5acc;
                                            fVar11 = local_58 + local_1c + _DAT_005a34a0;
                                            fVar10 = (float10)FUN_00493bf0(((float)local_1f0 *
                                                                            _DAT_005a4148 +
                                                                           (float)local_1f0 *
                                                                           _DAT_005a4148) /
                                                                           _DAT_005a366c,fVar13,
                                                                           fVar12);
                                            uVar2 = FUN_00430f50((float)(fVar10 * (float10)
                                                  _DAT_005a5ad0 + (float10)fVar11),fVar13,fVar12);
                                            FUN_004310a0(uVar2);
                                            FUN_0054adbc(local_60c,local_60);
                                          }
                                        }
                                      }
                                      goto LAB_005411aa;
                                    }
                                    pvVar3 = operator_new(0x168);
                                    local_8 = 0x27;
                                    if (pvVar3 == (void *)0x0) {
                                      local_5dc = 0;
                                    }
                                    else {
                                      local_5dc = FUN_0052fc70();
                                    }
                                    *(undefined4 *)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4) =
                                         local_5dc;
                                  }
                                }
                              }
                            }
                            else {
                              local_50 = local_50 + local_3c;
                              pvVar3 = operator_new(0x160);
                              local_8 = 0x20;
                              if (pvVar3 == (void *)0x0) {
                                local_5c0 = 0;
                              }
                              else {
                                local_5c0 = FUN_0053e1d2();
                              }
                              *(undefined4 *)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4) =
                                   local_5c0;
                            }
                            goto LAB_00545171;
                          }
                          local_50 = local_50 + local_3c;
                          pvVar3 = operator_new(0xcc);
                          local_8 = 0x1d;
                          if (pvVar3 == (void *)0x0) {
                            local_5b4 = 0;
                          }
                          else {
                            local_5b4 = FUN_004d2230(local_44,0x223333,local_5c - _DAT_005a4380,0);
                          }
                          local_8 = 0xffffffff;
                          if (local_5b4 != 0) {
                            *(float *)(local_5b4 + 0x98) = local_14;
                            *(float *)(local_5b4 + 0x9c) = local_54;
                            *(undefined4 *)(local_5b4 + 0x6c) = 1;
                            uVar2 = FUN_00430f50(local_58 + local_1c,local_28,local_4c + local_18);
                            FUN_004310a0(uVar2);
                            FUN_0054adbc(local_5b4,local_60);
                          }
                        }
                        else {
                          local_50 = local_50 + local_3c;
                          local_1b0 = 0x3f800000;
                          local_1ac = 0x3f800000;
                          if (*(int *)(DAT_0067cf38 + 0x24) == 0x7531) {
                            if ((DAT_005bdeec < 2) || (3 < DAT_005bdeec)) {
                              if (DAT_005bdeec < 6) {
                                local_1b0 = 0;
                                local_1ac = 0x3f19999a;
                              }
                            }
                            else {
                              local_1b0 = 0x3f800000;
                              local_1ac = 0;
                            }
                          }
                          pvVar3 = operator_new(0xa4);
                          local_8 = 0x1c;
                          if (pvVar3 == (void *)0x0) {
                            local_5b0 = 0;
                          }
                          else {
                            local_5b0 = FUN_0053d710(local_1b0,local_1ac,0,0x3f800000,1);
                          }
                          local_8 = 0xffffffff;
                          *(undefined4 *)(param_1 + 0x2c + *(int *)(param_1 + 0x28) * 4) = local_5b0
                          ;
                          if (*(int *)(param_1 + 0x2c + *(int *)(param_1 + 0x28) * 4) != 0) {
                            (**(code **)(**(int **)(param_1 + 0x2c + *(int *)(param_1 + 0x28) * 4) +
                                        0x38))(local_58 + local_1c,local_28,local_4c + local_18);
                          }
                          if (*(int *)(DAT_0067cf38 + 0x24) == 0x7531) {
                            DAT_005bdeec = DAT_005bdeec + 1;
                          }
                          *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + 1;
                        }
                      }
                    }
                    else {
                      local_108 = (int *)0x0;
                      local_50 = local_50 + local_3c;
                      if (local_44 == 0x1f5) {
                        pvVar3 = operator_new(0x180);
                        local_8 = 0xd;
                        if (pvVar3 == (void *)0x0) {
                          local_56c = (int *)0x0;
                        }
                        else {
                          local_56c = (int *)FUN_004c8f70(0xb,0,local_14,local_54,local_14,0,8,0x50)
                          ;
                        }
                        local_8 = 0xffffffff;
                        local_108 = local_56c;
                        if (local_56c != (int *)0x0) {
                          (**(code **)(*local_56c + 0x3c))(0xeeeecc00);
                          local_108[0x41] = 0;
                        }
                        pvVar3 = operator_new(0x180);
                        local_8 = 0xe;
                        if (pvVar3 == (void *)0x0) {
                          local_570 = (int *)0x0;
                        }
                        else {
                          local_570 = (int *)FUN_004c8f70(2,0,local_14 * _DAT_005a4524,
                                                          local_54 * _DAT_005a4524,
                                                          local_14 * _DAT_005a4524,0,1,0x50);
                        }
                        local_8 = 0xffffffff;
                        local_10c = local_570;
                        if (local_570 != (int *)0x0) {
                          local_570[0x41] = 0;
                          (**(code **)(*local_570 + 0x3c))(0x55553300);
                          local_10c[0x1b] = 1;
                          uVar2 = FUN_00430f50(local_58 + local_1c,local_28,local_4c + local_18);
                          FUN_004310a0(uVar2);
                          FUN_0054adbc(local_10c,local_60);
                        }
                      }
                      else if (local_44 == 0x1f6) {
                        pvVar3 = operator_new(0x180);
                        local_8 = 0xf;
                        if (pvVar3 == (void *)0x0) {
                          local_574 = (int *)0x0;
                        }
                        else {
                          local_574 = (int *)FUN_004c8f70(0x3d,0,local_14,local_54,local_14,0,6,0x50
                                                         );
                        }
                        local_8 = 0xffffffff;
                        local_108 = local_574;
                        if (local_574 != (int *)0x0) {
                          local_574[0x41] = 0;
                        }
                        pvVar3 = operator_new(0x180);
                        local_8 = 0x10;
                        if (pvVar3 == (void *)0x0) {
                          local_578 = (int *)0x0;
                        }
                        else {
                          local_578 = (int *)FUN_004c8f70(2,0,local_14 * _DAT_005a4524,
                                                          local_54 * _DAT_005a4524,
                                                          local_14 * _DAT_005a4524,0,1,0x50);
                        }
                        local_8 = 0xffffffff;
                        local_110 = local_578;
                        if (local_578 != (int *)0x0) {
                          local_578[0x41] = 0;
                          (**(code **)(*local_578 + 0x3c))(0x55553300);
                          local_110[0x1b] = 1;
                          uVar2 = FUN_00430f50(local_58 + local_1c,local_28,local_4c + local_18);
                          FUN_004310a0(uVar2);
                          FUN_0054adbc(local_110,local_60);
                        }
                      }
                      else if (local_44 == 0x1f7) {
                        pvVar3 = operator_new(0x180);
                        local_8 = 0x11;
                        if (pvVar3 == (void *)0x0) {
                          local_57c = (int *)0x0;
                        }
                        else {
                          local_57c = (int *)FUN_004c8f70(0x65,0,local_14,local_54,local_14,0,8,0x50
                                                         );
                        }
                        local_8 = 0xffffffff;
                        local_108 = local_57c;
                        if (local_57c != (int *)0x0) {
                          local_57c[0x41] = 0;
                          (**(code **)(*local_57c + 0x3c))(0xff5500ff);
                        }
                        pvVar3 = operator_new(0x180);
                        local_8 = 0x12;
                        if (pvVar3 == (void *)0x0) {
                          local_580 = (int *)0x0;
                        }
                        else {
                          local_580 = (int *)FUN_004c8f70(0x65,0,local_14 * _DAT_005a34a0,
                                                          local_54 * _DAT_005a34a0,
                                                          local_14 * _DAT_005a34a0,0,8,0x50);
                        }
                        local_8 = 0xffffffff;
                        local_114 = local_580;
                        if (local_580 != (int *)0x0) {
                          local_580[0x41] = 0;
                          (**(code **)(*local_580 + 0x3c))(0xffffffff);
                          local_114[0x1b] = 1;
                          uVar2 = FUN_00430f50(local_58 + local_1c,
                                               local_28 - local_54 * _DAT_005a4158,
                                               local_4c + local_18);
                          FUN_004310a0(uVar2);
                          FUN_0054adbc(local_114,local_60);
                        }
                        pvVar3 = operator_new(0x180);
                        local_8 = 0x13;
                        if (pvVar3 == (void *)0x0) {
                          local_584 = (int *)0x0;
                        }
                        else {
                          local_584 = (int *)FUN_004c8f70(2,0,local_14 * _DAT_005a4524,
                                                          local_54 * _DAT_005a4524,
                                                          local_14 * _DAT_005a4524,0,1,0x50);
                        }
                        local_8 = 0xffffffff;
                        local_118 = local_584;
                        if (local_584 != (int *)0x0) {
                          local_584[0x41] = 0;
                          (**(code **)(*local_584 + 0x3c))(0xff330055);
                          local_118[0x1b] = 1;
                          uVar2 = FUN_00430f50(local_58 + local_1c,local_28,local_4c + local_18);
                          FUN_004310a0(uVar2);
                          FUN_0054adbc(local_118,local_60);
                        }
                      }
                      else if (local_44 == 0x1f8) {
                        pvVar3 = operator_new(0x180);
                        local_8 = 0x14;
                        if (pvVar3 == (void *)0x0) {
                          local_588 = (int *)0x0;
                        }
                        else {
                          local_588 = (int *)FUN_004c8f70(0x38,0,local_14,local_54,local_14,0,1,0x50
                                                         );
                        }
                        local_8 = 0xffffffff;
                        local_108 = local_588;
                        if (local_588 != (int *)0x0) {
                          (**(code **)(*local_588 + 0x3c))(0xffff0000);
                          local_108[0x41] = 2;
                        }
                      }
                      else if (local_44 == 0x1f9) {
                        pvVar3 = operator_new(0x180);
                        local_8 = 0x15;
                        if (pvVar3 == (void *)0x0) {
                          local_58c = (int *)0x0;
                        }
                        else {
                          local_58c = (int *)FUN_004c8f70(0x4f,0,local_14,local_54,local_14,0,1,0x50
                                                         );
                        }
                        local_8 = 0xffffffff;
                        local_108 = local_58c;
                        if (local_58c != (int *)0x0) {
                          local_58c[0x41] = 0;
                          local_58c[0x42] = 0;
                          local_58c[0x45] = (int)local_5c;
                          (**(code **)(*local_58c + 0x3c))(0x33330000);
                        }
                      }
                      if (local_108 != (int *)0x0) {
                        local_108[0x1b] = 1;
                        uVar2 = FUN_00430f50(local_58 + local_1c,local_28,local_4c + local_18);
                        FUN_004310a0(uVar2);
                        FUN_0054adbc(local_108,local_60);
                      }
                      if ((500 < local_44) && (local_44 < 0x1f8)) {
                        iVar9 = local_44 - 0x1f5;
                        local_130[0] = &DAT_00ffaa00;
                        local_130[1] = (undefined *)0x33331100;
                        local_130[2] = &DAT_00ffaa00;
                        local_130[3] = (undefined *)0x33331100;
                        local_130[4] = &DAT_00aa00ff;
                        local_11c = 0x11033;
                        puVar6 = (undefined4 *)FUN_00430f20(local_58 + local_1c,local_4c + local_18)
                        ;
                        FUN_0049cb58(local_144,*puVar6,puVar6[1]);
                        uVar4 = __ftol();
                        uVar5 = __ftol();
                        uVar7 = __ftol();
                        uVar8 = __ftol();
                        uVar4 = ((uVar4 & 0xff) + ((uint)local_130[iVar9 * 2] >> 0x18) >> 1) << 0x18
                                | ((uVar5 & 0xff) +
                                   (((uint)local_130[iVar9 * 2] & 0xff0000) >> 0x10) >> 1) << 0x10 |
                                ((uVar7 & 0xff) + (((uint)local_130[iVar9 * 2] & 0xff00) >> 8) >> 1)
                                << 8 | (uVar8 & 0xff) + ((uint)local_130[iVar9 * 2] & 0xff) >> 1;
                        local_134 = uVar4;
                        puVar6 = (undefined4 *)FUN_00430f20(local_58 + local_1c,local_4c + local_18)
                        ;
                        FUN_0049da58(*puVar6,puVar6[1],uVar4);
                        local_174 = 5;
                        if (*(int *)(DAT_0067cf38 + 0x24) == 0x7531) {
                          local_174 = 9;
                        }
                        pvVar3 = operator_new(0xc0);
                        local_8 = 0x16;
                        if (pvVar3 == (void *)0x0) {
                          local_590 = (int *)0x0;
                        }
                        else {
                          local_590 = (int *)FUN_004df250(local_174,7,0x3f800000);
                        }
                        local_8 = 0xffffffff;
                        if (local_590 != (int *)0x0) {
                          (**(code **)(*local_590 + 0x44))(local_130[iVar9 * 2 + 1]);
                          local_590[0x1b] = 1;
                          puVar6 = (undefined4 *)
                                   FUN_00430f20(local_58 + local_1c,local_4c + local_18);
                          (**(code **)(*local_590 + 0x3c))(*puVar6,puVar6[1]);
                          FUN_0054adbc(local_590,local_60);
                        }
                      }
                    }
                  }
                  goto LAB_005411aa;
                }
                local_104 = local_44 == 0x1da;
                if (((((local_44 == 0x111) || (local_44 == 0x112)) || (local_44 == 0xc3)) ||
                    (((local_44 == 0x2b9 || (local_44 == 699)) ||
                     ((local_44 == 0x5f0 || ((local_44 == 0x5ff || (local_44 == 0x69f)))))))) ||
                   (local_44 == 0x7c9)) {
                  local_104 = 2;
                }
                if ((((local_44 == 0x124) || (local_44 == 0x1ea)) || (local_44 == 0x5f6)) ||
                   ((local_44 == 0x681 || (local_44 == 0x7d5)))) {
                  local_104 = 3;
                }
                if (local_44 == 0x25f) {
                  local_104 = 4;
                }
                if (local_44 == 0x262) {
                  local_104 = 5;
                }
                if (local_44 == 0x266) {
                  local_104 = 6;
                }
                if (local_44 == 0x6d6) {
                  local_104 = 7;
                }
                if (local_44 == 0x6cb) {
                  local_104 = 8;
                }
                if (local_44 == 0x6af) {
                  local_104 = 9;
                }
                if (local_44 == 0x73f) {
                  local_104 = 0xb;
                }
                pvVar3 = operator_new(0x180);
                local_8 = 0xc;
                if (pvVar3 == (void *)0x0) {
                  local_568 = 0;
                }
                else {
                  local_568 = FUN_0053aa20(local_104);
                }
                *(undefined4 *)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4) = local_568;
              }
            }
            else {
              pvVar3 = operator_new(0x1a8);
              local_8 = 9;
              if (pvVar3 == (void *)0x0) {
                local_55c = 0;
              }
              else {
                local_55c = FUN_00547290(local_44);
              }
              *(undefined4 *)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4) = local_55c;
            }
          }
          else {
            pvVar3 = operator_new(0x194);
            local_8 = 8;
            if (pvVar3 == (void *)0x0) {
              local_558 = 0;
            }
            else {
              local_558 = FUN_00549d60(local_44);
            }
            *(undefined4 *)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4) = local_558;
          }
LAB_00545171:
          local_8 = 0xffffffff;
          if ((local_44 == 0x291) || (local_44 == 0x292)) {
            *(undefined4 *)(*(int *)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4) + 0x4c) = 1;
          }
          *(uint *)(*(int *)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4) + 0x24) = local_44;
          *(uint *)(*(int *)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4) + 0x3c) = local_24;
          *(uint *)(*(int *)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4) + 0x40) = local_40;
          if (((((0x74d < local_44) && (local_44 < 0x754)) ||
               ((0x754 < local_44 && (local_44 < 0x761)))) ||
              (((0x762 < local_44 && (local_44 < 0x768)) ||
               ((0x774 < local_44 && (local_44 < 0x77a)))))) ||
             (((((0x77b < local_44 && (local_44 < 0x780)) || (local_44 == 0x76d)) ||
               (local_44 == 0x78b)) || ((0x78e < local_44 && (local_44 < 0x791)))))) {
            *(undefined4 *)(*(int *)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4) + 0x15c) = 1;
          }
          (**(code **)(**(int **)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4) + 0x38))();
          (**(code **)(**(int **)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4) + 0x40))
                    (0,local_5c,0);
          (**(code **)(**(int **)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4) + 0x3c))
                    (local_58 + local_1c,local_28,local_4c + local_18);
          if (((local_44 == 0x7f3) && (local_58 + local_1c == (float)_DAT_005a5af0)) &&
             ((local_4c + local_18 == (float)_DAT_005a5ae8 && (DAT_0067cf38 != 0)))) {
            *(undefined4 *)(DAT_0067cf38 + 0x28e34) =
                 *(undefined4 *)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4);
          }
          (**(code **)(**(int **)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4) + 100))
                    (*(undefined4 *)(param_1 + 0x24),local_1c,local_18);
          FUN_0054adbc(*(undefined4 *)(param_1 + 0x4c + *(int *)(param_1 + 0x48) * 4),local_60);
        }
        else {
          pvVar3 = operator_new(400);
          local_8 = 7;
          if (pvVar3 == (void *)0x0) {
            local_550 = (int *)0x0;
          }
          else {
            local_550 = (int *)FUN_0053cd40(local_44);
          }
          local_8 = 0xffffffff;
          local_100 = local_550;
          if (local_550 != (int *)0x0) {
            if (local_44 - 0x137 < 6) {
              *(undefined2 *)(local_550 + 0x58) = 0;
              iVar9 = __ftol();
              if ((((iVar9 >> 7 < 0x1b) || (iVar9 = __ftol(), 0x1e < iVar9 >> 7)) ||
                  (iVar9 = __ftol(), iVar9 >> 7 < 0x15)) || (iVar9 = __ftol(), 0x18 < iVar9 >> 7)) {
                bVar1 = false;
              }
              else {
                bVar1 = true;
              }
              if (bVar1) {
                *(undefined2 *)((int)local_100 + 0x162) = 9;
              }
              else {
                *(ushort *)((int)local_100 + 0x162) = (ushort)(byte)((char)local_44 - 0x37);
              }
            }
            else {
              *(undefined2 *)(local_550 + 0x58) = 2;
              *(ushort *)((int)local_550 + 0x162) = (ushort)(byte)((char)local_44 - 0x39);
            }
            iVar9 = *(int *)(param_1 + 0x24);
            uVar4 = __ftol();
            uVar4 = uVar4 & 0x8000007f;
            if ((int)uVar4 < 0) {
              uVar4 = (uVar4 - 1 | 0xffffff80) + 1;
            }
            uVar5 = __ftol();
            uVar5 = uVar5 & 0x8000007f;
            if ((int)uVar5 < 0) {
              uVar5 = (uVar5 - 1 | 0xffffff80) + 1;
            }
            *(undefined1 *)(iVar9 + 0x100cc + uVar4 * 0x80 + uVar5) = 1;
            iVar9 = *(int *)(param_1 + 0x24);
            uVar4 = __ftol();
            uVar4 = uVar4 & 0x8000007f;
            if ((int)uVar4 < 0) {
              uVar4 = (uVar4 - 1 | 0xffffff80) + 1;
            }
            uVar5 = __ftol();
            uVar5 = uVar5 & 0x8000007f;
            if ((int)uVar5 < 0) {
              uVar5 = (uVar5 - 1 | 0xffffff80) + 1;
            }
            *(undefined1 *)(iVar9 + 0x100cc + uVar4 * 0x80 + uVar5) = 1;
            iVar9 = *(int *)(param_1 + 0x24);
            uVar4 = __ftol();
            uVar4 = uVar4 & 0x8000007f;
            if ((int)uVar4 < 0) {
              uVar4 = (uVar4 - 1 | 0xffffff80) + 1;
            }
            uVar5 = __ftol();
            uVar5 = uVar5 & 0x8000007f;
            if ((int)uVar5 < 0) {
              uVar5 = (uVar5 - 1 | 0xffffff80) + 1;
            }
            *(undefined1 *)(iVar9 + 0x100cc + uVar4 * 0x80 + uVar5) = 1;
            iVar9 = *(int *)(param_1 + 0x24);
            uVar4 = __ftol();
            uVar4 = uVar4 & 0x8000007f;
            if ((int)uVar4 < 0) {
              uVar4 = (uVar4 - 1 | 0xffffff80) + 1;
            }
            uVar5 = __ftol();
            uVar5 = uVar5 & 0x8000007f;
            if ((int)uVar5 < 0) {
              uVar5 = (uVar5 - 1 | 0xffffff80) + 1;
            }
            *(undefined1 *)(iVar9 + 0x100cc + uVar4 * 0x80 + uVar5) = 1;
            local_100[9] = local_44;
            local_100[0xf] = local_24;
            local_100[0x10] = local_40;
            (**(code **)(*local_100 + 0x38))();
            (**(code **)(*local_100 + 0x40))(0,local_5c,0);
            (**(code **)(*local_100 + 0x3c))(local_58 + local_1c,local_28,local_4c + local_18);
            if (((local_44 == 0x138) && (*(int *)(*(int *)(param_1 + 0x24) + 0x20a20) == 0x1c)) &&
               (*(int *)(*(int *)(param_1 + 0x24) + 0x20a24) == 0x18)) {
              FUN_0054ac09(local_100);
            }
            else {
              FUN_0054adbc(local_100,local_60);
            }
          }
        }
LAB_005411aa:
        *(int *)(param_1 + 0x48) = *(int *)(param_1 + 0x48) + 1;
      }
      if (local_34 != (void *)0x0) {
        FUN_0058ffd8(local_34);
      }
      iVar9 = FUN_00494dcf();
      if ((iVar9 == 30000) &&
         (*(int *)(&DAT_005bbe18 +
                  *(int *)(*(int *)(param_1 + 0x24) + 0x20a20) * 4 +
                  (*(int *)(*(int *)(param_1 + 0x24) + 0x20a24) + 0x20) * 0x80) !=
          local_2c +
          *(int *)(*(int *)(param_1 + 0x24) + 0x20a24) *
          *(int *)(*(int *)(param_1 + 0x24) + 0x20a20))) {
        FUN_00431159(s_CheckSum_Error____d__d_TMGround__005bdfa0,
                     *(undefined4 *)(*(int *)(param_1 + 0x24) + 0x20a20),
                     *(undefined4 *)(*(int *)(param_1 + 0x24) + 0x20a24),
                     *(undefined4 *)
                      (&DAT_005bbe18 +
                      *(int *)(*(int *)(param_1 + 0x24) + 0x20a20) * 4 +
                      (*(int *)(*(int *)(param_1 + 0x24) + 0x20a24) + 0x20) * 0x80),
                     local_2c +
                     *(int *)(*(int *)(param_1 + 0x24) + 0x20a24) *
                     *(int *)(*(int *)(param_1 + 0x24) + 0x20a20));
        if (*(int *)(DAT_0067cf38 + 0x54) == 0) {
          FUN_0049ed58(9,0,0,0,0);
        }
        *(undefined4 *)(DAT_0067cf38 + 0x54) = 1;
        uVar2 = 0;
      }
      else {
        uVar2 = 1;
      }
    }
  }
  ExceptionList = local_10;
  return uVar2;
}

