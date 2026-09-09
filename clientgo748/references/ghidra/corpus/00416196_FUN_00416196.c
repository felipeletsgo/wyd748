// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00416196 | Name: FUN_00416196


undefined4 __thiscall
FUN_00416196(int *param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  int local_184;
  undefined1 local_17c [8];
  undefined1 local_174 [8];
  undefined4 *local_16c;
  undefined4 *local_168;
  undefined4 *local_164;
  undefined4 *local_160;
  void *local_15c;
  int local_158;
  short *local_154;
  int *local_150;
  undefined4 *local_14c;
  undefined4 *local_148;
  short local_144;
  int *local_140;
  int local_13c;
  undefined4 local_138;
  int *local_134;
  undefined4 local_130;
  undefined1 local_12c [4];
  undefined2 local_128;
  undefined2 local_126;
  int local_120;
  int local_11c;
  int local_118;
  int local_114;
  undefined2 local_110;
  undefined2 local_10e;
  undefined2 local_10c;
  short local_108;
  int local_104;
  int local_100;
  short local_fc;
  int local_f8;
  int local_f4;
  short local_f0;
  undefined4 *local_ec;
  int local_e8;
  int local_e4;
  short *local_e0;
  int local_dc;
  int *local_d8;
  int local_d4;
  undefined1 local_d0 [4];
  undefined2 local_cc;
  undefined2 local_ca;
  char local_c4 [20];
  undefined1 local_b0 [128];
  undefined1 local_30 [4];
  undefined2 local_2c;
  undefined2 local_2a;
  undefined2 local_24;
  undefined2 local_22;
  short local_20;
  short local_1c;
  undefined2 local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059ef79;
  local_10 = ExceptionList;
  if (param_1[0x100] == 1) {
    local_14 = DAT_0067cf38;
    DAT_005ccf10 = DAT_005ccec0[0x7a];
    if (DAT_005ccf10 == 0) {
      return 1;
    }
    if (*(int *)(DAT_005d0504 + 0x438) == 0) {
      ExceptionList = &local_10;
      FUN_0058f078(local_b0,&DAT_00a3ec78,&DAT_00d44950 + **(short **)(DAT_005ccf10 + 0x670) * 0x8c)
      ;
      (**(code **)(*(int *)local_14[0x23] + 0x8c))(local_b0,0x37a,&DAT_00a3ecf8);
      (**(code **)(*(int *)local_14[0x23] + 0x60))(1);
      DAT_005ccec0[0x7a] = 0;
    }
    else {
      ExceptionList = &local_10;
      local_18 = (**(code **)(*param_1 + 0xc0))
                           (*(undefined4 *)(*(int *)(DAT_005ccf10 + 0x1e4) + 0x1e4),
                            *(undefined4 *)(*(int *)(DAT_005ccf10 + 0x1e4) + 0x400));
      local_1c = (**(code **)(*param_1 + 0xc4))
                           (*(undefined4 *)(*(int *)(DAT_005ccf10 + 0x1e4) + 0x1e4));
      if (local_1c == -1) {
        local_1c = (short)*(undefined4 *)(DAT_005ccf10 + 0x1d0) +
                   (short)*(undefined4 *)(DAT_005ccf10 + 0x1d4) * 9;
      }
      _memset(local_30,0,0x14);
      local_2a = *(undefined2 *)(DAT_0067cf38[0x13] + 0x20);
      local_2c = 0x37a;
      local_24 = (undefined2)param_1[0x101];
      local_22 = local_18;
      local_20 = local_1c;
      FUN_0055f2dd(local_30,0x14);
      DAT_005ccf10 = 0;
    }
    param_1[0x15e] = 0;
  }
  else if (param_1[0x100] == 5) {
    if ((**(short **)(DAT_005ccec0[0x7a] + 0x670) < 5000) ||
       (0x13f0 < **(short **)(DAT_005ccec0[0x7a] + 0x670))) {
      return 1;
    }
    local_dc = 0;
    if (param_1[0x11] == 0x24a) {
      local_dc = 10;
    }
    ExceptionList = &local_10;
    local_ec = (undefined4 *)(**(code **)(*param_1 + 0xa4))(param_2,param_3);
    local_154 = operator_new(8);
    local_e0 = local_154;
    FUN_0058f220(local_154,*(undefined4 *)(DAT_005ccec0[0x7a] + 0x670),8);
    local_15c = operator_new(0x678);
    local_8 = 0;
    if (local_15c == (void *)0x0) {
      local_184 = 0;
    }
    else {
      local_184 = FUN_0040d13e(0,local_e0,0,0);
    }
    local_158 = local_184;
    local_8 = 0xffffffff;
    local_e8 = local_184;
    (**(code **)(*param_1 + 0x8c))(local_184,param_2,param_3);
    if ((*(char *)(DAT_013b71e8 + 0xef5) - local_dc == param_2) &&
       (*(undefined4 *)(local_e8 + 0xa0) = 2, DAT_005b892c == 2)) {
      *(undefined4 *)(local_e8 + 0xa0) = 200;
    }
    (**(code **)(*DAT_005ccec0 + 0x98))();
    if (local_ec != (undefined4 *)0x0) {
      local_164 = local_ec;
      local_160 = local_ec;
      if (local_ec != (undefined4 *)0x0) {
        (**(code **)*local_ec)(1);
      }
      local_ec = (undefined4 *)0x0;
    }
    local_e4 = DAT_013b71e8 + 0x6ec;
    *(undefined *)(DAT_013b71e8 + 0xef6 + param_2 + local_dc) = (&DAT_00d44992)[*local_e0 * 0x8c];
    _memset(local_d0,0,0x20);
    local_ca = *(undefined2 *)(DAT_0067cf38[0x13] + 0x20);
    local_cc = 0x378;
    FUN_0058f220(local_c4,DAT_013b71e8 + 0xef6,0x14);
    for (local_d4 = 0; local_d4 < 0x14; local_d4 = local_d4 + 1) {
      if ((-1 < local_c4[local_d4]) && (local_c4[local_d4] < '`')) {
        local_c4[local_d4] = local_c4[local_d4] + *(char *)(DAT_013b71e8 + 0x700) * -0x18;
      }
    }
    FUN_0055f2dd(local_d0,0x20);
    if ((DAT_005ccf98 != 0) && (iVar1 = FUN_00429a6d(0x1f), iVar1 != 0)) {
      uVar3 = 0;
      uVar2 = 0;
      FUN_00429a6d(0x1f);
      FUN_0042ad2b(uVar2,uVar3);
    }
    local_d8 = DAT_0067cf38;
    FUN_004431e4(0);
    FUN_004470b9();
  }
  else {
    ExceptionList = &local_10;
    local_100 = (**(code **)(*param_1 + 0xb4))(param_2,param_3);
    local_f4 = FUN_0054cd07(*(undefined4 *)(DAT_005ccec0[0x7a] + 0x670),0x26);
    local_f8 = -1;
    if (local_100 != 0) {
      local_f8 = FUN_0054cd07(*(undefined4 *)(local_100 + 0x670),0x26);
    }
    local_f0 = (**(code **)(*param_1 + 0xc0))(param_1[0x79],param_1[0x100]);
    local_fc = (**(code **)(*param_1 + 0xc4))(param_1[0x79]);
    if (((((((((local_f4 < 4) || (6 < local_f4)) && (local_f4 != 9)) &&
            ((local_f4 != 0xf && (local_f4 != 0x10)))) && ((local_f4 < 0xb4 || (0xb7 < local_f4))))
          && ((((local_f4 < 0x5a || (0x5e < local_f4)) && (local_f4 != 0xb3)) &&
              ((local_f4 != 0xba && (local_f4 != 0xc4)))))) || (local_f0 != 0)) &&
        (((local_f4 != 0xbe || (local_f0 != 1)) || (param_1[0x15e] != 0x330000ff)))) ||
       ((local_f8 != 0 || (local_100 == 0)))) {
      if (((local_f4 == 0xbe) && ((local_f0 == 1 && (param_1[0x15e] != 0x330000ff)))) &&
         (local_100 != 0)) {
        local_150 = DAT_0067cf38;
        FUN_00403df2(&DAT_00a3d8f8,3000);
        (**(code **)(*(int *)local_150[0x21] + 0x88))(1,1);
      }
      else {
        iVar1 = (**(code **)(*param_1 + 0x9c))(DAT_005ccec0[0x7a],param_2,param_3,0);
        if (iVar1 != 0) {
          DAT_005ccf08 = DAT_005ccec0[0x7a];
          FUN_0040c110(local_174);
          uVar2 = __ftol();
          FUN_0040c110(local_17c);
          uVar2 = __ftol(uVar2);
          (**(code **)(*DAT_0067cf38 + 8))(param_4,param_5,uVar2);
        }
      }
    }
    else {
      local_140 = DAT_0067cf38;
      local_104 = (**(code **)(*DAT_0092e654 + 8))();
      if ((local_140[0x9d1e] != 0) && ((uint)(local_104 - local_140[0x9d1e]) < 200)) {
        ExceptionList = local_10;
        return 1;
      }
      local_144 = (**(code **)(*param_1 + 0xc0))
                            (*(undefined4 *)(*(int *)(DAT_005ccec0[0x7a] + 0x1e4) + 0x1e4),
                             *(undefined4 *)(*(int *)(DAT_005ccec0[0x7a] + 0x1e4) + 0x400));
      local_108 = (**(code **)(*param_1 + 0xc4))
                            (*(undefined4 *)(*(int *)(DAT_005ccec0[0x7a] + 0x1e4) + 0x1e4));
      if (local_108 == -1) {
        local_108 = (short)*(undefined4 *)(DAT_005ccec0[0x7a] + 0x1d0) +
                    (short)*(undefined4 *)(DAT_005ccec0[0x7a] + 0x1d4) * 9;
      }
      if (local_fc == -1) {
        local_fc = (short)*(undefined4 *)(local_100 + 0x1d0) +
                   (short)*(undefined4 *)(local_100 + 0x1d4) * 9;
      }
      _memset(local_12c,0,0x24);
      local_126 = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
      local_128 = 0x373;
      local_120 = (int)local_144;
      local_11c = (int)local_108;
      local_118 = (int)local_f0;
      local_114 = (int)local_fc;
      local_10c = 0;
      local_110 = 0;
      local_10e = 0;
      FUN_0055f2dd(local_12c,0x24);
      local_140[0x9d1e] = local_104;
      local_130 = *(undefined4 *)(DAT_005ccec0[0x7a] + 0x1d0);
      local_138 = *(undefined4 *)(DAT_005ccec0[0x7a] + 0x1d4);
      local_134 = *(int **)(DAT_005ccec0[0x7a] + 0x1e4);
      local_148 = (undefined4 *)(**(code **)(*local_134 + 0xb4))(local_130,local_138);
      local_13c = 0;
      if (local_148 != (undefined4 *)0x0) {
        local_13c = FUN_0054e93b(local_148[0x19c]);
        if ((0x919 < *(short *)local_148[0x19c]) && (*(short *)local_148[0x19c] < 0x956)) {
          local_13c = 0;
        }
        if (local_13c < 2) {
          (**(code **)(*local_134 + 0xa4))(local_130,local_138);
          if ((DAT_005ccec0[0x7a] != 0) && ((undefined4 *)DAT_005ccec0[0x7a] == local_148)) {
            DAT_005ccec0[0x7a] = 0;
          }
          if (local_148 != (undefined4 *)0x0) {
            local_16c = local_148;
            local_168 = local_148;
            if (local_148 != (undefined4 *)0x0) {
              (**(code **)*local_148)(1);
            }
            local_148 = (undefined4 *)0x0;
          }
        }
        else {
          FUN_0054e9ab(local_148[0x19c],local_13c + -1);
          local_14c = local_148;
          FUN_0058f078((int)local_148 + 0x39a,&PTR_DAT_005b12cc,local_13c + -1);
          FUN_00421b8f((int)local_14c + 0x39a,local_14c[0xe1],0);
        }
      }
      (**(code **)(*DAT_005ccec0 + 0x98))();
      if (local_13c < 2) {
        if (local_144 == 0) {
          _memset((void *)(DAT_013b71e8 + 0x748 + local_108 * 8),0,8);
        }
        else if (local_144 == 1) {
          _memset((void *)(DAT_013b71e8 + 0x7c8 + local_108 * 8),0,8);
        }
        else if (local_144 == 2) {
          _memset((void *)(DAT_013b71e8 + 0x2ec + local_108 * 8),0,8);
        }
      }
    }
  }
  ExceptionList = local_10;
  return 2;
}

