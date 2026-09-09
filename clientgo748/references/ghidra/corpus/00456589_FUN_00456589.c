// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00456589 | Name: FUN_00456589


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall
FUN_00456589(int *param_1,undefined4 param_2,undefined4 param_3,float param_4,undefined4 param_5,
            float param_6,undefined4 param_7,undefined4 param_8,int *param_9)

{
  undefined2 uVar1;
  short sVar2;
  int iVar3;
  uint uVar4;
  void *pvVar5;
  uint uVar6;
  int iVar7;
  int iVar8;
  float10 fVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  int local_32c;
  float local_30c;
  int local_308;
  int local_304;
  int *local_2d8;
  int local_2d4;
  int local_2d0;
  undefined1 local_2c4 [4];
  undefined4 local_2c0;
  undefined2 local_2b8;
  undefined2 local_2b6;
  undefined2 local_2b4;
  undefined2 local_2b2;
  undefined2 local_2b0;
  undefined2 local_2ae;
  short local_2ac;
  undefined2 local_2aa;
  undefined1 local_2a8;
  undefined1 local_2a7;
  undefined1 local_2a6;
  undefined2 local_298 [26];
  undefined1 local_264 [6];
  undefined2 local_25e;
  undefined1 local_246;
  byte local_245;
  int *local_204;
  uint local_200;
  undefined4 local_1fc;
  int local_1f8;
  int local_1f4;
  int local_1f0;
  int local_1ec;
  int local_1e8;
  int local_1e4;
  int local_1e0;
  uint local_1dc;
  int local_1d8;
  int local_1d4;
  int local_1d0;
  int local_1cc;
  int local_1c8;
  int local_1c4;
  undefined1 local_1c0 [6];
  undefined2 local_1ba;
  undefined1 local_1a2;
  byte local_1a1;
  int local_160;
  undefined4 local_15c;
  int local_158;
  undefined2 local_154;
  int local_150;
  uint local_14c;
  undefined4 local_148;
  undefined4 local_144;
  uint local_140;
  int local_13c;
  int local_138;
  int local_134;
  int local_130;
  int local_12c;
  int local_128;
  int local_124;
  int local_120;
  int local_11c;
  int local_118;
  int local_114;
  int local_110;
  uint local_10c;
  uint local_108;
  int local_104;
  uint local_100;
  int *local_fc;
  int local_f8;
  int local_f4;
  int local_f0;
  float local_ec;
  float local_e8;
  float local_e4;
  int local_e0;
  int local_dc;
  int local_d8;
  int local_d4;
  int local_d0;
  float local_cc;
  uint local_c8;
  float local_c4;
  int local_c0;
  uint local_bc;
  int local_b8;
  undefined4 local_b4;
  undefined4 local_b0;
  float local_ac;
  int local_a8;
  int local_a4;
  int local_a0;
  int local_9c;
  int local_98;
  undefined1 local_94 [4];
  undefined4 local_90;
  undefined2 local_88;
  undefined2 local_86;
  undefined2 local_84;
  undefined2 local_82;
  undefined2 local_80;
  undefined2 local_7e;
  short local_7c;
  undefined2 local_7a;
  undefined1 local_78;
  undefined1 local_77;
  undefined1 local_76;
  undefined4 local_68;
  uint local_34;
  int local_30;
  int *local_2c;
  int local_28;
  int *local_24;
  int local_20;
  int local_1c;
  char local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f643;
  local_10 = ExceptionList;
  if (*(char *)(param_1[0x13] + 0x795) == '\x01') {
    return 0;
  }
  if (*(int *)(param_1[0xa304] + 0x28) == 1) {
    return 0;
  }
  if (*(int *)(DAT_005d0504 + 0x434) == 1) {
    return 0;
  }
  if ((uint)param_9[8] < 1000) {
    return 0;
  }
  ExceptionList = &local_10;
  iVar3 = FUN_00516105();
  if (iVar3 != 0) {
    ExceptionList = local_10;
    return 0;
  }
  if (*(char *)((int)param_9 + 0x249) == '\x01') {
    ExceptionList = local_10;
    return 0;
  }
  if (*(char *)((int)param_9 + 0x245) == '\x01') {
    ExceptionList = local_10;
    return 0;
  }
  iVar3 = (**(code **)(*(int *)param_1[0x13] + 0x50))();
  if (iVar3 == 1) {
    ExceptionList = local_10;
    return 0;
  }
  if ((*(int *)(param_1[0x13] + 0x740) == 2) && (*(int *)(param_1[0x13] + 0x740) == 3)) {
    ExceptionList = local_10;
    return 0;
  }
  uVar4 = (**(code **)(**(int **)(DAT_013b71e0 + 0xf4) + 8))();
  param_1[0x9ba6] = 0;
  local_20 = 0;
  local_2c = DAT_013b71e8 + 0x1bb;
  if ((local_2c != (int *)0x0) && ((DAT_013b71e8[0x272] & 0x20000000U) != 0)) {
    local_20 = 1;
  }
  local_24 = param_9;
  if (param_9 == (int *)0x0) {
    ExceptionList = local_10;
    return 0;
  }
  local_18 = *(char *)((int)DAT_013b71e8 + *(char *)((int)DAT_013b71e8 + 0xef5) + 0xef6);
  local_14 = 0xffffffff;
  iVar3 = FUN_00434228((int)local_18);
  if (iVar3 == 0) {
    ExceptionList = local_10;
    return 0;
  }
  local_28 = *(int *)(&DAT_0092ff04 + local_18 * 0x60);
  if ((8 < param_1[0x9b9e]) && (1 < local_28)) {
    local_28 = local_28 + -1;
  }
  if (uVar4 < param_1[0x9d18] + 1000U) {
    ExceptionList = local_10;
    return 0;
  }
  if (uVar4 < (uint)(*(int *)(&DAT_0092eaf8 + local_18 * 4) + local_28 * 1000)) {
    ExceptionList = local_10;
    return 0;
  }
  local_1c = *(int *)(&DAT_0092fefc + local_18 * 0x60);
  if ((((local_1c != 0) && (local_1c != 3)) && (local_1c != 4)) &&
     ((local_1c != 5 && (local_1c != 6)))) goto LAB_0045819d;
  local_34 = (uint)*(short *)(param_1[0x13] + 0x444);
  local_30 = (int)local_18 + (uint)*(byte *)(DAT_013b71e8 + 0x1c0) * -0x18;
  if (local_18 < '`') {
    local_34 = (uint)*(byte *)((int)DAT_013b71e8 +
                              ((int)(local_30 + (local_30 >> 0x1f & 7U)) >> 3) + 0x745);
  }
  iVar3 = FUN_00555790((int)local_18,*(undefined1 *)((int)DAT_013b71e8 + 0x9d3),local_34);
  if (*(short *)((int)DAT_013b71e8 + 0x73a) < iVar3) {
    local_98 = param_1[0x9eba];
    pvVar5 = operator_new(0xe50);
    local_8 = 0;
    if (pvVar5 == (void *)0x0) {
      local_304 = 0;
    }
    else {
      local_304 = FUN_00407203(&DAT_00a35078,0xffffaaaa,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
    }
    local_8 = 0xffffffff;
    local_9c = local_304;
    if (local_304 != 0) {
      FUN_00408d33(local_304);
    }
    if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x21), iVar3 != 0)) {
      uVar11 = 0;
      uVar10 = 0;
      FUN_00429a6d(0x21);
      FUN_0042ad2b(uVar10,uVar11);
    }
    ExceptionList = local_10;
    return 0;
  }
  if ((local_18 == 'V') &&
     (iVar3 = (uint)*(byte *)(param_1[0x13] + 0x45e) * 100,
     iVar3 - DAT_013b71e8[0x1c1] != 0 && DAT_013b71e8[0x1c1] <= iVar3)) {
    local_a0 = param_1[0x9eba];
    pvVar5 = operator_new(0xe50);
    local_8 = 1;
    if (pvVar5 == (void *)0x0) {
      local_308 = 0;
    }
    else {
      local_308 = FUN_00407203(&DAT_00a38ef8,0xffffaaaa,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
    }
    local_8 = 0xffffffff;
    local_a4 = local_308;
    if (local_308 != 0) {
      FUN_00408d33(local_308);
    }
    if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x21), iVar3 != 0)) {
      uVar11 = 0;
      uVar10 = 0;
      FUN_00429a6d(0x21);
      FUN_0042ad2b(uVar10,uVar11);
    }
    ExceptionList = local_10;
    return 0;
  }
  _memset(local_94,0,0x60);
  local_90 = CONCAT22(*(undefined2 *)(param_1[0x13] + 0x20),0x36c);
  local_88 = *(undefined2 *)(param_1[0x13] + 0x20);
  local_84 = __ftol();
  local_82 = __ftol();
  local_80 = __ftol();
  local_7e = __ftol();
  if (param_1[0xa1e5] != 0) {
    local_84 = (undefined2)param_1[0xa1e5];
    local_82 = (undefined2)param_1[0xa1e6];
    local_80 = local_84;
    local_7e = local_82;
  }
  local_7a = 0xffff;
  local_7c = (short)local_18;
  local_77 = 0;
  local_78 = 0xff;
  if (*(int *)(&DAT_0092fefc + local_18 * 0x60) == 5) {
    local_76 = 0;
    local_a8 = 0;
    local_bc = (uint)*(byte *)((int)DAT_013b71e8 + 0x9d2);
    local_b8 = *(int *)(param_1[0xd] + 0x10);
    if (local_b8 == 0) {
      ExceptionList = local_10;
      return 0;
    }
    local_c4 = param_4 - *(float *)(param_1[0x13] + 0x28);
    local_cc = param_6 - *(float *)(param_1[0x13] + 0x2c);
    fVar9 = (float10)FUN_00493c30(local_c4,local_cc);
    local_ac = (float)fVar9;
    local_c8 = *(byte *)(param_1[0x13] + 0x45f) / 0x4b;
    if (3 < local_c8) {
      local_c8 = 3;
    }
    local_b0 = __ftol();
    local_b4 = __ftol();
    local_c0 = 0;
LAB_00456cac:
    do {
      while( true ) {
        if ((local_b8 == 0) || (1000 < local_c0)) goto LAB_00457ff3;
        local_c0 = local_c0 + 1;
        if (local_b8 != param_1[0x13]) break;
        local_b8 = *(int *)(local_b8 + 0xc);
      }
      local_d0 = __ftol();
      local_d4 = __ftol();
      local_d8 = __ftol();
      local_dc = __ftol();
      if (param_1[0xa1e5] != 0) {
        local_d8 = param_1[0xa1e5];
        local_dc = param_1[0xa1e6];
      }
      local_e0 = FUN_005563a0(local_d8,local_dc,local_d0,local_d4);
      if (((*(int *)(local_b8 + 0x24c) == 0x38) && (*(short *)(local_b8 + 0x1d2) == 0)) &&
         (local_e0 = local_e0 + -0xc, local_e0 < 0)) {
        local_e0 = 0;
      }
      if (local_e0 <= (int)(local_c8 + 3)) {
        local_e4 = *(float *)(local_b8 + 0x28) - *(float *)(param_1[0x13] + 0x28);
        local_e8 = *(float *)(local_b8 + 0x2c) - *(float *)(param_1[0x13] + 0x2c);
        fVar9 = (float10)FUN_00493c30(local_e4,local_e8);
        local_ec = (float)fVar9;
        if (local_ec - local_ac <= _DAT_005a3534) {
          local_30c = -(local_ec - local_ac);
        }
        else {
          local_30c = local_ec - local_ac;
        }
        if (local_30c < _DAT_005a437c) {
          local_f4 = local_d0;
          local_f0 = local_d4;
          FUN_00556416(local_d8,local_dc,&local_f4,&local_f0,param_1 + 0x25,8);
          if ((*(int *)(local_b8 + 0x24c) == 0x38) && (*(short *)(local_b8 + 0x1d2) == 0)) {
            local_f4 = local_d0;
            local_f0 = local_d4;
          }
          if ((local_f4 != local_d0) || (local_f0 != local_d4)) {
            local_b8 = *(int *)(local_b8 + 0xc);
            goto LAB_00456cac;
          }
          if (*(char *)(local_b8 + 0x245) == '\x01') {
            local_b8 = *(int *)(local_b8 + 0xc);
            goto LAB_00456cac;
          }
          *(undefined2 *)(&local_68 + local_a8) = *(undefined2 *)(local_b8 + 0x20);
          *(undefined2 *)((int)&local_68 + local_a8 * 4 + 2) = 0xffff;
          local_a8 = local_a8 + 1;
          local_80 = (undefined2)local_d0;
          local_7e = (undefined2)local_d4;
        }
      }
      local_b8 = *(int *)(local_b8 + 0xc);
      if ((*(int *)(&DAT_0092ff44 + local_18 * 0x60) <= local_a8) || (0xc < local_a8))
      goto LAB_00457ff3;
    } while( true );
  }
  if (((*(int *)(&DAT_0092fefc + local_18 * 0x60) == 3) ||
      (*(int *)(&DAT_0092fefc + local_18 * 0x60) == 4)) ||
     (*(int *)(&DAT_0092fefc + local_18 * 0x60) == 6)) {
    local_76 = 0;
    local_f8 = 0;
    local_100 = (uint)*(byte *)((int)DAT_013b71e8 + 0x9d2);
    local_fc = *(int **)(param_1[0xd] + 0x10);
    if (local_fc == (int *)0x0) {
      ExceptionList = local_10;
      return 0;
    }
    local_108 = __ftol();
    local_10c = __ftol();
    if (((local_108 == 0) && (local_10c == 0)) && (local_24 != (int *)0x0)) {
      local_108 = __ftol();
      local_10c = __ftol();
    }
    local_104 = 0;
LAB_004570e6:
    while( true ) {
      while( true ) {
        if ((local_fc == (int *)0x0) || (1000 < local_104)) goto LAB_00457fdd;
        local_104 = local_104 + 1;
        local_110 = __ftol();
        local_118 = __ftol();
        if ((local_18 == '\0') ||
           ((((((local_18 == '\x02' || (local_18 == '\a')) || (local_18 == '\x11')) ||
              ((local_18 == '\x17' || (local_18 == '#')))) || (local_18 == '\'')) ||
            ((local_18 == '3' || (local_18 == '7')))))) {
          if (local_24 == (int *)0x0) {
            ExceptionList = local_10;
            return 0;
          }
          local_108 = __ftol();
          local_108 = local_108 & 0xffff;
          local_10c = __ftol();
          local_10c = local_10c & 0xffff;
          if ((local_24[0x93] == 0x38) && (*(short *)((int)local_24 + 0x1d2) == 0)) {
            local_134 = __ftol();
            local_138 = __ftol();
            if (param_1[0xa1e5] != 0) {
              local_134 = param_1[0xa1e5];
              local_138 = param_1[0xa1e6];
            }
            uVar10 = __ftol();
            uVar10 = __ftol(uVar10);
            local_13c = FUN_005563a0(local_134,local_138,uVar10);
            local_13c = local_13c + -0xc;
            if (local_13c < 1) {
              local_108 = __ftol();
              local_10c = __ftol();
            }
          }
        }
        if ((local_18 != '_') || (local_fc == local_24)) break;
        if (local_24 == (int *)0x0) {
          ExceptionList = local_10;
          return 0;
        }
        local_fc = (int *)local_fc[3];
        local_108 = __ftol();
        local_10c = __ftol();
      }
      if (((local_fc == (int *)0x0) ||
          (((short)local_fc[0x1a0] == 0 || (*(int *)(&DAT_0092ff40 + local_18 * 0x60) != 1)))) ||
         ((*(short *)(param_1[0x13] + 0x680) != (short)local_fc[0x1a0] &&
          (*(short *)((int)DAT_013b71e8 + 0xf12) != (short)local_fc[0x1a0])))) break;
      local_fc = (int *)local_fc[3];
    }
    local_120 = __ftol();
    local_124 = __ftol();
    if (param_1[0xa1e5] != 0) {
      local_120 = param_1[0xa1e5];
      local_124 = param_1[0xa1e6];
    }
    local_128 = FUN_005563a0(local_120,local_124,local_108,local_10c);
    if (((local_18 == '\0') && (local_128 == 2)) &&
       ((uVar6 = __ftol(), local_108 == uVar6 || (uVar6 = __ftol(), local_10c == uVar6)))) {
      local_128 = 1;
    }
    if ((((local_24 != (int *)0x0) && (local_24[0x93] == 0x38)) &&
        (*(short *)((int)local_24 + 0x1d2) == 0)) && (local_128 = local_128 + -0xc, local_128 < 0))
    {
      local_128 = 0;
    }
    if ((((local_18 == '\0') || (local_18 == '#')) ||
        (((local_18 == '\x02' || ((local_18 == '\a' || (local_18 == '\x11')))) ||
         (local_18 == '\x17')))) ||
       ((((local_18 == '\'' || (local_18 == '3')) || (local_18 == '7')) || (local_18 == '_')))) {
      if ((*(int *)(&DAT_0092ff08 + local_18 * 0x60) + local_20 < local_128) &&
         (*(int *)(&DAT_0092ff08 + local_18 * 0x60) != -1)) {
        ExceptionList = local_10;
        return 0;
      }
      if (*(int *)(&DAT_0092ff08 + local_18 * 0x60) == -1) {
        local_150 = FUN_0054eb20(DAT_013b71e8 + 0x1bb,0x1b);
        local_150 = local_150 + local_20;
        if (local_150 < local_128) {
          ExceptionList = local_10;
          return 0;
        }
      }
      local_144 = __ftol();
      local_148 = __ftol();
      local_14c = local_108;
      local_140 = local_10c;
      FUN_00556416(local_144,local_148,&local_14c,&local_140,param_1 + 0x25,8);
      if (((local_24 != (int *)0x0) && (local_24[0x93] == 0x38)) &&
         (*(short *)((int)local_24 + 0x1d2) == 0)) {
        local_14c = local_108;
        local_140 = local_10c;
      }
      if ((local_14c != local_108) || (local_140 != local_10c)) {
        ExceptionList = local_10;
        return 0;
      }
    }
    else {
      local_108 = __ftol();
      local_108 = local_108 & 0xffff;
      local_10c = __ftol();
      local_10c = local_10c & 0xffff;
      if (param_1[0xa1e5] != 0) {
        local_108 = param_1[0xa1e5];
        local_10c = param_1[0xa1e6];
      }
    }
    local_130 = FUN_005563a0(local_108,local_10c,local_110,local_118);
    local_114 = local_110;
    local_11c = local_118;
    FUN_00556927(local_108,local_10c,&local_114,&local_11c,param_1 + 0x25,8);
    local_12c = -1;
    if (*(int *)(&DAT_0092fefc + local_18 * 0x60) == 3) {
      local_12c = 1;
    }
    else if (*(int *)(&DAT_0092fefc + local_18 * 0x60) == 4) {
      local_12c = 2;
    }
    else if (*(int *)(&DAT_0092fefc + local_18 * 0x60) == 6) {
      local_12c = 3;
    }
    iVar3 = (**(code **)(*local_fc + 0x54))();
    if ((iVar3 == 1) && (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)) {
      if ((char)local_fc[0x92] == '\x01') {
        local_fc = (int *)local_fc[3];
        goto LAB_004570e6;
      }
      if (DAT_005d0404 == 0) {
        if ((uint)local_fc[8] < 1000) {
          if ((short)DAT_013b71e8[0x3c4] != (short)local_fc[0x1a0]) {
            if (DAT_005d03f8 == 0) {
              local_fc = (int *)local_fc[3];
              goto LAB_004570e6;
            }
            if ((*(char *)(param_1[0x13] + 0x79b) != '\0') &&
               (*(char *)((int)local_fc + 0x79b) == *(char *)(param_1[0x13] + 0x79b))) {
              local_fc = (int *)local_fc[3];
              goto LAB_004570e6;
            }
          }
          if ((((short)DAT_013b71e8[0x3c4] != (short)local_fc[0x1a0]) &&
              (*(char *)((int)local_fc + 0x79b) == '\0')) && (DAT_005d03f8 == 0)) {
            local_fc = (int *)local_fc[3];
            goto LAB_004570e6;
          }
        }
        if (*(char *)((int)local_fc + 0x249) == '\x01') {
          if (((short)DAT_013b71e8[0x3c4] != (short)local_fc[0x1a0]) &&
             ((short)local_fc[0x1a0] != 0)) {
            if (DAT_005d03f8 == 0) {
              local_fc = (int *)local_fc[3];
              goto LAB_004570e6;
            }
            if ((('\0' < *(char *)(param_1[0x13] + 0x79b)) &&
                (*(char *)((int)local_fc + 0x79b) == *(char *)(param_1[0x13] + 0x79b))) &&
               (iVar3 = (**(code **)(*local_fc + 0x58))(), iVar3 == 0)) {
              local_fc = (int *)local_fc[3];
              goto LAB_004570e6;
            }
          }
          if ((short)local_fc[0x1a0] == 0) {
            if (DAT_005d03f8 == 0) {
              local_fc = (int *)local_fc[3];
              goto LAB_004570e6;
            }
            if ((('\0' < *(char *)(param_1[0x13] + 0x79b)) &&
                (*(char *)((int)local_fc + 0x79b) == *(char *)(param_1[0x13] + 0x79b))) &&
               (iVar3 = (**(code **)(*local_fc + 0x58))(), iVar3 == 0)) {
              local_fc = (int *)local_fc[3];
              goto LAB_004570e6;
            }
          }
        }
        if (((0 < DAT_005d03f8) && (iVar3 = (**(code **)(*local_fc + 0x58))(), iVar3 == 0)) &&
           ((uint)local_fc[8] < 1000)) {
          local_fc = (int *)local_fc[3];
          goto LAB_004570e6;
        }
        if (((0 < DAT_005d03f8) && (*(char *)(param_1[0x13] + 0x79b) == '\x03')) &&
           (((uint)local_fc[8] < 1000 ||
            (('\0' < *(char *)((int)local_fc + 0x79b) &&
             (*(char *)((int)local_fc + 0x79b) != '\x04')))))) {
          local_fc = (int *)local_fc[3];
          goto LAB_004570e6;
        }
      }
      if (((('\0' < *(char *)(param_1[0x13] + 0x79b)) && ('\0' < *(char *)((int)local_fc + 0x79b)))
          && (*(char *)(param_1[0x13] + 0x79b) == *(char *)((int)local_fc + 0x79b))) &&
         (DAT_005d0404 == 0)) {
        if (DAT_005d03f8 != 0) {
          local_fc = (int *)local_fc[3];
          goto LAB_004570e6;
        }
        if (999 < (uint)local_fc[8]) {
          local_fc = (int *)local_fc[3];
          goto LAB_004570e6;
        }
      }
    }
    if ((((uint)local_fc[8] < 1000) || ((char)local_fc[0x92] != '\0')) ||
       ((*(char *)((int)local_fc + 0x249) != '\0' || (iVar3 = FUN_00516105(), iVar3 == 0)))) {
      if (*(char *)((int)local_fc + 0x245) == '\x01') {
        local_fc = (int *)local_fc[3];
      }
      else {
        iVar3 = (**(code **)(*local_fc + 0x54))();
        if (((iVar3 == 1) && (iVar3 = (**(code **)(*(int *)param_1[0x13] + 0x54))(), iVar3 == 0)) &&
           (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 0)) {
          local_fc = (int *)local_fc[3];
        }
        else {
          if (((((local_130 <= local_12c) && (local_114 == local_110)) && (local_11c == local_118))
              && ((local_fc != (int *)param_1[0x13] &&
                  ((iVar3 = (**(code **)(*local_fc + 0x50))(), iVar3 == 0 ||
                   (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 0)))))) &&
             (((999 < (uint)local_fc[8] &&
               (((char)local_fc[0x92] == '\0' && (*(char *)((int)local_fc + 0x249) == '\0')))) ||
              ((((uint)local_fc[8] < 1000 && (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 0)) ||
               (iVar3 = (**(code **)(*local_fc + 0x54))(), iVar3 != 0)))))) {
            if ((*(int *)(&DAT_0092ff08 + local_18 * 0x60) == -1) && (local_f8 == 0)) {
                    /* WARNING: Ignoring partial resolution of indirect */
              local_68._0_2_ = (short)local_fc[8];
              local_86 = DAT_005d0400;
              local_f8 = 1;
            }
            *(short *)(&local_68 + local_f8) = (short)local_fc[8];
            *(undefined2 *)((int)&local_68 + local_f8 * 4 + 2) = 0xffff;
            if ((((local_24 != (int *)0x0) && (local_24 == local_fc)) && (local_f8 != 0)) &&
               (((*(int *)(&DAT_0092fefc + local_18 * 0x60) == 3 ||
                 (*(int *)(&DAT_0092fefc + local_18 * 0x60) == 4)) ||
                (*(int *)(&DAT_0092fefc + local_18 * 0x60) == 6)))) {
              uVar1 = (undefined2)local_68;
              local_154 = (undefined2)local_68;
              sVar2 = local_68._2_2_;
              local_158 = (int)local_68._2_2_;
              local_68 = CONCAT22(local_68._2_2_,*(undefined2 *)(&local_68 + local_f8));
              local_68 = CONCAT22(*(undefined2 *)((int)&local_68 + local_f8 * 4 + 2),
                                  *(undefined2 *)(&local_68 + local_f8));
              *(undefined2 *)(&local_68 + local_f8) = uVar1;
              *(short *)((int)&local_68 + local_f8 * 4 + 2) = sVar2;
            }
            local_f8 = local_f8 + 1;
          }
          local_fc = (int *)local_fc[3];
          if (((*(int *)(&DAT_0092fefc + local_18 * 0x60) == 3) && (7 < local_f8)) ||
             ((*(int *)(&DAT_0092ff44 + local_18 * 0x60) <= local_f8 || (0xc < local_f8))))
          goto LAB_00457fdd;
        }
      }
    }
    else {
      local_fc = (int *)local_fc[3];
    }
    goto LAB_004570e6;
  }
LAB_00457ff3:
  if (((((local_68 & 0xffff) != 0) || (local_18 == 'a')) || (local_18 == '#')) || (local_18 == '3'))
  {
    local_160 = *(int *)(&DAT_0092ff44 + local_18 * 0x60);
    for (local_1c4 = local_160; local_1c4 < 0xd; local_1c4 = local_1c4 + 1) {
      *(undefined2 *)(&local_68 + local_1c4) = 0;
      *(undefined2 *)((int)&local_68 + local_1c4 * 4 + 2) = 0;
    }
    local_15c = 0x60;
    if (*(int *)(&DAT_0092ff44 + local_18 * 0x60) == 1) {
      local_90 = CONCAT22(local_90._2_2_,0x39d);
      local_15c = 0x30;
    }
    if (*(int *)(&DAT_0092ff44 + local_18 * 0x60) == 2) {
      local_90 = CONCAT22(local_90._2_2_,0x39e);
      local_15c = 0x34;
    }
    FUN_0055f2dd(local_94,local_15c);
    FUN_0044dc1f();
    _memset(local_1c0,0,0x60);
    FUN_0058f220(local_1c0,local_94,local_15c);
    local_1ba = (undefined2)param_1[8];
    local_1a2 = 1;
    if (local_20 != 0) {
      local_1a1 = local_1a1 | 8;
    }
    (**(code **)(*param_1 + 4))(local_90 & 0xffff,local_1c0);
    param_1[0x9d18] = uVar4;
    *(uint *)(&DAT_0092eaf8 + local_18 * 4) = uVar4;
    ExceptionList = local_10;
    return 1;
  }
  if (local_18 != 'U') {
    ExceptionList = local_10;
    return 0;
  }
LAB_0045819d:
  if ((local_24 == (int *)0x0) || ((local_24[0x8e] != 1 && (param_9 == (int *)0x0)))) {
    param_1[0x1f] = 0;
  }
  else {
    if (*(char *)(param_1[0x13] + 0x795) == '\x01') {
      ExceptionList = local_10;
      return 0;
    }
    if ((uint)*(ushort *)((int)param_1 + 0x275ae) == *(uint *)(param_1[0x13] + 0x20)) {
      ExceptionList = local_10;
      return 0;
    }
    local_1dc = (uint)*(short *)(param_1[0x13] + 0x444);
    local_1d8 = (int)local_18 + (uint)*(byte *)(DAT_013b71e8 + 0x1c0) * -0x18;
    if (local_18 < '`') {
      local_1dc = (uint)*(byte *)((int)DAT_013b71e8 +
                                 ((int)(local_1d8 + (local_1d8 >> 0x1f & 7U)) >> 3) + 0x745);
    }
    iVar3 = FUN_00555790((int)local_18,*(undefined1 *)((int)DAT_013b71e8 + 0x9d3),local_1dc);
    if (*(short *)((int)DAT_013b71e8 + 0x73a) < iVar3) {
      local_1f0 = param_1[0x9eba];
      pvVar5 = operator_new(0xe50);
      local_8 = 2;
      if (pvVar5 == (void *)0x0) {
        local_32c = 0;
      }
      else {
        local_32c = FUN_00407203(&DAT_00a35078,0xffffaaaa,0,0,0x43960000,0x41800000,0,0x77777777,1,0
                                );
      }
      local_8 = 0xffffffff;
      local_1f4 = local_32c;
      if (local_32c != 0) {
        FUN_00408d33(local_32c);
      }
      if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x21), iVar3 != 0)) {
        uVar11 = 0;
        uVar10 = 0;
        FUN_00429a6d(0x21);
        FUN_0042ad2b(uVar10,uVar11);
      }
      ExceptionList = local_10;
      return 0;
    }
    if (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1) {
      iVar3 = (**(code **)(*(int *)param_1[0x13] + 0x50))();
      if ((iVar3 == 1) || (iVar3 = (**(code **)(*local_24 + 0x50))(), iVar3 == 1)) {
        ExceptionList = local_10;
        return 0;
      }
      if (((uint)local_24[8] < 1000) || ((char)local_24[0x92] == '\x01')) {
        iVar3 = (**(code **)(*local_24 + 0x54))();
        if (iVar3 == 0) {
          ExceptionList = local_10;
          return 0;
        }
        iVar3 = (**(code **)(*(int *)param_1[0x13] + 0x54))();
        if (iVar3 == 0) {
          ExceptionList = local_10;
          return 0;
        }
      }
    }
    iVar3 = (**(code **)(*local_24 + 0x54))();
    if (((iVar3 == 1) && (iVar3 = (**(code **)(*(int *)param_1[0x13] + 0x54))(), iVar3 == 0)) &&
       (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 0)) {
      ExceptionList = local_10;
      return 0;
    }
    iVar3 = (**(code **)(*local_24 + 0x54))();
    if (((iVar3 == 0) && (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)) &&
       ((*(char *)((int)local_24 + 0x249) == '\x01' || ((char)local_24[0x92] == '\x01')))) {
      ExceptionList = local_10;
      return 0;
    }
    if (((DAT_005d0404 == 0) && (*(char *)((int)local_24 + 0x249) == '\x01')) &&
       (((short)DAT_013b71e8[0x3c4] != (short)local_24[0x1a0] && ((short)local_24[0x1a0] != 0)))) {
      if (DAT_005d03f8 == 0) {
        ExceptionList = local_10;
        return 0;
      }
      if ((('\0' < *(char *)(param_1[0x13] + 0x79b)) &&
          (*(char *)(param_1[0x13] + 0x79b) == *(char *)((int)local_24 + 0x79b))) &&
         (iVar3 = (**(code **)(*local_24 + 0x58))(), iVar3 == 0)) {
        ExceptionList = local_10;
        return 0;
      }
    }
    if ((((DAT_005d0404 == 0) && ((short)local_24[0x1a0] != 0)) &&
        (((short)DAT_013b71e8[0x3c4] != (short)local_24[0x1a0] &&
         ((*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1 && ((uint)local_24[8] < 1000)))))) &&
       (iVar3 = (**(code **)(*local_24 + 0x54))(), iVar3 == 1)) {
      if (DAT_005d03f8 == 0) {
        ExceptionList = local_10;
        return 0;
      }
      if (('\0' < *(char *)(param_1[0x13] + 0x79b)) &&
         (*(char *)(param_1[0x13] + 0x79b) == *(char *)((int)local_24 + 0x79b))) {
        ExceptionList = local_10;
        return 0;
      }
    }
    if (((((DAT_005d0404 == 0) && (0 < DAT_005d03f8)) &&
         (*(char *)(param_1[0x13] + 0x79b) == '\x03')) &&
        (((uint)local_24[8] < 1000 ||
         (('\0' < *(char *)((int)local_24 + 0x79b) && (*(char *)((int)local_24 + 0x79b) != '\x04')))
         ))) && (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)) {
      ExceptionList = local_10;
      return 0;
    }
    if (((*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 0) && (999 < (uint)local_24[8])) &&
       (((char)local_24[0x92] == '\0' && (*(char *)((int)local_24 + 0x249) == '\0')))) {
      ExceptionList = local_10;
      return 0;
    }
    if ((((local_24 != (int *)0x0) && ((short)local_24[0x1a0] != 0)) &&
        (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)) &&
       ((*(short *)(param_1[0x13] + 0x680) == (short)local_24[0x1a0] ||
        (*(short *)((int)DAT_013b71e8 + 0xf12) == (short)local_24[0x1a0])))) {
      ExceptionList = local_10;
      return 0;
    }
    if (((local_24 != (int *)0x0) && (DAT_005d0404 == 0)) &&
       ((*(char *)((int)local_24 + 0x249) == '\x01' && ((short)local_24[0x1a0] == 0)))) {
      if (DAT_005d03f8 == 0) {
        ExceptionList = local_10;
        return 0;
      }
      if (('\0' < *(char *)(param_1[0x13] + 0x79b)) &&
         (*(char *)(param_1[0x13] + 0x79b) == *(char *)((int)local_24 + 0x79b))) {
        ExceptionList = local_10;
        return 0;
      }
    }
    if ((((999 < (uint)local_24[8]) && ((char)local_24[0x92] == '\0')) &&
        (*(char *)((int)local_24 + 0x249) == '\0')) && (iVar3 = FUN_00516105(), iVar3 != 0)) {
      ExceptionList = local_10;
      return 0;
    }
    if ((char)local_24[0x1c9] != '\0') {
      ExceptionList = local_10;
      return 0;
    }
    if (((*(byte *)((int)local_24 + 0x44a) & 0xf) == 0xf) &&
       (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)) {
      if (*(char *)(param_1[0x13] + 0x79b) == '\0') {
        ExceptionList = local_10;
        return 0;
      }
      if ((('\0' < *(char *)((int)local_24 + 0x79b)) && ('\0' < *(char *)(param_1[0x13] + 0x79b)))
         && (*(char *)((int)local_24 + 0x79b) == *(char *)(param_1[0x13] + 0x79b))) {
        ExceptionList = local_10;
        return 0;
      }
    }
    local_1d0 = __ftol();
    local_1d4 = __ftol();
    local_1e0 = __ftol();
    local_1ec = __ftol();
    if (param_1[0xa1e5] != 0) {
      local_1e0 = param_1[0xa1e5];
      local_1ec = param_1[0xa1e6];
    }
    local_1e8 = FUN_005563a0(local_1e0,local_1ec,local_1d0,local_1d4);
    local_1e4 = *(int *)(&DAT_0092ff08 + local_18 * 0x60) + local_20;
    local_1c8 = local_1d0;
    local_1cc = local_1d4;
    FUN_00556416(local_1e0,local_1ec,&local_1c8,&local_1cc,param_1 + 0x25,8);
    if (local_1e4 == -1) {
      local_1e4 = 1;
    }
    if (((local_24 != (int *)0x0) && (local_24[0x93] == 0x38)) &&
       (*(short *)((int)local_24 + 0x1d2) == 0)) {
      local_1e8 = local_1e8 + -0xc;
      if (local_1e8 < 0) {
        local_1e8 = 0;
      }
      local_1c8 = __ftol();
      local_1cc = __ftol();
    }
    iVar3 = FUN_0054cd07(local_2c + 0x23,0x15);
    if (iVar3 < 100) {
      local_1f8 = FUN_0054eb20(local_2c,0x1b);
      local_1f8 = local_1f8 + local_20;
      if (local_1e4 < local_1f8) {
        local_1e4 = local_1f8;
      }
    }
    if (((local_1e8 <= local_1e4) && (local_1c8 == local_1d0)) && (local_1cc == local_1d4)) {
      if ((*(int *)(&DAT_0092ff08 + local_18 * 0x60) == -1) &&
         (iVar3 = FUN_0054eb20(DAT_013b71e8 + 0x1bb,0x1b), iVar3 + local_20 < local_1e8)) {
        ExceptionList = local_10;
        return 0;
      }
      if ((DAT_005d0404 == 0) &&
         ((((*(char *)(param_1[0x13] + 0x79b) == '\0' && ((uint)local_24[8] < 1000)) &&
           (iVar3 = (**(code **)(*local_24 + 0x54))(), iVar3 == 1)) &&
          (((short)DAT_013b71e8[0x3c4] != (short)local_24[0x1a0] &&
           (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)))))) {
        ExceptionList = local_10;
        return 0;
      }
      if ((DAT_005d0404 == 0) &&
         (((0 < DAT_005d03f8 && ('\0' < *(char *)(param_1[0x13] + 0x79b))) &&
          (('\0' < *(char *)((int)local_24 + 0x79b) &&
           ((*(char *)(param_1[0x13] + 0x79b) == *(char *)((int)local_24 + 0x79b) &&
            (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)))))))) {
        ExceptionList = local_10;
        return 0;
      }
      if ((DAT_005d0404 == 0) &&
         ((((short)DAT_013b71e8[0x3c4] != (short)local_24[0x1a0] && ((uint)local_24[8] < 1000)) &&
          (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)))) {
        if (DAT_005d03f8 == 0) {
          ExceptionList = local_10;
          return 0;
        }
        if (*(char *)(param_1[0x13] + 0x79b) == *(char *)((int)local_24 + 0x79b)) {
          ExceptionList = local_10;
          return 0;
        }
      }
      if (((DAT_005d0404 == 0) && (0 < DAT_005d03f8)) &&
         (((iVar3 = (**(code **)(*local_24 + 0x58))(), iVar3 == 0 &&
           (((uint)local_24[8] < 1000 || (*(char *)((int)local_24 + 0x249) == '\x01')))) &&
          (*(int *)(&DAT_0092ff40 + local_18 * 0x60) == 1)))) {
        ExceptionList = local_10;
        return 0;
      }
      _memset(local_2c4,0,0x60);
      local_2c0 = CONCAT22(*(undefined2 *)(param_1[0x13] + 0x20),0x36c);
      local_2b8 = *(undefined2 *)(param_1[0x13] + 0x20);
      local_2b4 = (undefined2)param_1[0xa1e5];
      local_2b2 = (undefined2)param_1[0xa1e6];
      local_2b0 = __ftol();
      local_2ae = __ftol();
      local_2aa = 0xffff;
      local_2ac = (short)local_18;
      local_2a7 = 0;
      local_2a8 = 0xff;
      local_2a6 = 0;
      local_298[0] = (short)local_24[8];
      local_204 = (int *)(**(code **)(*DAT_013b71e8 + 0x34))((short)local_24[8]);
      if (local_204 == (int *)0x0) {
        ExceptionList = local_10;
        return 0;
      }
      if (*(int *)(&DAT_0092ff08 + local_18 * 0x60) == -1) {
                    /* WARNING: Ignoring partial resolution of indirect */
        local_298[0] = (short)local_204[8];
        local_2b6 = DAT_005d0400;
      }
      local_200 = (uint)(*(int *)(&DAT_0092ff08 + local_18 * 0x60) == -1);
      local_298[local_200 * 2] = (short)local_204[8];
      local_298[local_200 * 2 + 1] = 0xffff;
      local_200 = local_200 + 1;
      if (((local_18 == '\x10') || (local_18 == '\f')) || (local_18 == '\x1c')) {
        local_2d0 = __ftol();
        iVar3 = __ftol();
        local_2d0 = local_2d0 - iVar3;
        local_2d4 = __ftol();
        iVar3 = __ftol();
        local_2d4 = local_2d4 - iVar3;
        if (local_2d0 < 1) {
          if (local_2d0 < 0) {
            local_2d0 = -1;
          }
        }
        else {
          local_2d0 = 1;
        }
        if (local_2d4 < 1) {
          if (local_2d4 < 0) {
            local_2d4 = -1;
          }
        }
        else {
          local_2d4 = 1;
        }
        iVar3 = __ftol();
        iVar7 = __ftol();
        local_2d8 = *(int **)(param_1[0xd] + 0x10);
        while (local_2d8[3] != 0) {
          if (((local_2d8 == (int *)param_1[0x13]) || (local_2d8 == local_204)) ||
             ((iVar8 = __ftol(), iVar8 != iVar3 + local_2d0 ||
              (iVar8 = __ftol(), iVar8 != iVar7 + local_2d4)))) {
            local_2d8 = (int *)local_2d8[3];
          }
          else if ((uint)local_2d8[8] < 1000) {
            iVar8 = (**(code **)(*local_2d8 + 0x54))();
            if (((iVar8 == 0) || (DAT_005d0404 == 0)) || ((char)local_2d8[0x92] == '\x01')) {
              local_2d8 = (int *)local_2d8[3];
            }
            else if (((DAT_005d0404 == 0) && ((short)DAT_013b71e8[0x3c4] != (short)local_2d8[0x1a0])
                     ) && (DAT_005d03f8 == 0)) {
              local_2d8 = (int *)local_2d8[3];
            }
            else if (((DAT_005d0404 == 0) && ('\0' < *(char *)(param_1[0x13] + 0x79b))) &&
                    ((*(char *)(param_1[0x13] + 0x79b) == *(char *)((int)local_2d8 + 0x79b) &&
                     (0 < DAT_005d03f8)))) {
              local_2d8 = (int *)local_2d8[3];
            }
            else if ((((DAT_005d0404 == 0) && (0 < DAT_005d03f8)) &&
                     (*(char *)(param_1[0x13] + 0x79b) == '\x03')) &&
                    (((uint)local_2d8[8] < 1000 ||
                     (('\0' < *(char *)((int)local_2d8 + 0x79b) &&
                      (*(char *)((int)local_2d8 + 0x79b) != '\x04')))))) {
              local_2d8 = (int *)local_2d8[3];
            }
            else {
              if (((DAT_005d0404 != 0) || (DAT_005d03f8 < 1)) ||
                 (iVar8 = (**(code **)(*local_2d8 + 0x58))(), iVar8 != 0)) {
LAB_004593e1:
                local_298[local_200 * 2] = (short)local_2d8[8];
                local_298[local_200 * 2 + 1] = 0xffff;
                local_200 = local_200 + 1;
                break;
              }
              local_2d8 = (int *)local_2d8[3];
            }
          }
          else if (((DAT_005d0404 == 0) && (*(char *)((int)local_2d8 + 0x249) == '\x01')) &&
                  ((((short)DAT_013b71e8[0x3c4] != (short)local_2d8[0x1a0] &&
                    ((short)local_2d8[0x1a0] != 0)) || ((short)local_2d8[0x1a0] == 0)))) {
            iVar8 = (**(code **)(*local_2d8 + 0x58))();
            if (((iVar8 == 0) && (0 < DAT_005d03f8)) || (DAT_005d03f8 == 0)) {
              local_2d8 = (int *)local_2d8[3];
            }
          }
          else {
            if (((local_24 == (int *)0x0) || (DAT_005d0404 != 0)) ||
               ((*(char *)((int)local_2d8 + 0x249) != '\x01' || ((short)local_2d8[0x1a0] != 0))))
            goto LAB_004593e1;
            iVar8 = (**(code **)(*local_2d8 + 0x58))();
            if (((iVar8 == 0) && (0 < DAT_005d03f8)) || (DAT_005d03f8 == 0)) {
              local_2d8 = (int *)local_2d8[3];
            }
          }
        }
      }
      local_2b0 = __ftol();
      local_2ae = __ftol();
      if (local_18 != '\0') {
        local_2b4 = (undefined2)param_1[0xa1e5];
        local_2b2 = (undefined2)param_1[0xa1e6];
        local_2b0 = (undefined2)param_1[0xa1e5];
        local_2ae = (undefined2)param_1[0xa1e6];
      }
      local_1fc = 0x60;
      if (*(int *)(&DAT_0092ff44 + local_18 * 0x60) == 1) {
        local_2c0 = CONCAT22(local_2c0._2_2_,0x39d);
        local_1fc = 0x30;
      }
      if (*(int *)(&DAT_0092ff44 + local_18 * 0x60) == 2) {
        local_2c0 = CONCAT22(local_2c0._2_2_,0x39e);
        local_1fc = 0x34;
      }
      FUN_0055f2dd(local_2c4,local_1fc);
      FUN_0044dc1f();
      FUN_0058f220(local_264,local_2c4,0x60);
      local_25e = (undefined2)param_1[8];
      local_246 = 1;
      if (local_20 != 0) {
        local_245 = local_245 | 8;
      }
      (**(code **)(*param_1 + 4))(local_2c0 & 0xffff,local_264);
      param_1[0x9d18] = uVar4;
      *(uint *)(&DAT_0092eaf8 + local_18 * 4) = uVar4;
      *(undefined4 *)(param_1[0x13] + 0x76c) = 0;
      ExceptionList = local_10;
      return 1;
    }
  }
  FUN_00455ea7(uVar4);
  ExceptionList = local_10;
  return 0;
LAB_00457fdd:
  local_80 = (undefined2)local_108;
  local_7e = (undefined2)local_10c;
  goto LAB_00457ff3;
}

