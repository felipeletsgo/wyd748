// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004110f5 | Name: FUN_004110f5


/* WARNING: Removing unreachable block (ram,0x00412845) */
/* WARNING: Removing unreachable block (ram,0x00412851) */
/* WARNING: Removing unreachable block (ram,0x00412937) */
/* WARNING: Removing unreachable block (ram,0x00412943) */

undefined4 __thiscall FUN_004110f5(int *param_1,undefined4 param_2,undefined4 param_3)

{
  bool bVar1;
  undefined2 uVar2;
  undefined4 uVar3;
  int iVar4;
  void *pvVar5;
  int *piVar6;
  int iVar7;
  undefined4 *puVar8;
  undefined4 local_578;
  undefined4 local_574;
  undefined4 local_570;
  undefined4 local_56c;
  undefined4 local_568;
  undefined4 local_564;
  undefined4 local_560;
  undefined4 local_55c;
  undefined4 local_558;
  undefined4 local_554;
  undefined4 local_530;
  undefined4 local_528;
  undefined4 local_524;
  int local_520;
  int local_41c;
  int aiStack_418 [4];
  int *local_408;
  int local_404;
  undefined4 *local_400;
  int local_3fc;
  int local_3f8;
  int aiStack_3f4 [7];
  int local_3d8;
  int *local_3d4;
  int local_3d0;
  undefined4 *local_3cc;
  int local_3c8;
  int local_3c4;
  int aiStack_3c0 [4];
  int *local_3b0;
  int local_3ac;
  undefined4 *local_3a8;
  int local_3a4;
  int local_3a0;
  int aiStack_39c [13];
  int local_368;
  int *local_364;
  int local_360;
  undefined4 *local_35c;
  int local_358;
  int local_354;
  int aiStack_350 [13];
  int local_31c;
  int *local_318;
  int local_314;
  undefined4 *local_310;
  int local_30c;
  int local_308;
  int aiStack_304 [13];
  int local_2d0;
  int local_2cc;
  undefined4 local_2c8;
  undefined4 local_2c4;
  void *local_2c0;
  undefined4 local_2bc;
  int local_2b8;
  int local_2b4;
  short local_2b0;
  int aiStack_2ac [5];
  undefined4 local_298;
  void *local_294;
  int local_290;
  undefined4 local_28c;
  void *local_288;
  undefined4 local_284;
  void *local_280;
  int local_27c;
  int local_278;
  short local_274;
  int aiStack_270 [7];
  int local_254;
  int local_250;
  int local_24c;
  uint local_248;
  undefined4 local_244;
  int local_240;
  int local_23c;
  int local_238;
  int local_234;
  int local_230;
  undefined4 local_22c;
  int local_228;
  undefined4 local_224;
  int local_220;
  int local_21c;
  uint local_218;
  int local_214;
  uint local_210;
  undefined4 local_20c;
  int local_208;
  int local_204;
  undefined4 local_200;
  void *local_1fc;
  int local_1f8;
  int local_1f4;
  short local_1f0;
  int *local_1ec [5];
  int local_1d8;
  int local_1d4;
  int local_1d0;
  int local_1cc;
  int local_1c8;
  int local_1c4;
  int local_1c0;
  int local_1bc;
  int local_1b8;
  int local_1b4;
  int local_1b0;
  int local_1ac;
  int local_1a8;
  int local_1a4;
  uint local_1a0;
  int local_19c;
  undefined4 local_198;
  void *local_194;
  int local_190;
  int local_18c;
  short local_188;
  int *local_184 [6];
  int local_16c;
  int local_168;
  undefined2 local_164;
  undefined2 local_160;
  undefined1 local_15c [2];
  undefined2 local_15a;
  undefined2 local_158;
  undefined2 local_156;
  undefined4 local_154;
  undefined2 *local_150;
  undefined4 local_14c;
  undefined2 *local_148;
  int local_144;
  undefined4 local_140;
  void *local_13c;
  undefined4 local_138;
  void *local_134;
  undefined2 *local_130;
  int local_12c;
  int local_128;
  int local_124;
  int local_120;
  undefined4 local_11c;
  void *local_118;
  undefined4 *local_114;
  int local_110;
  int local_10c;
  short local_108;
  int *local_104 [6];
  int *local_ec;
  int local_e8;
  int local_e4;
  int local_e0;
  int local_dc;
  int local_d8;
  uint local_d4;
  undefined4 local_d0;
  void *local_cc;
  int local_c8;
  int local_c4;
  undefined4 local_c0;
  void *local_bc;
  int local_b8;
  int local_b4;
  short local_b0;
  undefined4 auStack_ac [6];
  int local_94;
  int local_90;
  int *local_8c;
  int *local_88;
  int local_84;
  int *local_80;
  undefined4 local_7c;
  int local_78;
  int local_74;
  int local_70;
  void *local_6c;
  int local_68;
  int local_64;
  int local_60;
  short local_5c;
  undefined4 auStack_58 [15];
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059ef61;
  local_10 = ExceptionList;
  if (param_1[0x100] == 7) {
    ExceptionList = &local_10;
    local_14 = (**(code **)(*param_1 + 0xb4))(param_2,param_3);
    if ((local_14 == 0) || (*(int *)(local_14 + 0x94) != -1)) {
      ExceptionList = local_10;
      return 2;
    }
    local_5c = (short)*(undefined4 *)(local_14 + 0x1d0) +
               (short)*(undefined4 *)(local_14 + 0x1d4) * 9;
    local_64 = DAT_0067cf38;
    for (local_60 = 0; local_60 < 0xf; local_60 = local_60 + 1) {
      uVar3 = (**(code **)(**(int **)(local_64 + 0x28) + 0x48))(local_60 + 0x2100);
      auStack_58[local_60] = uVar3;
    }
    local_18 = 0;
    local_60 = 0;
    do {
      if (0xe < local_60) {
LAB_00411316:
        if (local_18 != 0) {
          local_1c = (**(code **)(**(int **)(local_64 + 0x28) + 0x48))(0x269);
          local_68 = (**(code **)(**(int **)(local_64 + 0x28) + 0x48))(0x259);
          *(undefined4 *)(local_1c + 0x1e8) = 0;
          *(undefined4 *)(local_68 + 0x1e8) = 0;
          uVar3 = (**(code **)(**(int **)(DAT_013b71e0 + 0xf4) + 8))();
          *(undefined4 *)(local_64 + 0x274d0) = uVar3;
          *(undefined1 *)(DAT_013b71e8 + 0xcf8) = *(undefined1 *)(local_1c + 0x1e8);
          *(undefined2 *)(DAT_013b71e8 + 0xc64) = 899;
          FUN_0055f2dd(DAT_013b71e8 + 0xc60,0x9c);
          ExceptionList = local_10;
          return 1;
        }
        ExceptionList = local_10;
        return 0;
      }
      iVar4 = (**(code **)(*(int *)auStack_58[local_60] + 0xb4))(0,0);
      if (iVar4 == 0) {
        local_6c = operator_new(8);
        FUN_0058f220(local_6c,*(undefined4 *)(local_14 + 0x670),8);
        pvVar5 = operator_new(0x678);
        local_8 = 0;
        if (pvVar5 == (void *)0x0) {
          local_520 = 0;
        }
        else {
          local_520 = FUN_0040d13e(0,local_6c,0,0);
        }
        local_8 = 0xffffffff;
        local_70 = local_520;
        if (local_520 != 0) {
          (**(code **)(*(int *)auStack_58[local_60] + 0x8c))(local_520,0,0);
        }
        FUN_0058f220(DAT_013b71e8 + 0xc6c + local_60 * 8,*(undefined4 *)(local_14 + 0x670),8);
        *(undefined1 *)(DAT_013b71e8 + local_60 + 0xce4) = (undefined1)local_5c;
        *(undefined4 *)(local_14 + 0x94) = 0xffff0000;
        local_18 = 1;
        goto LAB_00411316;
      }
      local_60 = local_60 + 1;
    } while( true );
  }
  if (param_1[0x100] == 10) {
    local_74 = DAT_0067cf38;
    if (*(int *)(DAT_0067cf38 + 0x24) == 30000) {
      ExceptionList = &local_10;
      local_80 = (int *)(**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x48))(0x272);
      local_84 = local_74;
      local_78 = (**(code **)(*param_1 + 0xb4))(param_2,param_3);
      local_7c = (**(code **)(**(int **)(local_74 + 0x28) + 0x48))(0x29b);
      iVar4 = FUN_0040c0f0();
      if (iVar4 == 0) {
        ExceptionList = local_10;
        return 1;
      }
      if ((local_78 == 0) || (*(int *)(local_78 + 0x670) == 0)) {
        ExceptionList = local_10;
        return 1;
      }
      iVar4 = FUN_0054cd07(*(undefined4 *)(local_78 + 0x670),0x6f);
      if (iVar4 != 0) {
        FUN_00403df2(&DAT_00a3dbf8,2000);
        (**(code **)(**(int **)(local_84 + 0x84) + 0x88))(1,1);
        ExceptionList = local_10;
        return 1;
      }
      if (((((**(short **)(local_78 + 0x670) == 0x1fc) || (**(short **)(local_78 + 0x670) == 0x1fd))
           || (**(short **)(local_78 + 0x670) == 0x20a)) ||
          ((0x20d < **(short **)(local_78 + 0x670) && (**(short **)(local_78 + 0x670) < 0x21a)))) ||
         ((**(short **)(local_78 + 0x670) == 0x2eb ||
          ((0xc7f < **(short **)(local_78 + 0x670) && (**(short **)(local_78 + 0x670) < 0xce4))))))
      {
        FUN_00403df2(&DAT_00a3dbf8,2000);
        (**(code **)(**(int **)(local_84 + 0x84) + 0x88))(1,1);
        ExceptionList = local_10;
        return 1;
      }
      iVar4 = FUN_0040c0f0();
      if ((iVar4 == 0) && (*(int *)(local_78 + 0x94) == -1)) {
        local_8c = (int *)(**(code **)(**(int **)(local_74 + 0x28) + 0x48))(0x276);
        local_88 = (int *)(**(code **)(**(int **)(local_74 + 0x28) + 0x48))(0x273);
        *(undefined4 *)(local_78 + 0x94) = 0xffff00ff;
        *(undefined4 *)(local_84 + 0x26ee0) = 4;
        *(int *)(local_84 + 0x26ee4) = *(int *)(local_78 + 0x1d0) + *(int *)(local_78 + 0x1d4) * 9;
        (**(code **)(*local_8c + 0x80))(&DAT_00a38878,0);
        (**(code **)(**(int **)(local_74 + 0x28) + 0x40))(local_88);
        _memset(local_88 + 0x3d3,0,0x100);
        (**(code **)(*local_88 + 0x90))(&DAT_005ccf28);
        (**(code **)(*local_80 + 0x60))(1);
      }
    }
    ExceptionList = local_10;
    return 1;
  }
  if (param_1[0x100] == 0xd) {
    ExceptionList = &local_10;
    local_90 = (**(code **)(*param_1 + 0xb4))(param_2,param_3);
    if (local_90 == 0) {
      ExceptionList = local_10;
      return 2;
    }
    if (*(int *)(local_90 + 0x94) != -1) {
      ExceptionList = local_10;
      return 2;
    }
    local_b0 = (short)*(undefined4 *)(local_90 + 0x1d0) +
               (short)*(undefined4 *)(local_90 + 0x1d4) * 9;
    local_b8 = DAT_0067cf38;
    for (local_b4 = 0; local_b4 < 6; local_b4 = local_b4 + 1) {
      uVar3 = (**(code **)(**(int **)(local_b8 + 0x28) + 0x48))(local_b4 + 0x557);
      auStack_ac[local_b4] = uVar3;
    }
    local_94 = 0;
    if (DAT_005ccf04 == 0) {
      for (local_b4 = 0; local_b4 < 6; local_b4 = local_b4 + 1) {
        iVar4 = (**(code **)(*(int *)auStack_ac[local_b4] + 0xb4))(0,0);
        if (iVar4 == 0) {
          if (local_b4 == 0) {
            local_c4 = FUN_0054e06c(*(undefined4 *)(local_90 + 0x670));
            if ((local_c4 < 9) ||
               (*(short *)(&DAT_00d449d6 + **(short **)(local_90 + 0x670) * 0x8c) == 0)) {
              FUN_00403df2(&DAT_00a3a178,2000);
              (**(code **)(**(int **)(local_b8 + 0x84) + 0x88))(1,1);
              ExceptionList = local_10;
              return 1;
            }
          }
          else if (local_b4 == 1) {
            if ((**(short **)(local_90 + 0x670) < 0x989) || (0x992 < **(short **)(local_90 + 0x670))
               ) {
              FUN_00403df2(&DAT_00a3cbf8,2000);
              (**(code **)(**(int **)(local_b8 + 0x84) + 0x88))(1,1);
              ExceptionList = local_10;
              return 1;
            }
          }
          else {
            local_c8 = FUN_0054e06c(*(undefined4 *)(local_90 + 0x670));
            if ((local_c8 < 7) ||
               (*(short *)(&DAT_00d449d6 + **(short **)(local_90 + 0x670) * 0x8c) == 0)) {
              FUN_00403df2(&DAT_00a3a278,2000);
              (**(code **)(**(int **)(local_b8 + 0x84) + 0x88))(1,1);
              ExceptionList = local_10;
              return 1;
            }
          }
          local_bc = operator_new(8);
          FUN_0058f220(local_bc,*(undefined4 *)(local_90 + 0x670),8);
          pvVar5 = operator_new(0x678);
          local_8 = 1;
          if (pvVar5 == (void *)0x0) {
            local_524 = 0;
          }
          else {
            local_524 = FUN_0040d13e(0,local_bc,0,0);
          }
          local_8 = 0xffffffff;
          local_c0 = local_524;
          (**(code **)(*(int *)auStack_ac[local_b4] + 0x8c))(local_524,0,0);
          FUN_0058f220(DAT_013b71e8 + 0xd08 + local_b4 * 8,*(undefined4 *)(local_90 + 0x670),8);
          *(undefined1 *)(DAT_013b71e8 + local_b4 + 0xd48) = (undefined1)local_b0;
          *(undefined4 *)(local_90 + 0x94) = 0xffff0000;
          local_94 = 1;
          break;
        }
      }
    }
    else if (DAT_005ccf04 == 1) {
      for (local_b4 = 0; local_b4 < 6; local_b4 = local_b4 + 1) {
        iVar4 = (**(code **)(*(int *)auStack_ac[local_b4] + 0xb4))(0,0);
        if (iVar4 == 0) {
          if (local_b4 == 0) {
            local_d8 = FUN_0054e06c(*(undefined4 *)(local_90 + 0x670));
            local_d4 = (uint)*(short *)(&DAT_00d449d6 + **(short **)(local_90 + 0x670) * 0x8c);
            local_dc = FUN_0054cd07(*(undefined4 *)(local_90 + 0x670),0x57);
            if ((local_d8 != 9) ||
               (*(short *)(&DAT_00d449d6 + **(short **)(local_90 + 0x670) * 0x8c) == 0)) {
              FUN_00403df2(&DAT_00a3d4f8,2000);
              (**(code **)(**(int **)(local_b8 + 0x84) + 0x88))(1,1);
              ExceptionList = local_10;
              return 1;
            }
            if (((local_d4 & 0x3f) != 0) || (local_d4 == 0x80)) {
              FUN_00403df2(&DAT_00a3cb78,2000);
              (**(code **)(**(int **)(local_b8 + 0x84) + 0x88))(1,1);
              ExceptionList = local_10;
              return 1;
            }
            if (4 < local_dc) {
              FUN_00403df2(&DAT_00a3f4f8,2000);
              (**(code **)(**(int **)(local_b8 + 0x84) + 0x88))(1,1);
              ExceptionList = local_10;
              return 1;
            }
          }
          else if (local_b4 == 1) {
            if (**(short **)(local_90 + 0x670) != 0xd7f) {
              FUN_00403df2(&DAT_00a3f578,2000);
              (**(code **)(**(int **)(local_b8 + 0x84) + 0x88))(1,1);
              ExceptionList = local_10;
              return 1;
            }
          }
          else if ((1 < local_b4) && (local_b4 < 6)) {
            if ((**(short **)(local_90 + 0x670) < 0x989) ||
               (((0x992 < **(short **)(local_90 + 0x670) ||
                 (**(short **)(local_90 + 0x670) == 0x6ee)) ||
                (**(short **)(local_90 + 0x670) == 0x2b9)))) {
              FUN_00403df2(&DAT_00a3cbf8,2000);
              (**(code **)(**(int **)(local_b8 + 0x84) + 0x88))(1,1);
              ExceptionList = local_10;
              return 1;
            }
            if (((2 < local_b4) && (local_b4 < 6)) &&
               (*(short *)(DAT_013b71e8 + 0xd18) != **(short **)(local_90 + 0x670))) {
              FUN_00403df2(&DAT_00a3f5f8,2000);
              (**(code **)(**(int **)(local_b8 + 0x84) + 0x88))(1,1);
              ExceptionList = local_10;
              return 1;
            }
          }
          local_cc = operator_new(8);
          FUN_0058f220(local_cc,*(undefined4 *)(local_90 + 0x670),8);
          pvVar5 = operator_new(0x678);
          local_8 = 2;
          if (pvVar5 == (void *)0x0) {
            local_528 = 0;
          }
          else {
            local_528 = FUN_0040d13e(0,local_cc,0,0);
          }
          local_8 = 0xffffffff;
          local_d0 = local_528;
          (**(code **)(*(int *)auStack_ac[local_b4] + 0x8c))(local_528,0,0);
          FUN_0058f220(DAT_013b71e8 + 0xd08 + local_b4 * 8,*(undefined4 *)(local_90 + 0x670),8);
          *(undefined1 *)(DAT_013b71e8 + local_b4 + 0xd48) = (undefined1)local_b0;
          *(undefined4 *)(local_90 + 0x94) = 0xffff0000;
          local_94 = 1;
          break;
        }
      }
    }
    if (local_94 != 0) {
      ExceptionList = local_10;
      return 1;
    }
    ExceptionList = local_10;
    return 0;
  }
  if (param_1[0x100] != 0xf) {
    if (param_1[0x100] != 0x11) {
      if (param_1[0x100] != 0x13) {
        if (param_1[0x100] != 0x15) {
          if (param_1[0x100] != 0x17) {
            if (param_1[0x100] == 6) {
              return 1;
            }
            if (param_1[0x100] == 0xc) {
              ExceptionList = &local_10;
              local_2d0 = (**(code **)(*param_1 + 0xb4))(param_2,param_3);
              if (local_2d0 == 0) {
                ExceptionList = local_10;
                return 1;
              }
              local_30c = DAT_0067cf38;
              for (local_308 = 0; local_308 < 0xd; local_308 = local_308 + 1) {
                iVar4 = (**(code **)(**(int **)(local_30c + 0x28) + 0x48))(local_308 + 0x557);
                aiStack_304[local_308] = iVar4;
              }
              local_308 = 0;
              while( true ) {
                if (0xc < local_308) {
                  ExceptionList = local_10;
                  return 1;
                }
                if (param_1 == (int *)aiStack_304[local_308]) break;
                local_308 = local_308 + 1;
              }
              local_310 = (undefined4 *)(**(code **)(*(int *)aiStack_304[local_308] + 0xa4))(0,0);
              local_318 = (int *)(**(code **)(**(int **)(local_30c + 0x28) + 0x48))(0x151);
              local_314 = (**(code **)(*local_318 + 0xb4))
                                    ((int)*(char *)(DAT_013b71e8 + local_308 + 0xd48) % 9,
                                     (int)*(char *)(DAT_013b71e8 + local_308 + 0xd48) / 9);
              *(undefined4 *)(local_314 + 0x94) = 0xffffffff;
              *(undefined1 *)(DAT_013b71e8 + local_308 + 0xd48) = 0xff;
              _memset((void *)(DAT_013b71e8 + 0xd08 + local_308 * 8),0,8);
              if ((DAT_005ccec0[0x7a] != 0) && ((undefined4 *)DAT_005ccec0[0x7a] == local_310)) {
                DAT_005ccec0[0x7a] = 0;
              }
              if (local_310 == (undefined4 *)0x0) {
                ExceptionList = local_10;
                return 1;
              }
              if (local_310 == (undefined4 *)0x0) {
                ExceptionList = local_10;
                return 1;
              }
              (**(code **)*local_310)(1);
              ExceptionList = local_10;
              return 1;
            }
            if (param_1[0x100] == 0xe) {
              ExceptionList = &local_10;
              local_31c = (**(code **)(*param_1 + 0xb4))(param_2,param_3);
              if (local_31c == 0) {
                ExceptionList = local_10;
                return 1;
              }
              local_358 = DAT_0067cf38;
              for (local_354 = 0; local_354 < 0xd; local_354 = local_354 + 1) {
                iVar4 = (**(code **)(**(int **)(local_358 + 0x28) + 0x48))(local_354 + 0x17e0);
                aiStack_350[local_354] = iVar4;
              }
              local_354 = 0;
              while( true ) {
                if (0xc < local_354) {
                  ExceptionList = local_10;
                  return 1;
                }
                if (param_1 == (int *)aiStack_350[local_354]) break;
                local_354 = local_354 + 1;
              }
              local_35c = (undefined4 *)(**(code **)(*(int *)aiStack_350[local_354] + 0xa4))(0,0);
              local_364 = (int *)(**(code **)(**(int **)(local_358 + 0x28) + 0x48))(0x151);
              local_360 = (**(code **)(*local_364 + 0xb4))
                                    ((int)*(char *)(DAT_013b71e8 + local_354 + 0xd9c) % 9,
                                     (int)*(char *)(DAT_013b71e8 + local_354 + 0xd9c) / 9);
              *(undefined4 *)(local_360 + 0x94) = 0xffffffff;
              *(undefined1 *)(DAT_013b71e8 + local_354 + 0xd9c) = 0xff;
              _memset((void *)(DAT_013b71e8 + 0xd5c + local_354 * 8),0,8);
              if ((DAT_005ccec0[0x7a] != 0) && ((undefined4 *)DAT_005ccec0[0x7a] == local_35c)) {
                DAT_005ccec0[0x7a] = 0;
              }
              if (local_35c == (undefined4 *)0x0) {
                ExceptionList = local_10;
                return 1;
              }
              if (local_35c == (undefined4 *)0x0) {
                ExceptionList = local_10;
                return 1;
              }
              (**(code **)*local_35c)(1);
              ExceptionList = local_10;
              return 1;
            }
            if (param_1[0x100] == 0x10) {
              ExceptionList = &local_10;
              local_368 = (**(code **)(*param_1 + 0xb4))(param_2,param_3);
              if (local_368 == 0) {
                ExceptionList = local_10;
                return 1;
              }
              local_3a4 = DAT_0067cf38;
              for (local_3a0 = 0; local_3a0 < 0xd; local_3a0 = local_3a0 + 1) {
                iVar4 = (**(code **)(**(int **)(local_3a4 + 0x28) + 0x48))(local_3a0 + 0x1803);
                aiStack_39c[local_3a0] = iVar4;
              }
              local_3a0 = 0;
              while( true ) {
                if (0xc < local_3a0) {
                  ExceptionList = local_10;
                  return 1;
                }
                if (param_1 == (int *)aiStack_39c[local_3a0]) break;
                local_3a0 = local_3a0 + 1;
              }
              local_3a8 = (undefined4 *)(**(code **)(*(int *)aiStack_39c[local_3a0] + 0xa4))(0,0);
              local_3b0 = (int *)(**(code **)(**(int **)(local_3a4 + 0x28) + 0x48))(0x151);
              local_3ac = (**(code **)(*local_3b0 + 0xb4))
                                    ((int)*(char *)(DAT_013b71e8 + local_3a0 + 0xdf0) % 9,
                                     (int)*(char *)(DAT_013b71e8 + local_3a0 + 0xdf0) / 9);
              *(undefined4 *)(local_3ac + 0x94) = 0xffffffff;
              *(undefined1 *)(DAT_013b71e8 + local_3a0 + 0xdf0) = 0xff;
              _memset((void *)(DAT_013b71e8 + 0xdb0 + local_3a0 * 8),0,8);
              if ((DAT_005ccec0[0x7a] != 0) && ((undefined4 *)DAT_005ccec0[0x7a] == local_3a8)) {
                DAT_005ccec0[0x7a] = 0;
              }
              if (local_3a8 == (undefined4 *)0x0) {
                ExceptionList = local_10;
                return 1;
              }
              if (local_3a8 == (undefined4 *)0x0) {
                ExceptionList = local_10;
                return 1;
              }
              (**(code **)*local_3a8)(1);
              ExceptionList = local_10;
              return 1;
            }
            if (param_1[0x100] == 0x12) {
              ExceptionList = &local_10;
              aiStack_3c0[3] = (**(code **)(*param_1 + 0xb4))(param_2,param_3);
              if (aiStack_3c0[3] == 0) {
                ExceptionList = local_10;
                return 1;
              }
              local_3c8 = DAT_0067cf38;
              for (local_3c4 = 0; local_3c4 < 3; local_3c4 = local_3c4 + 1) {
                iVar4 = (**(code **)(**(int **)(local_3c8 + 0x28) + 0x48))(local_3c4 + 0x1924);
                aiStack_3c0[local_3c4] = iVar4;
              }
              local_3c4 = 0;
              while( true ) {
                if (2 < local_3c4) {
                  ExceptionList = local_10;
                  return 1;
                }
                if (param_1 == (int *)aiStack_3c0[local_3c4]) break;
                local_3c4 = local_3c4 + 1;
              }
              local_3cc = (undefined4 *)(**(code **)(*(int *)aiStack_3c0[local_3c4] + 0xa4))(0,0);
              local_3d4 = (int *)(**(code **)(**(int **)(local_3c8 + 0x28) + 0x48))(0x151);
              local_3d0 = (**(code **)(*local_3d4 + 0xb4))
                                    ((int)*(char *)(DAT_013b71e8 + local_3c4 + 0xe44) % 9,
                                     (int)*(char *)(DAT_013b71e8 + local_3c4 + 0xe44) / 9);
              *(undefined4 *)(local_3d0 + 0x94) = 0xffffffff;
              *(undefined1 *)(DAT_013b71e8 + local_3c4 + 0xe44) = 0xff;
              _memset((void *)(DAT_013b71e8 + 0xe04 + local_3c4 * 8),0,8);
              if ((DAT_005ccec0[0x7a] != 0) && ((undefined4 *)DAT_005ccec0[0x7a] == local_3cc)) {
                DAT_005ccec0[0x7a] = 0;
              }
              if (local_3cc == (undefined4 *)0x0) {
                ExceptionList = local_10;
                return 1;
              }
              if (local_3cc == (undefined4 *)0x0) {
                ExceptionList = local_10;
                return 1;
              }
              (**(code **)*local_3cc)(1);
              ExceptionList = local_10;
              return 1;
            }
            if (param_1[0x100] == 0x14) {
              ExceptionList = &local_10;
              local_3d8 = (**(code **)(*param_1 + 0xb4))(param_2,param_3);
              if (local_3d8 == 0) {
                ExceptionList = local_10;
                return 1;
              }
              local_3fc = DAT_0067cf38;
              for (local_3f8 = 0; local_3f8 < 7; local_3f8 = local_3f8 + 1) {
                iVar4 = (**(code **)(**(int **)(local_3fc + 0x28) + 0x48))(local_3f8 + 0x1953);
                aiStack_3f4[local_3f8] = iVar4;
              }
              local_3f8 = 0;
              while( true ) {
                if (6 < local_3f8) {
                  ExceptionList = local_10;
                  return 1;
                }
                if (param_1 == (int *)aiStack_3f4[local_3f8]) break;
                local_3f8 = local_3f8 + 1;
              }
              local_400 = (undefined4 *)(**(code **)(*(int *)aiStack_3f4[local_3f8] + 0xa4))(0,0);
              local_408 = (int *)(**(code **)(**(int **)(local_3fc + 0x28) + 0x48))(0x151);
              local_404 = (**(code **)(*local_408 + 0xb4))
                                    ((int)*(char *)(DAT_013b71e8 + local_3f8 + 0xe98) % 9,
                                     (int)*(char *)(DAT_013b71e8 + local_3f8 + 0xe98) / 9);
              *(undefined4 *)(local_404 + 0x94) = 0xffffffff;
              *(undefined1 *)(DAT_013b71e8 + local_3f8 + 0xe98) = 0xff;
              _memset((void *)(DAT_013b71e8 + 0xe58 + local_3f8 * 8),0,8);
              if ((DAT_005ccec0[0x7a] != 0) && ((undefined4 *)DAT_005ccec0[0x7a] == local_400)) {
                DAT_005ccec0[0x7a] = 0;
              }
              if (local_400 == (undefined4 *)0x0) {
                ExceptionList = local_10;
                return 1;
              }
              if (local_400 == (undefined4 *)0x0) {
                ExceptionList = local_10;
                return 1;
              }
              (**(code **)*local_400)(1);
              ExceptionList = local_10;
              return 1;
            }
            if (param_1[0x100] == 0x16) {
              ExceptionList = &local_10;
              aiStack_418[3] = (**(code **)(*param_1 + 0xb4))(param_2,param_3);
              iVar4 = DAT_0067cf38;
              if (aiStack_418[3] == 0) {
                ExceptionList = local_10;
                return 1;
              }
              for (local_41c = 0; local_41c < 3; local_41c = local_41c + 1) {
                iVar7 = (**(code **)(**(int **)(iVar4 + 0x28) + 0x48))(local_41c + 0x1974);
                aiStack_418[local_41c] = iVar7;
              }
              local_41c = 0;
              while( true ) {
                if (2 < local_41c) {
                  ExceptionList = local_10;
                  return 1;
                }
                if (param_1 == (int *)aiStack_418[local_41c]) break;
                local_41c = local_41c + 1;
              }
              puVar8 = (undefined4 *)(**(code **)(*(int *)aiStack_418[local_41c] + 0xa4))(0,0);
              piVar6 = (int *)(**(code **)(**(int **)(iVar4 + 0x28) + 0x48))(0x151);
              iVar4 = (**(code **)(*piVar6 + 0xb4))
                                ((int)*(char *)(DAT_013b71e8 + local_41c + 0xeec) % 9,
                                 (int)*(char *)(DAT_013b71e8 + local_41c + 0xeec) / 9);
              *(undefined4 *)(iVar4 + 0x94) = 0xffffffff;
              *(undefined1 *)(DAT_013b71e8 + local_41c + 0xeec) = 0xff;
              _memset((void *)(DAT_013b71e8 + 0xeac + local_41c * 8),0,8);
              if ((DAT_005ccec0[0x7a] != 0) && ((undefined4 *)DAT_005ccec0[0x7a] == puVar8)) {
                DAT_005ccec0[0x7a] = 0;
              }
              if (puVar8 == (undefined4 *)0x0) {
                ExceptionList = local_10;
                return 1;
              }
              if (puVar8 == (undefined4 *)0x0) {
                ExceptionList = local_10;
                return 1;
              }
              (**(code **)*puVar8)(1);
              ExceptionList = local_10;
              return 1;
            }
            if (param_1[0x100] == 0xb) {
              ExceptionList = &local_10;
              iVar7 = (**(code **)(*param_1 + 0xb8))(0,0);
              iVar4 = DAT_0067cf38;
              if ((iVar7 != 0) && (*(int *)(DAT_0067cf38 + 0x24) == 30000)) {
                (**(code **)(**(int **)(DAT_0067cf38 + 0x8c) + 0x8c))(&DAT_00a38978,0x286,0);
                (**(code **)(**(int **)(iVar4 + 0x8c) + 0x60))(1);
                *(int *)(*(int *)(iVar4 + 0x8c) + 0x1e8) = param_1[0x11];
              }
              ExceptionList = local_10;
              return 1;
            }
            if (param_1[0x100] == 1) {
              return 2;
            }
            if (param_1[0x100] == 4) {
              return 2;
            }
            ExceptionList = &local_10;
            iVar4 = (**(code **)(*param_1 + 0xac))(param_2,param_3);
            if ((iVar4 != 0) &&
               (iVar7 = FUN_0040db40(**(short **)(iVar4 + 0x670) + -5000), iVar7 == 1)) {
              ExceptionList = local_10;
              return 1;
            }
            if ((iVar4 != 0) && (*(int *)(iVar4 + 0x94) != -0x10000)) {
              (**(code **)(*DAT_005ccec0 + 0x94))(iVar4);
            }
            ExceptionList = local_10;
            return 1;
          }
          ExceptionList = &local_10;
          aiStack_2ac[4] = (**(code **)(*param_1 + 0xb4))(param_2,param_3);
          if (aiStack_2ac[4] == 0) {
            ExceptionList = local_10;
            return 2;
          }
          if (*(int *)(aiStack_2ac[4] + 0x94) != -1) {
            ExceptionList = local_10;
            return 2;
          }
          local_2b0 = (short)*(undefined4 *)(aiStack_2ac[4] + 0x1d0) +
                      (short)*(undefined4 *)(aiStack_2ac[4] + 0x1d4) * 9;
          local_2b8 = DAT_0067cf38;
          for (local_2b4 = 0; local_2b4 < 3; local_2b4 = local_2b4 + 1) {
            iVar4 = (**(code **)(**(int **)(local_2b8 + 0x28) + 0x48))(local_2b4 + 0x1974);
            aiStack_2ac[local_2b4] = iVar4;
          }
          aiStack_2ac[3] = 0;
          local_2b4 = 0;
          do {
            if (2 < local_2b4) {
LAB_0041514d:
              if (aiStack_2ac[3] != 0) {
                ExceptionList = local_10;
                return 1;
              }
              ExceptionList = local_10;
              return 0;
            }
            if ((aiStack_2ac[local_2b4] != 0) &&
               (iVar4 = (**(code **)(*(int *)aiStack_2ac[local_2b4] + 0xb4))(0,0), iVar4 == 0)) {
              local_2c4 = 0;
              if ((local_2b4 == 0) || (local_2b4 == 1)) {
                if (**(short **)(aiStack_2ac[4] + 0x670) != 0x2b9) {
                  FUN_00403df2(&DAT_00a3ca78,2000);
                  (**(code **)(**(int **)(local_2b8 + 0x84) + 0x88))(1,1);
                  ExceptionList = local_10;
                  return 1;
                }
                local_2bc = 0;
              }
              local_2c4 = 0;
              if (local_2b4 == 2) {
                local_2cc = FUN_0054e06c(*(undefined4 *)(aiStack_2ac[4] + 0x670));
                if ((local_2cc < 9) ||
                   (*(short *)(&DAT_00d449d6 + **(short **)(aiStack_2ac[4] + 0x670) * 0x8c) == 0)) {
                  FUN_00403df2(&DAT_00a3a178,2000);
                  (**(code **)(**(int **)(local_2b8 + 0x84) + 0x88))(1,1);
                  ExceptionList = local_10;
                  return 1;
                }
                local_2bc = 0x41500000;
                local_2c4 = 0x41e00000;
              }
              local_2c0 = operator_new(8);
              FUN_0058f220(local_2c0,*(undefined4 *)(aiStack_2ac[4] + 0x670),8);
              pvVar5 = operator_new(0x678);
              local_8 = 0xd;
              if (pvVar5 == (void *)0x0) {
                local_578 = 0;
              }
              else {
                local_578 = FUN_0040d13e(0,local_2c0,local_2bc,local_2c4);
              }
              local_8 = 0xffffffff;
              local_2c8 = local_578;
              (**(code **)(*(int *)aiStack_2ac[local_2b4] + 0x8c))(local_578,0,0);
              FUN_0058f220(DAT_013b71e8 + 0xeac + local_2b4 * 8,
                           *(undefined4 *)(aiStack_2ac[4] + 0x670),8);
              *(undefined1 *)(DAT_013b71e8 + local_2b4 + 0xeec) = (undefined1)local_2b0;
              *(undefined4 *)(aiStack_2ac[4] + 0x94) = 0xffff0000;
              aiStack_2ac[3] = 1;
              goto LAB_0041514d;
            }
            local_2b4 = local_2b4 + 1;
          } while( true );
        }
        ExceptionList = &local_10;
        local_250 = (**(code **)(*param_1 + 0xb4))(param_2,param_3);
        if (local_250 == 0) {
          ExceptionList = local_10;
          return 2;
        }
        if (*(int *)(local_250 + 0x94) != -1) {
          ExceptionList = local_10;
          return 2;
        }
        local_274 = (short)*(undefined4 *)(local_250 + 0x1d0) +
                    (short)*(undefined4 *)(local_250 + 0x1d4) * 9;
        local_27c = DAT_0067cf38;
        for (local_278 = 0; local_278 < 7; local_278 = local_278 + 1) {
          iVar4 = (**(code **)(**(int **)(local_27c + 0x28) + 0x48))(local_278 + 0x1953);
          aiStack_270[local_278] = iVar4;
        }
        local_254 = 0;
        if (DAT_005ccf04 == 0) {
          for (local_278 = 0; local_278 < 7; local_278 = local_278 + 1) {
            if ((aiStack_270[local_278] != 0) &&
               (iVar4 = (**(code **)(*(int *)aiStack_270[local_278] + 0xb4))(0,0), iVar4 == 0)) {
              if (((local_278 == 0) || (local_278 == 1)) &&
                 ((**(short **)(local_250 + 0x670) != 0 &&
                  (**(short **)(local_250 + 0x670) != 0xd78)))) {
                FUN_00403df2(&DAT_00a3df78,2000);
                (**(code **)(**(int **)(local_27c + 0x84) + 0x88))(1,1);
                ExceptionList = local_10;
                return 1;
              }
              if (((local_278 == 2) && (**(short **)(local_250 + 0x670) != 0)) &&
                 (**(short **)(local_250 + 0x670) != 0x101f)) {
                FUN_00403df2(&DAT_00a3dff8,2000);
                (**(code **)(**(int **)(local_27c + 0x84) + 0x88))(1,1);
                ExceptionList = local_10;
                return 1;
              }
              if (((2 < local_278) && (**(short **)(local_250 + 0x670) != 0)) &&
                 ((**(short **)(local_250 + 0x670) != 0x19d &&
                  (**(short **)(local_250 + 0x670) != 0x102d)))) {
                FUN_00403df2(&DAT_00a3e078,2000);
                (**(code **)(**(int **)(local_27c + 0x84) + 0x88))(1,1);
                ExceptionList = local_10;
                return 1;
              }
              local_280 = operator_new(8);
              FUN_0058f220(local_280,*(undefined4 *)(local_250 + 0x670),8);
              pvVar5 = operator_new(0x678);
              local_8 = 10;
              if (pvVar5 == (void *)0x0) {
                local_56c = 0;
              }
              else {
                local_56c = FUN_0040d13e(0,local_280,0,0);
              }
              local_8 = 0xffffffff;
              local_284 = local_56c;
              (**(code **)(*(int *)aiStack_270[local_278] + 0x8c))(local_56c,0,0);
              FUN_0058f220(DAT_013b71e8 + 0xe58 + local_278 * 8,*(undefined4 *)(local_250 + 0x670),8
                          );
              *(undefined1 *)(DAT_013b71e8 + local_278 + 0xe98) = (undefined1)local_274;
              *(undefined4 *)(local_250 + 0x94) = 0xffff0000;
              local_254 = 1;
              break;
            }
          }
        }
        else if (DAT_005ccf04 == 1) {
          for (local_278 = 0; local_278 < 7; local_278 = local_278 + 1) {
            if ((aiStack_270[local_278] != 0) &&
               (iVar4 = (**(code **)(*(int *)aiStack_270[local_278] + 0xb4))(0,0), iVar4 == 0)) {
              if (((**(short **)(local_250 + 0x670) != 0) &&
                  ((((**(short **)(local_250 + 0x670) != 0x21c &&
                     (**(short **)(local_250 + 0x670) != 0x21d)) &&
                    (**(short **)(local_250 + 0x670) != 0x277)) &&
                   ((**(short **)(local_250 + 0x670) != 0x278 &&
                    (**(short **)(local_250 + 0x670) != 0x279)))))) && (local_278 < 3)) {
                FUN_00403df2(&DAT_00a3e178,2000);
                (**(code **)(**(int **)(local_27c + 0x84) + 0x88))(1,1);
                ExceptionList = local_10;
                return 1;
              }
              if (((local_278 == 0) || (local_278 == 1)) &&
                 ((**(short **)(local_250 + 0x670) != 0 &&
                  (local_290 = FUN_0054e06c(*(undefined4 *)(local_250 + 0x670)), local_290 < 9)))) {
                FUN_00403df2(&DAT_00a3a178,2000);
                (**(code **)(**(int **)(local_27c + 0x84) + 0x88))(1,1);
                ExceptionList = local_10;
                return 1;
              }
              if ((((2 < local_278) && (**(short **)(local_250 + 0x670) != 0)) &&
                  (**(short **)(local_250 + 0x670) != 0x19d)) &&
                 (**(short **)(local_250 + 0x670) != 0x102d)) {
                FUN_00403df2(&DAT_00a3e078,2000);
                (**(code **)(**(int **)(local_27c + 0x84) + 0x88))(1,1);
                ExceptionList = local_10;
                return 1;
              }
              local_288 = operator_new(8);
              FUN_0058f220(local_288,*(undefined4 *)(local_250 + 0x670),8);
              pvVar5 = operator_new(0x678);
              local_8 = 0xb;
              if (pvVar5 == (void *)0x0) {
                local_570 = 0;
              }
              else {
                local_570 = FUN_0040d13e(0,local_288,0,0);
              }
              local_8 = 0xffffffff;
              local_28c = local_570;
              (**(code **)(*(int *)aiStack_270[local_278] + 0x8c))(local_570,0,0);
              FUN_0058f220(DAT_013b71e8 + 0xe58 + local_278 * 8,*(undefined4 *)(local_250 + 0x670),8
                          );
              *(undefined1 *)(DAT_013b71e8 + local_278 + 0xe98) = (undefined1)local_274;
              *(undefined4 *)(local_250 + 0x94) = 0xffff0000;
              local_254 = 1;
              break;
            }
          }
        }
        else if (DAT_005ccf04 == 2) {
          for (local_278 = 0; local_278 < 7; local_278 = local_278 + 1) {
            if ((aiStack_270[local_278] != 0) &&
               (iVar4 = (**(code **)(*(int *)aiStack_270[local_278] + 0xb4))(0,0), iVar4 == 0)) {
              if (local_278 == 0) {
                if ((((**(short **)(local_250 + 0x670) != 0x19d) &&
                     (**(short **)(local_250 + 0x670) != 0x101f)) &&
                    ((**(short **)(local_250 + 0x670) < 0x13f6 ||
                     (0x140d < **(short **)(local_250 + 0x670))))) &&
                   ((**(short **)(local_250 + 0x670) != 0x1a5 &&
                    (**(short **)(local_250 + 0x670) != 0x1032)))) {
                  FUN_00403df2(&DAT_00a3ca78,2000);
                  (**(code **)(**(int **)(local_27c + 0x84) + 0x88))(1,1);
                  ExceptionList = local_10;
                  return 1;
                }
              }
              else if (*(short *)(DAT_013b71e8 + 0xe58) == 0x19d) {
                if (**(short **)(local_250 + 0x670) != 0x19d) {
                  FUN_00403df2(&DAT_00a3ca78,2000);
                  (**(code **)(**(int **)(local_27c + 0x84) + 0x88))(1,1);
                  ExceptionList = local_10;
                  return 1;
                }
              }
              else if (*(short *)(DAT_013b71e8 + 0xe58) == 0x101f) {
                if (((local_278 == 0) || (local_278 == 1)) &&
                   (**(short **)(local_250 + 0x670) != 0x101f)) {
                  FUN_00403df2(&DAT_00a3ca78,2000);
                  (**(code **)(**(int **)(local_27c + 0x84) + 0x88))(1,1);
                  ExceptionList = local_10;
                  return 1;
                }
                if ((local_278 == 2) && (**(short **)(local_250 + 0x670) != 0x140f)) {
                  FUN_00403df2(&DAT_00a3ca78,2000);
                  (**(code **)(**(int **)(local_27c + 0x84) + 0x88))(1,1);
                  ExceptionList = local_10;
                  return 1;
                }
                if ((2 < local_278) && (**(short **)(local_250 + 0x670) != 0x19d)) {
                  FUN_00403df2(&DAT_00a3ca78,2000);
                  (**(code **)(**(int **)(local_27c + 0x84) + 0x88))(1,1);
                  ExceptionList = local_10;
                  return 1;
                }
              }
              else if ((*(short *)(DAT_013b71e8 + 0xe58) < 0x13f6) ||
                      (0x140d < *(short *)(DAT_013b71e8 + 0xe58))) {
                if (*(short *)(DAT_013b71e8 + 0xe58) == 0x1a5) {
                  if (**(short **)(local_250 + 0x670) + -0x1a5 != local_278) {
                    FUN_00403df2(&DAT_00a3ca78,2000);
                    (**(code **)(**(int **)(local_27c + 0x84) + 0x88))(1,1);
                    ExceptionList = local_10;
                    return 1;
                  }
                }
                else {
                  if (*(short *)(DAT_013b71e8 + 0xe58) != 0x1032) {
                    FUN_00403df2(&DAT_00a3ca78,2000);
                    (**(code **)(**(int **)(local_27c + 0x84) + 0x88))(1,1);
                    ExceptionList = local_10;
                    return 1;
                  }
                  if ((local_278 == 1) && (**(short **)(local_250 + 0x670) != 0x1032)) {
                    FUN_00403df2(&DAT_00a3ca78,2000);
                    (**(code **)(**(int **)(local_27c + 0x84) + 0x88))(1,1);
                    ExceptionList = local_10;
                    return 1;
                  }
                  if ((local_278 == 2) && (**(short **)(local_250 + 0x670) != 0x140f)) {
                    FUN_00403df2(&DAT_00a3ca78,2000);
                    (**(code **)(**(int **)(local_27c + 0x84) + 0x88))(1,1);
                    ExceptionList = local_10;
                    return 1;
                  }
                  if ((2 < local_278) &&
                     ((**(short **)(local_250 + 0x670) < 0x13f6 ||
                      (0x140d < **(short **)(local_250 + 0x670))))) {
                    FUN_00403df2(&DAT_00a3ca78,2000);
                    (**(code **)(**(int **)(local_27c + 0x84) + 0x88))(1,1);
                    ExceptionList = local_10;
                    return 1;
                  }
                }
              }
              else if ((**(short **)(local_250 + 0x670) < 0x13f6) ||
                      (0x140d < **(short **)(local_250 + 0x670))) {
                FUN_00403df2(&DAT_00a3ca78,2000);
                (**(code **)(**(int **)(local_27c + 0x84) + 0x88))(1,1);
                ExceptionList = local_10;
                return 1;
              }
              local_294 = operator_new(8);
              FUN_0058f220(local_294,*(undefined4 *)(local_250 + 0x670),8);
              pvVar5 = operator_new(0x678);
              local_8 = 0xc;
              if (pvVar5 == (void *)0x0) {
                local_574 = 0;
              }
              else {
                local_574 = FUN_0040d13e(0,local_294,0,0);
              }
              local_8 = 0xffffffff;
              local_298 = local_574;
              (**(code **)(*(int *)aiStack_270[local_278] + 0x8c))(local_574,0,0);
              FUN_0058f220(DAT_013b71e8 + 0xe58 + local_278 * 8,*(undefined4 *)(local_250 + 0x670),8
                          );
              *(undefined1 *)(DAT_013b71e8 + local_278 + 0xe98) = (undefined1)local_274;
              *(undefined4 *)(local_250 + 0x94) = 0xffff0000;
              local_254 = 1;
              break;
            }
          }
        }
        if (local_254 != 0) {
          ExceptionList = local_10;
          return 1;
        }
        ExceptionList = local_10;
        return 0;
      }
      ExceptionList = &local_10;
      local_1d4 = (**(code **)(*param_1 + 0xb4))(param_2,param_3);
      if (local_1d4 == 0) {
        ExceptionList = local_10;
        return 2;
      }
      if (*(int *)(local_1d4 + 0x94) != -1) {
        ExceptionList = local_10;
        return 2;
      }
      local_1f0 = (short)*(undefined4 *)(local_1d4 + 0x1d0) +
                  (short)*(undefined4 *)(local_1d4 + 0x1d4) * 9;
      local_1f8 = DAT_0067cf38;
      for (local_1f4 = 0; local_1f4 < 3; local_1f4 = local_1f4 + 1) {
        piVar6 = (int *)(**(code **)(**(int **)(local_1f8 + 0x28) + 0x48))(local_1f4 + 0x1924);
        local_1ec[local_1f4] = piVar6;
      }
      local_1ec[3] = (int *)0x0;
      local_1d8 = 0;
      local_1f4 = 0;
      do {
        if (2 < local_1f4) {
LAB_00413e08:
          if (local_1d8 != 0) {
            ExceptionList = local_10;
            return 1;
          }
          ExceptionList = local_10;
          return 0;
        }
        if ((local_1ec[local_1f4] != (int *)0x0) &&
           (iVar4 = (**(code **)(*local_1ec[local_1f4] + 0xb4))(0,0), iVar4 == 0)) {
          if (local_1f4 == 0) {
            local_204 = FUN_0054cd07(*(undefined4 *)(local_1d4 + 0x670),0x70);
            local_210 = (uint)*(short *)(&DAT_00d449d6 + **(short **)(local_1d4 + 0x670) * 0x8c);
            local_20c = FUN_0054cd07(*(undefined4 *)(local_1d4 + 0x670),0x12);
            local_214 = FUN_0054cd07(*(undefined4 *)(local_1d4 + 0x670),0x57);
            local_208 = (int)*(short *)(&DAT_00d449d4 + **(short **)(local_1d4 + 0x670) * 0x8c);
            local_218 = (uint)(local_204 == 1);
            if (local_218 == 0) {
              FUN_00403df2(&DAT_00a3c9f8,2000);
              (**(code **)(**(int **)(local_1f8 + 0x84) + 0x88))(1,1);
              ExceptionList = local_10;
              return 1;
            }
            if (local_214 != 6) {
              local_218 = 0;
            }
            if (local_218 == 0) {
              FUN_00403df2(&DAT_00a3d878,2000);
              (**(code **)(**(int **)(local_1f8 + 0x84) + 0x88))(1,1);
              ExceptionList = local_10;
              return 1;
            }
            if ((local_210 & 0x3f) != 0) {
              FUN_00403df2(&DAT_00a3cb78,2000);
              (**(code **)(**(int **)(local_1f8 + 0x84) + 0x88))(1,1);
              ExceptionList = local_10;
              return 1;
            }
          }
          if (local_1f4 == 1) {
            local_224 = FUN_0054cd07(*(undefined4 *)(local_1d4 + 0x670),0x70);
            local_220 = (int)*(short *)(&DAT_00d449d4 + **(short **)(local_1d4 + 0x670) * 0x8c) / 10
            ;
            local_21c = (int)*(short *)(&DAT_00d449d6 + **(short **)(local_1d4 + 0x670) * 0x8c);
            local_244 = FUN_0054cd07(*(undefined4 *)(local_1d4 + 0x670),0x12);
            local_238 = (int)*(short *)(&DAT_00d449d4 + **(short **)(local_1d4 + 0x670) * 0x8c);
            local_240 = FUN_0054cd07(*(undefined4 *)(local_1d4 + 0x670),0x57);
            local_248 = (uint)(3 < local_220);
            if (local_248 == 0) {
              FUN_00403df2(&DAT_00a3cb78,2000);
              (**(code **)(**(int **)(local_1f8 + 0x84) + 0x88))(1,1);
              ExceptionList = local_10;
              return 1;
            }
            if (local_240 != 6) {
              local_248 = 0;
            }
            if (local_248 == 0) {
              FUN_00403df2(&DAT_00a3d878,2000);
              (**(code **)(**(int **)(local_1f8 + 0x84) + 0x88))(1,1);
              ExceptionList = local_10;
              return 1;
            }
            local_234 = (**(code **)(*local_1ec[0] + 0xb4))(0,0);
            local_23c = (int)*(short *)(&DAT_00d449d4 + **(short **)(local_234 + 0x670) * 0x8c) % 10
            ;
            local_230 = (int)*(short *)(&DAT_00d449d6 + **(short **)(local_234 + 0x670) * 0x8c);
            local_22c = FUN_0054cd07(*(undefined4 *)(local_234 + 0x670),0x12);
            local_228 = (int)*(short *)(&DAT_00d449d4 + **(short **)(local_234 + 0x670) * 0x8c);
            if (local_228 != local_238) {
              if (((local_228 == 0x2d) && (local_238 == 0x30)) && (local_230 == local_21c)) {
                local_248 = 1;
              }
              else {
                local_248 = 0;
              }
            }
            if (local_230 != local_21c) {
              local_248 = 0;
            }
            if (local_248 == 0) {
              FUN_00403df2(&DAT_00a3cb78,2000);
              (**(code **)(**(int **)(local_1f8 + 0x84) + 0x88))(1,1);
              ExceptionList = local_10;
              return 1;
            }
          }
          if (local_1f4 == 2) {
            local_1ec[4] = (int *)0x41500000;
            local_1ec[3] = (int *)0x41e00000;
          }
          if ((((local_1f4 == 0) || (local_1f4 == 1)) || (local_1f4 == 2)) &&
             ((local_24c = FUN_0054e06c(*(undefined4 *)(local_1d4 + 0x670)), local_24c < 9 ||
              (*(short *)(&DAT_00d449d6 + **(short **)(local_1d4 + 0x670) * 0x8c) == 0)))) {
            FUN_00403df2(&DAT_00a3a178,2000);
            (**(code **)(**(int **)(local_1f8 + 0x84) + 0x88))(1,1);
            ExceptionList = local_10;
            return 1;
          }
          local_1fc = operator_new(8);
          FUN_0058f220(local_1fc,*(undefined4 *)(local_1d4 + 0x670),8);
          pvVar5 = operator_new(0x678);
          local_8 = 9;
          if (pvVar5 == (void *)0x0) {
            local_568 = 0;
          }
          else {
            local_568 = FUN_0040d13e(0,local_1fc,local_1ec[4],local_1ec[3]);
          }
          local_8 = 0xffffffff;
          local_200 = local_568;
          (**(code **)(*local_1ec[local_1f4] + 0x8c))(local_568,0,0);
          FUN_0058f220(DAT_013b71e8 + 0xe04 + local_1f4 * 8,*(undefined4 *)(local_1d4 + 0x670),8);
          *(undefined1 *)(DAT_013b71e8 + local_1f4 + 0xe44) = (undefined1)local_1f0;
          *(undefined4 *)(local_1d4 + 0x94) = 0xffff0000;
          local_1d8 = 1;
          goto LAB_00413e08;
        }
        local_1f4 = local_1f4 + 1;
      } while( true );
    }
    ExceptionList = &local_10;
    local_168 = (**(code **)(*param_1 + 0xb4))(param_2,param_3);
    if (local_168 == 0) {
      ExceptionList = local_10;
      return 2;
    }
    if (*(int *)(local_168 + 0x94) != -1) {
      ExceptionList = local_10;
      return 2;
    }
    local_188 = (short)*(undefined4 *)(local_168 + 0x1d0) +
                (short)*(undefined4 *)(local_168 + 0x1d4) * 9;
    local_190 = DAT_0067cf38;
    for (local_18c = 0; local_18c < 6; local_18c = local_18c + 1) {
      piVar6 = (int *)(**(code **)(**(int **)(local_190 + 0x28) + 0x48))(local_18c + 0x1803);
      local_184[local_18c] = piVar6;
    }
    local_16c = 0;
    local_18c = 0;
    do {
      if (5 < local_18c) {
LAB_004135b8:
        if (local_16c != 0) {
          ExceptionList = local_10;
          return 1;
        }
        ExceptionList = local_10;
        return 0;
      }
      if ((local_184[local_18c] != (int *)0x0) &&
         (iVar4 = (**(code **)(*local_184[local_18c] + 0xb4))(0,0), iVar4 == 0)) {
        if (local_18c == 0) {
          local_1a4 = (int)*(short *)(&DAT_00d449d4 + **(short **)(local_168 + 0x670) * 0x8c) % 10;
          local_1a0 = (uint)*(short *)(&DAT_00d449d6 + **(short **)(local_168 + 0x670) * 0x8c);
          local_19c = FUN_0054cd07(*(undefined4 *)(local_168 + 0x670),0x12);
          local_1a8 = 1;
          if ((local_19c == 1) && (local_1a4 != 9)) {
            local_1a8 = 0;
          }
          if ((local_19c != 1) && (local_1a4 != 9)) {
            local_1a8 = 0;
          }
          if (local_1a8 == 0) {
            FUN_00403df2(&DAT_00a3c9f8,2000);
            (**(code **)(**(int **)(local_190 + 0x84) + 0x88))(1,1);
            ExceptionList = local_10;
            return 1;
          }
          if ((local_1a0 & 0x3f) == 0) {
            FUN_00403df2(&DAT_00a3cb78,2000);
            (**(code **)(**(int **)(local_190 + 0x84) + 0x88))(1,1);
            ExceptionList = local_10;
            return 1;
          }
          FUN_00403df2(&DAT_00a3da78,2000);
          (**(code **)(**(int **)(local_190 + 0x84) + 0x88))(1,1);
        }
        if (local_18c == 1) {
          local_1b0 = (int)*(short *)(&DAT_00d449d4 + **(short **)(local_168 + 0x670) * 0x8c) % 10;
          local_1ac = (int)*(short *)(&DAT_00d449d6 + **(short **)(local_168 + 0x670) * 0x8c);
          local_1c4 = FUN_0054cd07(*(undefined4 *)(local_168 + 0x670),0x12);
          local_1cc = (int)*(short *)(&DAT_00d449d4 + **(short **)(local_168 + 0x670) * 0x8c);
          local_1c8 = 1;
          if (local_1cc == 0) {
            local_1c8 = 0;
          }
          else {
            if (((local_1c4 == 1) && (local_1b0 < 7)) && (local_1b0 != 0)) {
              local_1c8 = 0;
            }
            if (((local_1c4 != 1) && (local_1b0 < 6)) && (local_1b0 != 0)) {
              local_1c8 = 0;
            }
          }
          if (local_1c8 == 0) {
            FUN_00403df2(&DAT_00a3caf8,2000);
            (**(code **)(**(int **)(local_190 + 0x84) + 0x88))(1,1);
            ExceptionList = local_10;
            return 1;
          }
          local_1bc = (**(code **)(*local_184[0] + 0xb4))(0,0);
          local_1c0 = (int)*(short *)(&DAT_00d449d4 + **(short **)(local_1bc + 0x670) * 0x8c) % 10;
          local_1b8 = (int)*(short *)(&DAT_00d449d6 + **(short **)(local_1bc + 0x670) * 0x8c);
          local_1b4 = FUN_0054cd07(*(undefined4 *)(local_1bc + 0x670),0x12);
          if (local_1b4 != local_1c4) {
            local_1c8 = 0;
          }
          if (local_1b8 != local_1ac) {
            local_1c8 = 0;
          }
          if (local_1c8 == 0) {
            FUN_00403df2(&DAT_00a3cb78,2000);
            (**(code **)(**(int **)(local_190 + 0x84) + 0x88))(1,1);
            ExceptionList = local_10;
            return 1;
          }
        }
        if ((local_18c == 0) || (local_18c == 1)) {
          local_1d0 = FUN_0054e06c(*(undefined4 *)(local_168 + 0x670));
          if ((local_1d0 < 9) ||
             (*(short *)(&DAT_00d449d6 + **(short **)(local_168 + 0x670) * 0x8c) == 0)) {
            FUN_00403df2(&DAT_00a3a178,2000);
            (**(code **)(**(int **)(local_190 + 0x84) + 0x88))(1,1);
            ExceptionList = local_10;
            return 1;
          }
        }
        else if (((1 < local_18c) && (local_18c < 6)) && (**(short **)(local_168 + 0x670) != 0xc44))
        {
          FUN_00403df2(&DAT_00a3c978,2000);
          (**(code **)(**(int **)(local_190 + 0x84) + 0x88))(1,1);
          ExceptionList = local_10;
          return 1;
        }
        local_194 = operator_new(8);
        FUN_0058f220(local_194,*(undefined4 *)(local_168 + 0x670),8);
        pvVar5 = operator_new(0x678);
        local_8 = 8;
        if (pvVar5 == (void *)0x0) {
          local_564 = 0;
        }
        else {
          local_564 = FUN_0040d13e(0,local_194,0,0);
        }
        local_8 = 0xffffffff;
        local_198 = local_564;
        (**(code **)(*local_184[local_18c] + 0x8c))(local_564,0,0);
        FUN_0058f220(DAT_013b71e8 + 0xdb0 + local_18c * 8,*(undefined4 *)(local_168 + 0x670),8);
        *(undefined1 *)(DAT_013b71e8 + local_18c + 0xdf0) = (undefined1)local_188;
        *(undefined4 *)(local_168 + 0x94) = 0xffff0000;
        local_16c = 1;
        goto LAB_004135b8;
      }
      local_18c = local_18c + 1;
    } while( true );
  }
  ExceptionList = &local_10;
  local_e0 = (**(code **)(*param_1 + 0xb4))(param_2,param_3);
  if (local_e0 == 0) {
    ExceptionList = local_10;
    return 2;
  }
  if (*(int *)(local_e0 + 0x94) != -1) {
    ExceptionList = local_10;
    return 2;
  }
  local_108 = (short)*(undefined4 *)(local_e0 + 0x1d0) +
              (short)*(undefined4 *)(local_e0 + 0x1d4) * 9;
  local_110 = DAT_0067cf38;
  for (local_10c = 0; local_10c < 7; local_10c = local_10c + 1) {
    piVar6 = (int *)(**(code **)(**(int **)(local_110 + 0x28) + 0x48))(local_10c + 0x17e0);
    local_104[local_10c] = piVar6;
  }
  iVar4 = (**(code **)(*local_ec + 0xb8))(0,0);
  if (iVar4 != 0) {
    ExceptionList = local_10;
    return 1;
  }
  for (local_10c = 0; local_10c < 4; local_10c = local_10c + 1) {
    local_114 = (undefined4 *)0x0;
    local_114 = (undefined4 *)
                (**(code **)(**(int **)(local_110 + 0x27c60 + local_10c * 4) + 0xa4))(0,0);
    if (local_114 != (undefined4 *)0x0) {
      if (local_114 != (undefined4 *)0x0) {
        (**(code **)*local_114)(1);
      }
      local_114 = (undefined4 *)0x0;
    }
  }
  local_e4 = 0;
  for (local_10c = 0; local_10c < 8; local_10c = local_10c + 1) {
    if ((local_104[local_10c] != (int *)0x0) &&
       (iVar4 = (**(code **)(*local_104[local_10c] + 0xb4))(0,0), iVar4 == 0)) {
      if ((local_10c == 0) || (local_10c == 1)) {
        local_120 = FUN_0054e06c(*(undefined4 *)(local_e0 + 0x670));
        if ((local_120 != 9) ||
           (*(short *)(&DAT_00d449d6 + **(short **)(local_e0 + 0x670) * 0x8c) == 0)) {
          FUN_00403df2(&DAT_00a3d4f8,2000);
          (**(code **)(**(int **)(local_110 + 0x84) + 0x88))(1,1);
          ExceptionList = local_10;
          return 1;
        }
        if (local_10c == 1) {
          local_124 = (**(code **)(*local_104[0] + 0xb4))(0,0);
          iVar4 = _strcmp(&DAT_00d44950 + **(short **)(local_124 + 0x670) * 0x8c,
                          &DAT_00d44950 + **(short **)(local_e0 + 0x670) * 0x8c);
          if (iVar4 != 0) {
            FUN_00403df2(&DAT_00a3c478,2000);
            (**(code **)(**(int **)(local_110 + 0x84) + 0x88))(1,1);
            ExceptionList = local_10;
            return 1;
          }
        }
      }
      else if (local_10c == 2) {
        if ((**(short **)(local_e0 + 0x670) < 0x6ee) || (0x6f0 < **(short **)(local_e0 + 0x670))) {
          FUN_00403df2(&DAT_00a3a1f8,2000);
          (**(code **)(**(int **)(local_110 + 0x84) + 0x88))(1,1);
          ExceptionList = local_10;
          return 1;
        }
      }
      else if ((local_10c < 3) || (6 < local_10c)) {
        local_128 = FUN_0054e06c(*(undefined4 *)(local_e0 + 0x670));
        if ((local_128 < 7) ||
           (*(short *)(&DAT_00d449d6 + **(short **)(local_e0 + 0x670) * 0x8c) == 0)) {
          FUN_00403df2(&DAT_00a3a278,2000);
          (**(code **)(**(int **)(local_110 + 0x84) + 0x88))(1,1);
          ExceptionList = local_10;
          return 1;
        }
      }
      else {
        if ((**(short **)(local_e0 + 0x670) < 0x989) || (0x992 < **(short **)(local_e0 + 0x670))) {
          FUN_00403df2(&DAT_00a3cbf8,2000);
          (**(code **)(**(int **)(local_110 + 0x84) + 0x88))(1,1);
          ExceptionList = local_10;
          return 1;
        }
        if ((local_10c == 3) && (iVar4 = _strcmp(&DAT_00a3db78,&DAT_005ccf2c), iVar4 != 0)) {
          FUN_00403df2(&DAT_00a3db78,3000);
          (**(code **)(**(int **)(local_110 + 0x84) + 0x88))(1,1);
        }
      }
      local_118 = operator_new(8);
      FUN_0058f220(local_118,*(undefined4 *)(local_e0 + 0x670),8);
      pvVar5 = operator_new(0x678);
      local_8 = 3;
      if (pvVar5 == (void *)0x0) {
        local_530 = 0;
      }
      else {
        local_530 = FUN_0040d13e(0,local_118,0,0);
      }
      local_8 = 0xffffffff;
      local_11c = local_530;
      (**(code **)(*local_104[local_10c] + 0x8c))(local_530,0,0);
      FUN_0058f220(DAT_013b71e8 + 0xd5c + local_10c * 8,*(undefined4 *)(local_e0 + 0x670),8);
      *(undefined1 *)(DAT_013b71e8 + local_10c + 0xd9c) = (undefined1)local_108;
      *(undefined4 *)(local_e0 + 0x94) = 0xffff0000;
      local_e4 = 1;
      break;
    }
  }
  local_e8 = (**(code **)(*local_104[3] + 0xb8))(0,0);
  if ((((local_e8 == 0) || (**(short **)(local_e0 + 0x670) < 0x989)) ||
      (0x992 < **(short **)(local_e0 + 0x670))) ||
     (iVar4 = (**(code **)(**(int **)(local_110 + 0x27c60) + 0xb8))(0,0), iVar4 != 0))
  goto LAB_00412e02;
  iVar4 = (**(code **)(*local_104[0] + 0xb8))(0,0);
  local_130 = *(undefined2 **)(iVar4 + 0x670);
  iVar4 = (**(code **)(*local_104[1] + 0xb8))(0,0);
  local_12c = *(int *)(iVar4 + 0x670);
  FUN_0058f220(local_15c,local_130,8);
  uVar2 = local_15a;
  if ((char)local_15a == '+') {
LAB_00412816:
    bVar1 = false;
  }
  else {
    if (((char)local_15a < 's') || ('~' < (char)local_15a)) {
      bVar1 = false;
    }
    else {
      bVar1 = true;
    }
    if (bVar1) goto LAB_00412816;
    bVar1 = true;
  }
  if ((bVar1) && ((char)local_158 == '+')) {
    local_160 = local_15a;
    local_15a = local_158;
    local_158 = uVar2;
  }
  uVar2 = local_15a;
  if ((char)local_15a == '+') {
LAB_00412908:
    bVar1 = false;
  }
  else {
    if (((char)local_15a < 's') || ('~' < (char)local_15a)) {
      bVar1 = false;
    }
    else {
      bVar1 = true;
    }
    if (bVar1) goto LAB_00412908;
    bVar1 = true;
  }
  if ((bVar1) && ((char)local_156 == '+')) {
    local_164 = local_15a;
    local_15a = local_156;
    local_156 = uVar2;
  }
  local_13c = operator_new(8);
  local_134 = operator_new(8);
  local_150 = operator_new(8);
  local_148 = operator_new(8);
  FUN_0058f220(local_13c,local_15c,8);
  FUN_0058f220(local_134,local_15c,8);
  FUN_0058f220(local_150,local_12c,8);
  FUN_0058f220(local_148,local_12c,8);
  *local_150 = *local_130;
  *local_148 = *local_130;
  local_144 = FUN_0054e06c(local_130);
  if (local_144 == 9) {
    local_144 = **(short **)(local_e8 + 0x670) + -0x97f;
  }
  else if (local_144 == 10) {
    local_144 = **(short **)(local_e8 + 0x670) + -0x97b;
  }
  else if (local_144 == 0xb) {
    local_144 = **(short **)(local_e8 + 0x670) + -0x977;
  }
  *(undefined2 *)((int)local_134 + 4) = *(undefined2 *)(local_12c + 4);
  local_148[2] = local_158;
  FUN_0054e75a(local_13c,local_144,0);
  FUN_0054e75a(local_134,local_144,0);
  FUN_0054e75a(local_150,local_144,0);
  FUN_0054e75a(local_148,local_144,0);
  pvVar5 = operator_new(0x678);
  local_8 = 4;
  if (pvVar5 == (void *)0x0) {
    local_554 = 0;
  }
  else {
    local_554 = FUN_0040d13e(0,local_13c,0,0);
  }
  local_8 = 0xffffffff;
  local_154 = local_554;
  pvVar5 = operator_new(0x678);
  local_8 = 5;
  if (pvVar5 == (void *)0x0) {
    local_558 = 0;
  }
  else {
    local_558 = FUN_0040d13e(0,local_134,0,0);
  }
  local_8 = 0xffffffff;
  local_14c = local_558;
  pvVar5 = operator_new(0x678);
  local_8 = 6;
  if (pvVar5 == (void *)0x0) {
    local_55c = 0;
  }
  else {
    local_55c = FUN_0040d13e(0,local_150,0,0);
  }
  local_8 = 0xffffffff;
  local_140 = local_55c;
  pvVar5 = operator_new(0x678);
  local_8 = 7;
  if (pvVar5 == (void *)0x0) {
    local_560 = 0;
  }
  else {
    local_560 = FUN_0040d13e(0,local_148,0,0);
  }
  local_8 = 0xffffffff;
  local_138 = local_560;
  (**(code **)(**(int **)(local_110 + 0x27c60) + 0x8c))(local_154,0,0);
  (**(code **)(**(int **)(local_110 + 0x27c64) + 0x8c))(local_14c,0,0);
  (**(code **)(**(int **)(local_110 + 0x27c68) + 0x8c))(local_140,0,0);
  (**(code **)(**(int **)(local_110 + 0x27c6c) + 0x8c))(local_138,0,0);
LAB_00412e02:
  if (local_e4 != 0) {
    ExceptionList = local_10;
    return 1;
  }
  ExceptionList = local_10;
  return 0;
}

