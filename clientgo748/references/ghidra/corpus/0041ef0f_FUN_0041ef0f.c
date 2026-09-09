// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0041ef0f | Name: FUN_0041ef0f


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0041ef0f(int *param_1,int param_2,uint param_3,int param_4)

{
  int *piVar1;
  short sVar2;
  ushort uVar3;
  undefined2 uVar4;
  int iVar5;
  void *pvVar6;
  uint uVar7;
  undefined4 *puVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 local_294;
  short local_258;
  int local_244;
  undefined1 local_220 [4];
  undefined2 local_21c;
  undefined2 local_21a;
  undefined4 local_214;
  int local_210;
  undefined2 local_20c;
  short local_208;
  undefined4 local_204;
  int *local_200;
  ushort local_1fc;
  undefined2 uStack_1fa;
  uint local_1f8;
  int local_1f4;
  undefined2 local_1f0;
  short local_1ec;
  undefined4 local_1e8;
  undefined4 local_1e4;
  undefined1 local_1e0 [4];
  undefined2 local_1dc;
  undefined2 local_1da;
  undefined4 local_1d4;
  int local_1d0;
  undefined2 local_1c4;
  undefined2 local_1c2;
  undefined2 local_1c0;
  int local_1bc;
  undefined4 local_1b8;
  short local_1b4;
  short local_1b0;
  int *local_1ac;
  int *local_1a8;
  undefined1 local_1a4 [128];
  short local_124;
  undefined2 local_120;
  int *local_11c;
  int local_118;
  int local_114;
  char local_110;
  int local_10c;
  int local_108;
  undefined4 local_104;
  int local_100;
  int local_fc;
  undefined1 local_f8 [4];
  undefined4 local_f4;
  undefined2 local_ec;
  undefined2 local_e8;
  undefined2 local_e6;
  undefined2 local_e4;
  undefined2 local_e2;
  short local_e0;
  undefined2 local_de;
  undefined1 local_dc;
  undefined1 local_db;
  undefined1 local_da;
  undefined2 local_cc;
  undefined2 local_ca;
  int *local_98;
  int local_94;
  uint local_90;
  undefined1 local_8c [6];
  undefined2 local_86;
  undefined1 local_6e;
  char local_2c;
  uint local_28;
  int local_24;
  undefined4 local_20;
  int local_1c;
  int *local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059ef91;
  local_10 = ExceptionList;
  local_18 = DAT_0067cf38;
  if ((DAT_0067cf38 != (int *)0x0) && (DAT_0067cf38[0x13] != 0)) {
    local_14 = DAT_0067cf38[0x13];
    ExceptionList = &local_10;
    iVar5 = (**(code **)(*DAT_005ccec0 + 0x90))();
    if (((iVar5 == 0) &&
        (((param_4 == 1 && (0x222 < (uint)param_1[0x11])) && ((uint)param_1[0x11] < 0x23b)))) &&
       ((*(int *)(local_14 + 0x740) != 2 && (*(int *)(local_14 + 0x740) != 3)))) {
      local_1c = (**(code **)(*param_1 + 0xb8))(0,0);
      if (local_1c != 0) {
        local_28 = (**(code **)(*DAT_0092e654 + 8))();
        local_98 = DAT_0067cf38;
        if (((((DAT_0067cf38 != (int *)0x0) && (local_24 = DAT_0067cf38[0x13], local_24 != 0)) &&
             (local_2c = (char)**(undefined2 **)(local_1c + 0x670) + 'x', -1 < local_2c)) &&
            (((local_2c < 'h' && (iVar5 = FUN_00434228((int)local_2c), iVar5 != 0)) &&
             ((*(int *)(&DAT_0092fefc + local_2c * 0x60) == 0 ||
              (*(int *)(&DAT_0092fefc + local_2c * 0x60) == 2)))))) &&
           (((*(int *)(&DAT_0092ff40 + local_2c * 0x60) != 1 &&
             ((0 < *(int *)(&DAT_0092ff1c + local_2c * 0x60) ||
              (0 < *(int *)(&DAT_0092ff14 + local_2c * 0x60))))) &&
            (*(int *)(&DAT_0092ff50 + local_2c * 0x60) != 1)))) {
          local_94 = *(int *)(&DAT_0092ff04 + local_2c * 0x60);
          if ((8 < local_98[0x9b9e]) && (1 < local_94)) {
            local_94 = local_94 + -1;
          }
          if ((((uint)(*(int *)(&DAT_0092eaf8 + local_2c * 4) + local_94 * 1000) <= local_28) &&
              (*(int *)(&DAT_0092eaf8 + local_2c * 4) + 1000U <= local_28)) &&
             (local_98[0x9d18] + 1000U <= local_28)) {
            local_90 = (uint)*(short *)(local_24 + 0x444);
            if (local_2c < '`') {
              local_fc = (int)local_2c + (uint)*(byte *)(DAT_013b71e8 + 0x700) * -0x18;
              local_90 = (uint)*(byte *)(DAT_013b71e8 + 0x745 +
                                        ((int)(local_fc + (local_fc >> 0x1f & 7U)) >> 3));
            }
            iVar5 = FUN_00555790((int)local_2c,*(undefined1 *)(DAT_013b71e8 + 0x9d3),local_90);
            if (*(short *)(DAT_013b71e8 + 0x73a) < iVar5) {
              local_100 = local_98[0x9eba];
              pvVar6 = operator_new(0xe50);
              local_8 = 0;
              if (pvVar6 == (void *)0x0) {
                local_294 = 0;
              }
              else {
                local_294 = FUN_00407203(&DAT_00a35078,0xffffaaaa,0,0,0x43960000,0x41800000,0,
                                         0x77777777,1,0);
              }
              local_8 = 0xffffffff;
              local_104 = local_294;
              FUN_00408d33(local_294);
              if ((DAT_005ccf98 != 0) && (iVar5 = FUN_00429a6d(0x21), iVar5 != 0)) {
                uVar10 = 0;
                uVar9 = 0;
                FUN_00429a6d(0x21);
                FUN_0042ad2b(uVar9,uVar10);
              }
            }
            else {
              _memset(local_f8,0,0x60);
              local_f4 = CONCAT22(*(undefined2 *)(local_24 + 0x20),0x36c);
              local_ec = *(undefined2 *)(local_24 + 0x20);
              local_e8 = __ftol();
              local_e6 = __ftol();
              local_e4 = __ftol();
              local_e2 = __ftol();
              local_de = 0xffff;
              local_e0 = (short)local_2c;
              local_db = 0;
              local_dc = 0xff;
              local_da = 0;
              local_ca = 0xffff;
              local_cc = *(undefined2 *)(local_24 + 0x20);
              local_20 = 0x60;
              if (*(int *)(&DAT_0092ff44 + local_2c * 0x60) == 1) {
                local_f4 = CONCAT22(local_f4._2_2_,0x39d);
                local_20 = 0x30;
              }
              else if (*(int *)(&DAT_0092ff44 + local_2c * 0x60) == 2) {
                local_f4 = CONCAT22(local_f4._2_2_,0x39e);
                local_20 = 0x34;
              }
              FUN_0055f2dd(local_f8,local_20);
              _memset(local_8c,0,0x60);
              FUN_0058f220(local_8c,local_f8,local_20);
              local_86 = (undefined2)param_1[8];
              local_6e = 1;
              (**(code **)(*local_98 + 4))(local_f4 & 0xffff,local_8c);
              local_98[0x9d18] = local_28;
              local_98[0x9ba4] = 0;
              *(uint *)(&DAT_0092eaf8 + local_2c * 4) = local_28;
            }
          }
        }
      }
    }
    else {
      iVar5 = (**(code **)(*DAT_005ccec0 + 0x90))();
      if ((iVar5 == 2) && (DAT_005ccec0[0x7a] != 0)) {
        (**(code **)(*DAT_005ccec0 + 0x98))();
        *(undefined4 *)(DAT_005d0504 + 0x43c) = 1;
      }
      else {
        iVar5 = (**(code **)(*DAT_005ccec0 + 0x90))();
        if ((((iVar5 == 0) && (param_1[0x79] == 0)) &&
            ((param_1[0x100] == 0 || (param_1[0x100] == 3)))) &&
           ((param_4 != 0 && (*(int *)(DAT_005d0504 + 0x43c) == 0)))) {
          local_108 = DAT_0067cf38[0x13];
          local_10c = (**(code **)(*param_1 + 0xb4))(param_2,param_3);
          if (local_10c != 0) {
            local_114 = FUN_0054cd07(*(undefined4 *)(local_10c + 0x670),0x26);
            local_118 = (int)**(short **)(local_10c + 0x670);
            local_110 = *(char *)(DAT_013b71e8 + 0xef6 + (int)*(char *)(DAT_013b71e8 + 0xef5));
            if ((local_110 == 'T') && (DAT_0067cf38[9] == 30000)) {
              local_11c = DAT_0067cf38;
              iVar5 = DAT_0067cf38[0x9d18];
              uVar7 = (**(code **)(*DAT_0092e654 + 8))();
              if (iVar5 + 1000U < uVar7) {
                local_1a8 = DAT_0067cf38;
                FUN_0058f078(local_1a4,&DAT_00a38bf8,
                             &DAT_00d44950 + **(short **)(local_10c + 0x670) * 0x8c);
                (**(code **)(*(int *)local_1a8[0x23] + 0x8c))(local_1a4,0x54,&DAT_00a38c78);
                (**(code **)(*(int *)local_1a8[0x23] + 0x60))(1);
                local_120 = (**(code **)(*param_1 + 0xc0))
                                      (*(undefined4 *)(*(int *)(local_10c + 0x1e4) + 0x1e4),
                                       *(undefined4 *)(*(int *)(local_10c + 0x1e4) + 0x400));
                local_124 = (**(code **)(*param_1 + 0xc4))
                                      (*(undefined4 *)(*(int *)(local_10c + 0x1e4) + 0x1e4));
                if (local_124 == -1) {
                  local_124 = (short)*(undefined4 *)(local_10c + 0x1d0) +
                              (short)*(undefined4 *)(local_10c + 0x1d4) * 9;
                }
                *(undefined2 *)(local_1a8 + 0x9d78) = local_120;
                *(short *)((int)local_1a8 + 0x275e2) = local_124;
              }
            }
            else if (local_114 == 0xe) {
              local_1b0 = (short)param_2;
              local_1b4 = (short)param_3;
              local_1ac = DAT_0067cf38;
              (**(code **)(*(int *)DAT_0067cf38[0x23] + 0x8c))(&DAT_00a398f8,0x26,0);
              *(int *)(local_1ac[0x23] + 0x1e8) = (int)local_1b0 << 0x10 | (int)local_1b4;
              (**(code **)(*(int *)local_1ac[0x23] + 0x60))(1);
            }
            else if (local_114 == 0x11) {
              if (*(short *)(DAT_013b71e8 + 0x7a8) != 0) {
                local_200 = DAT_0067cf38;
                local_1bc = (**(code **)(*DAT_0092e654 + 8))();
                if ((local_200[0x9d1e] == 0) || (199 < (uint)(local_1bc - local_200[0x9d1e]))) {
                  local_1f4 = DAT_013b71e8 + 0x7a8;
                  sVar2 = FUN_0054cd07(local_1f4,0x38);
                  uVar3 = FUN_0054cd07(local_1f4,0x39);
                  local_1f8 = CONCAT22(local_1f8._2_2_,sVar2 << 8 | uVar3);
                  sVar2 = FUN_0054cd07(*(undefined4 *)(local_10c + 0x670),0x38);
                  uVar3 = FUN_0054cd07(*(undefined4 *)(local_10c + 0x670),0x39);
                  uVar3 = sVar2 << 8 | uVar3;
                  _local_1fc = CONCAT22(uStack_1fa,uVar3);
                  if ((local_1f8 & 0xffff) == (uint)uVar3) {
                    local_1f0 = (**(code **)(*param_1 + 0xc0))
                                          (*(undefined4 *)(*(int *)(local_10c + 0x1e4) + 0x1e4),
                                           *(undefined4 *)(*(int *)(local_10c + 0x1e4) + 0x400));
                    local_1ec = (**(code **)(*param_1 + 0xc4))
                                          (*(undefined4 *)(*(int *)(local_10c + 0x1e4) + 0x1e4));
                    if (local_1ec == -1) {
                      local_1ec = (short)*(undefined4 *)(local_10c + 0x1d0) +
                                  (short)*(undefined4 *)(local_10c + 0x1d4) * 9;
                    }
                    local_1e8 = *(undefined4 *)(local_108 + 0x28);
                    local_1e4 = *(undefined4 *)(local_108 + 0x2c);
                    _memset(local_1e0,0,0x24);
                    local_1da = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
                    local_1dc = 0x373;
                    local_1d4 = 1;
                    local_1d0 = (int)local_1ec;
                    local_1c0 = 0;
                    local_1c4 = __ftol();
                    local_1c2 = __ftol();
                    FUN_0055f2dd(local_1e0,0x24);
                    local_200[0x9d1e] = local_1bc;
                    *(undefined4 *)(DAT_005d0504 + 0x43c) = 1;
                    local_1b8 = 0x36;
                    if ((DAT_005ccf98 != 0) && (iVar5 = FUN_00429a6d(0x36), iVar5 != 0)) {
                      uVar10 = 0;
                      uVar9 = 0;
                      FUN_00429a6d(local_1b8);
                      FUN_0042ad2b(uVar9,uVar10);
                    }
                  }
                }
              }
            }
            else {
              if ((local_114 == 0xb) || (local_114 == 0xd)) {
                local_20c = (**(code **)(*param_1 + 0xc0))
                                      (*(undefined4 *)(*(int *)(local_10c + 0x1e4) + 0x1e4),
                                       *(undefined4 *)(*(int *)(local_10c + 0x1e4) + 0x400));
                local_208 = (**(code **)(*param_1 + 0xc4))
                                      (*(undefined4 *)(*(int *)(local_10c + 0x1e4) + 0x1e4));
                piVar1 = DAT_0067cf38;
                if (local_208 == -1) {
                  local_208 = (short)*(undefined4 *)(local_10c + 0x1d0) +
                              (short)*(undefined4 *)(local_10c + 0x1d4) * 9;
                }
                if (DAT_0067cf38[9] == 30000) {
                  if (((DAT_0067cf38[0x13] != 0) &&
                      (*(float *)(DAT_0067cf38[0x13] + 0x230) < _DAT_005a40f4)) &&
                     (_DAT_005a3534 < *(float *)(DAT_0067cf38[0x13] + 0x230))) {
                    ExceptionList = local_10;
                    return;
                  }
                  iVar5 = (**(code **)(*DAT_0092e654 + 8))();
                  piVar1[0x9d1d] = iVar5;
                  piVar1[0x9c51] = piVar1[0x9d1d];
                  *(undefined1 *)((int)piVar1 + 0x271d1) = 1;
                  _memset(piVar1 + 0x9c55,0,0x24);
                  *(undefined2 *)((int)piVar1 + 0x2715a) = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
                  *(undefined2 *)(piVar1 + 0x9c56) = 0x373;
                  piVar1[0x9c58] = 1;
                  piVar1[0x9c59] = (int)local_208;
                  *(undefined2 *)(piVar1 + 0x9c5d) = 0;
                  uVar4 = __ftol();
                  *(undefined2 *)(piVar1 + 0x9c5c) = uVar4;
                  uVar4 = __ftol();
                  *(undefined2 *)((int)piVar1 + 0x27172) = uVar4;
                  local_21a = *(undefined2 *)(piVar1[0x13] + 0x20);
                  local_21c = 0x3ae;
                  local_214 = 1;
                  FUN_0055f2dd(local_220,0x10);
                }
                *(undefined4 *)(DAT_005d0504 + 0x43c) = 1;
                local_210 = FUN_0054e93b(*(undefined4 *)(local_10c + 0x670));
                if ((0x919 < **(short **)(local_10c + 0x670)) &&
                   (**(short **)(local_10c + 0x670) < 0x956)) {
                  local_210 = 0;
                }
                if (local_210 < 2) {
                  puVar8 = (undefined4 *)(**(code **)(*param_1 + 0xa4))(param_2,param_3);
                  if ((DAT_005ccec0[0x7a] != 0) && ((undefined4 *)DAT_005ccec0[0x7a] == puVar8)) {
                    DAT_005ccec0[0x7a] = 0;
                  }
                  if ((puVar8 != (undefined4 *)0x0) && (puVar8 != (undefined4 *)0x0)) {
                    (**(code **)*puVar8)(1);
                  }
                }
                else {
                  FUN_0054e9ab(*(undefined4 *)(local_10c + 0x670),local_210 + -1);
                  iVar5 = local_10c;
                  FUN_0058f078(local_10c + 0x39a,&PTR_DAT_005b16b4,local_210 + -1);
                  FUN_00421b8f(iVar5 + 0x39a,*(undefined4 *)(iVar5 + 900),0);
                }
                local_204 = 0x29;
                if ((10 < local_114) && (local_114 < 0xe)) {
                  local_204 = 0x36;
                }
                if ((DAT_005ccf98 != 0) && (iVar5 = FUN_00429a6d(local_204), iVar5 != 0)) {
                  uVar10 = 0;
                  uVar9 = 0;
                  FUN_00429a6d(local_204);
                  FUN_0042ad2b(uVar9,uVar10);
                }
                if (DAT_0067cf38[9] == 30000) {
                  FUN_004431e4(0);
                }
                if (local_210 < 2) {
                  _memset((void *)(DAT_013b71e8 + 0x7c8 + local_208 * 8),0,8);
                }
              }
              piVar1 = DAT_0067cf38;
              if ((local_118 == 0xd08) && (DAT_0067cf38[9] == 30000)) {
                FUN_00403df2(&DAT_00a3d078,3000);
                (**(code **)(*(int *)piVar1[0x21] + 0x88))(1,1);
              }
              else if (local_118 == 0xd72) {
                iVar5 = (**(code **)(*DAT_0092e654 + 8))();
                piVar1 = DAT_0067cf38;
                if ((DAT_0067cf38[0x9d1e] == 0) || (199 < (uint)(iVar5 - DAT_0067cf38[0x9d1e]))) {
                  (**(code **)(*(int *)DAT_0067cf38[0xa269] + 0x60))(1);
                  for (local_244 = 0; local_244 < 100; local_244 = local_244 + 1) {
                    (**(code **)(*(int *)piVar1[local_244 + 0xa26a] + 0x54))();
                  }
                  *(short *)(piVar1 + 0x9c4c) = (short)param_2;
                  *(short *)((int)piVar1 + 0x27132) = (short)param_3;
                  piVar1[0x9d1e] = iVar5;
                  *(undefined4 *)(DAT_005d0504 + 0x43c) = 1;
                }
              }
              else if (local_114 == 0xc3) {
                FUN_0044a9e9(1,**(short **)(local_10c + 0x670) + -0xd68);
                (**(code **)(*param_1 + 0xc0))
                          (*(undefined4 *)(*(int *)(local_10c + 0x1e4) + 0x1e4),
                           *(undefined4 *)(*(int *)(local_10c + 0x1e4) + 0x400));
                local_258 = (**(code **)(*param_1 + 0xc4))
                                      (*(undefined4 *)(*(int *)(local_10c + 0x1e4) + 0x1e4));
                if (local_258 == -1) {
                  local_258 = (short)*(undefined4 *)(local_10c + 0x1d0) +
                              (short)*(undefined4 *)(local_10c + 0x1d4) * 9;
                }
                _memset(piVar1 + 0x9c5e,0,0x24);
                *(undefined2 *)((int)piVar1 + 0x2717e) = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
                *(undefined2 *)(piVar1 + 0x9c5f) = 0x373;
                piVar1[0x9c61] = 1;
                piVar1[0x9c62] = (int)local_258;
                *(undefined2 *)(piVar1 + 0x9c66) = 0;
                uVar4 = __ftol();
                *(undefined2 *)(piVar1 + 0x9c65) = uVar4;
                uVar4 = __ftol();
                *(undefined2 *)((int)piVar1 + 0x27196) = uVar4;
              }
              else if (local_114 == 0xbb) {
                if (DAT_0067cf38[9] == 30000) {
                  (**(code **)(*(int *)DAT_0067cf38[0x23] + 0x8c))(&DAT_00a3d778,0x373,0);
                  *(uint *)(piVar1[0x23] + 0x1e8) = param_2 << 0x10 | param_3;
                  (**(code **)(*(int *)piVar1[0x23] + 0x60))(1);
                }
              }
              else if (((((((((((((local_114 == 1) || (local_114 == 0xc)) || (local_114 == 6)) ||
                               ((local_114 == 7 || (local_114 == 8)))) || (local_114 == 10)) ||
                             ((local_114 == 0xf || (local_114 == 0x12)))) ||
                            ((0x12 < local_114 && (local_114 < 0x1d)))) ||
                           ((local_114 == 0x1e ||
                            (((0x1e < local_114 && (local_114 < 0x25)) ||
                             ((0x27 < local_114 && (local_114 < 0x3b)))))))) ||
                          (((0x3b < local_114 && (local_114 < 0x46)) ||
                           ((0x45 < local_114 && (local_114 < 0x5a)))))) ||
                         ((((((((0x82 < local_114 && (local_114 < 0x8b)) || (local_114 == 0x8c)) ||
                              (((0xa0 < local_114 && (local_114 < 0xa9)) ||
                               ((local_114 == 0xaa || ((local_114 == 0xab || (local_114 == 0xac)))))
                               ))) || (local_114 == 200)) ||
                            (((local_114 == 0xc9 || (local_114 == 0xca)) || (local_114 == 0xad))))
                           || (((local_114 == 0xae || (local_114 == 0xaf)) ||
                               ((local_114 == 0xb0 || ((local_114 == 0xb1 || (local_114 == 0xb2)))))
                               ))) ||
                          ((((0xb7 < local_114 && (local_114 < 0xba)) ||
                            (((((((local_114 == 0xcb || (local_114 == 0xcc)) || (local_114 == 0xcd))
                                || ((local_114 == 0xbc || (local_114 == 0xbd)))) ||
                               ((local_114 == 0xbf || ((local_114 == 0xc0 || (local_114 == 0xc1)))))
                               ) || (local_114 == 0xc2)) ||
                             ((((local_114 == 0xc5 || (local_114 == 0xc6)) || (local_114 == 0xce))
                              || (((local_114 == 0xd2 || (local_114 == 0xd0)) ||
                                  ((0xc7f < local_118 && (local_118 < 0xce4)))))))))) ||
                           ((local_118 == 0x1033 || (local_118 == 0x14da)))))))) ||
                        ((0xd74 < local_118 && (local_118 < 0xd79)))) ||
                       (((((local_118 == 0xd7b || (local_118 == 0xd7c)) || (local_118 == 0x1411)) ||
                         (((local_118 == 0xd7d || (local_118 == 0xd7e)) ||
                          ((local_118 == 0x286 || ((local_118 == 0x287 || (local_118 == 0xd32)))))))
                         ) || ((local_118 == 0xfbe ||
                               ((((local_118 == 0xfbf || (local_118 == 0xfae)) ||
                                 (local_118 == 0xbcc)) || (local_118 == 0x6ed)))))))) &&
                      (((DAT_0067cf38[9] != 30000 ||
                        (*(char *)(DAT_0067cf38[0x13] + 0x788) != '\x01')) || (local_114 != 1)))) {
                if ((local_114 < 0x46) || (0x59 < local_114)) {
                  if (local_114 != 0xb8) {
                    if ((local_114 == 0xce) && (*(char *)(*(int *)(local_10c + 0x670) + 2) == ';'))
                    {
                      if (DAT_0067cf38 == (int *)0x0) {
                        ExceptionList = local_10;
                        return;
                      }
                      FUN_004658a7(local_10c,param_2,param_3);
                      ExceptionList = local_10;
                      return;
                    }
                    if (local_114 == 0xce) {
                      (**(code **)(*(int *)DAT_0067cf38[0x23] + 0x8c))(&DAT_00a3f078,0x373,0);
                      *(uint *)(piVar1[0x23] + 0x1e8) = param_2 << 0x10 | param_3;
                      (**(code **)(*(int *)piVar1[0x23] + 0x60))(1);
                      ExceptionList = local_10;
                      return;
                    }
                    if (local_114 == 0xd3) {
                      (**(code **)(*(int *)DAT_0067cf38[0x23] + 0x8c))(&DAT_00a3f378,0x373,0);
                      *(uint *)(piVar1[0x23] + 0x1e8) = param_2 << 0x10 | param_3;
                      (**(code **)(*(int *)piVar1[0x23] + 0x60))(1);
                      ExceptionList = local_10;
                      return;
                    }
                  }
                }
                else if ((DAT_0067cf38[9] == 30000) && (*(int *)(DAT_0067cf38[0xa1dd] + 0x200) != 0)
                        ) {
                  FUN_00403df2(&DAT_00a3cff8,3000);
                  (**(code **)(*(int *)piVar1[0x21] + 0x88))(1,1);
                  ExceptionList = local_10;
                  return;
                }
                if (DAT_0067cf38[0x13] != 0) {
                  if (local_118 == 0xfbe) {
                    FUN_0044c947();
                  }
                  else if (local_118 == 0xfbf) {
                    FUN_0044c9d6();
                  }
                  else if ((local_118 < 0x1024) || (0x102b < local_118)) {
                    FUN_00465f85(local_10c,local_114,local_118,param_2,param_3);
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  ExceptionList = local_10;
  return;
}

