// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004209fc | Name: FUN_004209fc


int __thiscall
FUN_004209fc(int *param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  int *piVar1;
  int iVar2;
  uint uVar3;
  int local_154;
  char local_150;
  int local_14c;
  undefined1 local_148 [4];
  undefined2 local_144;
  undefined2 local_142;
  undefined1 local_13c;
  char cStack_13b;
  char cStack_13a;
  char cStack_139;
  undefined1 uStack_138;
  undefined1 uStack_137;
  undefined2 local_134 [4];
  int local_12c;
  undefined2 local_128 [4];
  int local_120;
  undefined1 local_11c [4];
  undefined2 local_118;
  undefined2 local_116;
  undefined1 local_110;
  char cStack_10f;
  char cStack_10e;
  char cStack_10d;
  undefined1 uStack_10c;
  undefined1 uStack_10b;
  int local_108;
  int local_104;
  short local_100;
  int local_fc;
  undefined4 local_f8;
  undefined4 local_f4;
  short local_f0;
  undefined1 local_ec [128];
  int local_6c;
  int local_68;
  int local_64;
  int local_60;
  int local_5c;
  int local_58;
  int local_50;
  int local_4c;
  int local_48;
  int local_44;
  undefined4 local_40;
  undefined1 local_3c [8];
  undefined4 local_34;
  undefined4 local_30;
  undefined1 local_2c [8];
  undefined4 local_24;
  int local_20;
  undefined4 local_1c;
  int local_18;
  int local_14;
  int local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  if (param_1[0xf] == 0) {
    return 0;
  }
  if (*(short *)(DAT_013b71e8 + 0x738) < 1) {
    return 0;
  }
  if (param_1[0xb] == 0) {
    return 0;
  }
  local_c = __ftol();
  local_8 = __ftol();
  local_18 = __ftol();
  local_1c = __ftol();
  local_14 = FUN_00401000(param_4,param_5,param_1[0x13],param_1[0x14],param_1[0x15],param_1[0x16]);
  local_10 = 0;
  if (local_14 == 0) {
    for (local_20 = 0; local_20 < param_1[0x7f]; local_20 = local_20 + 1) {
      *(undefined4 *)(param_1[local_20 + 0x80] + 0x34) = 0;
    }
  }
  if (param_2 == 0x201) {
    _memset(local_2c,0,8);
    local_24 = 0;
    if (DAT_005ccec0[0x7a] != 0) {
      if ((((*(int *)(DAT_005ccec0[0x7a] + 0x1d0) < 0) ||
           (0x10 < *(int *)(DAT_005ccec0[0x7a] + 0x1d0))) ||
          (*(int *)(DAT_005ccec0[0x7a] + 0x1d4) < 0)) ||
         (0x10 < *(int *)(DAT_005ccec0[0x7a] + 0x1d4))) {
        return 0;
      }
      FUN_0058f220(local_2c,*(undefined4 *)(DAT_005ccec0[0x7a] + 0x670),8);
      local_24 = FUN_0054cd07(local_2c,0x11);
    }
    local_30 = FUN_0054cd07(local_2c,0x12);
    local_10 = 1;
    iVar2 = (**(code **)(*DAT_005ccec0 + 0x90))();
    if ((iVar2 == 0) && (local_14 != 0)) {
      uVar3 = (**(code **)(*DAT_0092e654 + 8))();
      if (uVar3 < param_1[0x102] + 500U) {
        return 0;
      }
      if (((((param_1[0x100] == 8) || (param_1[0x100] == 6)) ||
           ((param_1[0x100] == 9 ||
            (((param_1[0x100] == 7 || (param_1[0x100] == 10)) || (param_1[0x100] == 0xb)))))) ||
          ((param_1[0x100] == 0xd || (param_1[0x100] == 0xf)))) || (param_1[0x100] == 0x11)) {
        return 0;
      }
      FUN_00410a91(local_18,local_1c);
      iVar2 = (**(code **)(*DAT_0092e654 + 8))();
      param_1[0x102] = iVar2;
    }
    goto LAB_00420f67;
  }
  if (param_2 != 0x202) goto LAB_00420f67;
  local_44 = DAT_0067cf38;
  _memset(local_3c,0,8);
  local_34 = 0;
  if (DAT_005ccec0[0x7a] != 0) {
    FUN_0058f220(local_3c,*(undefined4 *)(DAT_005ccec0[0x7a] + 0x670),8);
    local_34 = FUN_0054cd07(local_3c,0x11);
  }
  local_40 = FUN_0054cd07(local_3c,0x12);
  iVar2 = (**(code **)(*DAT_005ccec0 + 0x90))();
  if (iVar2 == 0) {
    if (local_14 != 0) {
      if (param_1[0x100] == 5) {
        local_4c = DAT_0067cf38;
        (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x40))(0);
        if (local_18 < 9) {
          FUN_004528c5(local_18 + 0x31,0);
        }
        else {
          FUN_004528c5(0x30,0);
        }
        return 1;
      }
      if ((param_1[0x100] == 8) || (param_1[0x100] == 9)) {
        return 1;
      }
      iVar2 = FUN_004110f5(local_18,local_1c);
      local_48 = iVar2;
joined_r0x00420ec9:
      if (iVar2 != 2) {
        return iVar2;
      }
    }
  }
  else if ((((local_14 != 0) && (iVar2 = (**(code **)(*DAT_005ccec0 + 0x90))(), iVar2 == 2)) &&
           (DAT_005ccf08 == 0)) && (DAT_005ccec0[0x7a] != 0)) {
    iVar2 = FUN_00416196(local_18,local_1c,0x202,param_3);
    local_50 = iVar2;
    goto joined_r0x00420ec9;
  }
  if (DAT_005ccec0[0x7a] != 0) {
    FUN_0058f220(local_3c,*(undefined4 *)(DAT_005ccec0[0x7a] + 0x670),8);
    local_34 = FUN_0054cd07(local_3c,0x11);
  }
  if (((local_14 != 0) && (iVar2 = (**(code **)(*DAT_005ccec0 + 0x90))(), iVar2 == 2)) &&
     ((DAT_005ccf08 != 0 && (DAT_005ccec0[0x7a] != 0)))) {
    FUN_00416e8a(local_18,local_1c,local_c,local_8,local_3c);
  }
LAB_00420f67:
  if ((local_10 == 0) && (param_2 == 0x200)) {
    iVar2 = FUN_00418828(local_18,local_1c,local_14);
    if (iVar2 != 2) {
      return iVar2;
    }
  }
  else if (param_2 == 0x204) {
    FUN_0041ef0f(local_18,local_1c,local_14);
  }
  else if (((((local_10 == 0) && (param_2 == 0x205)) && (local_14 != 0)) &&
           (iVar2 = (**(code **)(*DAT_005ccec0 + 0x90))(), iVar2 == 0)) &&
          ((param_1[0x100] == 2 || (param_1[0x100] == 0)))) {
    local_68 = DAT_0067cf38;
    if ((DAT_0067cf38 == 0) || (*(int *)(DAT_0067cf38 + 0x4c) == 0)) {
      return 0;
    }
    local_58 = *(int *)(DAT_0067cf38 + 0x4c);
    if ((((param_1[0x100] == 0) &&
         ((*(short *)(local_58 + 0x7ae) == 0xf4a || (*(short *)(local_58 + 0x7ae) == 0xf4b)))) &&
        (*(int *)(DAT_005d0504 + 0x440) == 1)) &&
       ((local_6c = (**(code **)(*param_1 + 0xac))(local_18,local_1c), local_6c != 0 &&
        (*(int *)(*(int *)(local_6c + 0x1e4) + 0x1e4) == 0)))) {
      DAT_005ccf10 = local_6c;
      FUN_0058f078(local_ec,&DAT_00a3ec78,&DAT_00d44950 + **(short **)(local_6c + 0x670) * 0x8c);
      (**(code **)(**(int **)(local_68 + 0x8c) + 0x8c))(local_ec,0x37a,&DAT_00a3ecf8);
      (**(code **)(**(int **)(local_68 + 0x8c) + 0x60))(1);
      return 1;
    }
    if (*(int *)(DAT_005d0504 + 0x438) == 0) {
      return 0;
    }
    for (local_5c = 0; local_5c < 0x14; local_5c = local_5c + 1) {
      if (*(char *)(DAT_013b71e8 + local_5c + 0xef6) == 'T') {
        return 0;
      }
    }
    local_60 = (**(code **)(*param_1 + 0xac))(local_18,local_1c);
    local_64 = DAT_0067cf38;
    if ((local_60 != 0) && (*(int *)(local_60 + 0x94) != -0x10000)) {
      local_fc = FUN_0054cd07(*(undefined4 *)(local_60 + 0x670),0x11);
      local_108 = 0;
      while (local_fc = local_fc / 2, local_fc != 0) {
        local_108 = local_108 + 1;
      }
      local_fc = FUN_0054cd07(*(undefined4 *)(local_60 + 0x670),0x11);
      if (local_fc == 0) {
        return 0;
      }
      local_f0 = (**(code **)(*param_1 + 0xc0))
                           (*(undefined4 *)(*(int *)(local_60 + 0x1e4) + 0x1e4),
                            *(undefined4 *)(*(int *)(local_60 + 0x1e4) + 0x400));
      local_100 = (**(code **)(*param_1 + 0xc4))
                            (*(undefined4 *)(*(int *)(local_60 + 0x1e4) + 0x1e4));
      local_f4 = *(undefined4 *)(local_60 + 0x1d0);
      local_f8 = *(undefined4 *)(local_60 + 0x1d4);
      local_104 = DAT_013b71e8 + 0x6ec;
      if (local_100 == -1) {
        iVar2 = FUN_00553016(*(undefined4 *)(local_60 + 0x670),DAT_013b71e8 + 0x72c,0xffffffff,
                             (int)*(short *)(DAT_013b71e8 + 0x748),DAT_013b71e8 + 0x748,
                             (int)*(short *)(DAT_013b71e8 + 0xc4 +
                                            *(char *)(DAT_013b71e8 + 0xef4) * 0x80));
        if (iVar2 == 0) {
          return 0;
        }
        if ((0x3f < local_fc) && (local_fc < 0xc1)) {
          _memset(local_134,0,8);
          _memset(local_128,0,8);
          local_134[0] = *(undefined2 *)(*(int *)(DAT_0067cf38 + 0x4c) + 0x66c);
          local_128[0] = *(undefined2 *)(*(int *)(DAT_0067cf38 + 0x4c) + 0x66e);
          local_12c = FUN_0054cd07(local_134,0x11);
          local_120 = FUN_0054cd07(local_128,0x11);
          if ((local_12c == 0x40) && ((local_120 == 0x80 && (local_fc != 0x80)))) {
            local_108 = 6;
          }
          if ((local_12c == 0x40) && (local_fc == 0xc0)) {
            local_108 = 6;
          }
          if ((((local_12c == 0xc0) && (local_120 == 0xc0)) && (local_fc != 0x80)) &&
             (local_fc != 0xc0)) {
            return 0;
          }
          if ((local_120 == 0x80) && (local_108 == 7)) {
            local_108 = 6;
          }
          iVar2 = FUN_00553016(*(undefined4 *)(local_60 + 0x670),local_104 + 0x40,6,
                               (int)*(short *)(local_104 + 0x5c),local_104 + 0x5c,
                               (int)*(short *)(DAT_013b71e8 + 0xc4 +
                                              *(char *)(DAT_013b71e8 + 0xef4) * 0x80));
          if ((iVar2 == 0) && (local_fc != 0x80)) {
            return 0;
          }
        }
        _memset(local_11c,0,0x14);
        local_116 = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
        local_118 = 0x376;
        local_110 = 0;
        cStack_10f = (char)local_108;
        cStack_10e = (char)local_f0;
        uStack_10c = (undefined1)DAT_005d03e8;
        uStack_10b = (undefined1)((ushort)DAT_005d03e8 >> 8);
        if (local_f0 == 0) {
          cStack_10d = (char)local_100;
        }
        else {
          cStack_10d = (char)local_f8 * '\t' + (char)local_f4;
        }
        if ((cStack_10d != (char)local_108) || ((char)local_f0 != '\0')) {
          FUN_0055f2dd(local_11c,0x14);
          *(undefined4 *)(local_64 + 0x7c) = 0;
        }
      }
      else {
        piVar1 = *(int **)(local_64 + 0x2873c);
        local_14c = FUN_0054cd07(*(undefined4 *)(local_60 + 0x670),0x21);
        if ((7 < local_14c) || (local_14c < 0)) {
          local_14c = 0;
        }
        (**(code **)(*piVar1 + 0x98))
                  (&local_154,*(undefined4 *)(&DAT_005bec0c + local_14c * 8),
                   *(undefined4 *)(&DAT_005bec10 + local_14c * 8));
        if (local_154 == -1) {
          return 0;
        }
        _memset(local_148,0,0x14);
        local_142 = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
        local_144 = 0x376;
        local_13c = 1;
        cStack_13b = local_150 * '\t' + (char)local_154;
        cStack_13a = (char)local_f0;
        cStack_139 = (char)local_100;
        uStack_138 = (undefined1)DAT_005d03e8;
        uStack_137 = (undefined1)((ushort)DAT_005d03e8 >> 8);
        if (((char)local_100 != cStack_13b) || ((char)local_f0 != '\x01')) {
          FUN_0055f2dd(local_148,0x14);
          *(undefined4 *)(local_64 + 0x7c) = 0;
        }
      }
    }
  }
  return 0;
}

