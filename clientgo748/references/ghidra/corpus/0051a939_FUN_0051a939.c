// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0051a939 | Name: FUN_0051a939


/* WARNING: Removing unreachable block (ram,0x0051b99f) */

void __thiscall FUN_0051a939(int *param_1,int *param_2)

{
  bool bVar1;
  ushort uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int local_138;
  int local_134;
  int local_130;
  int local_12c;
  int local_120;
  int *local_114;
  int local_110;
  int local_10c;
  undefined1 local_108 [4];
  undefined2 local_104;
  undefined2 local_102;
  undefined2 local_fc;
  undefined2 local_fa;
  undefined2 local_f8;
  undefined2 local_f6;
  undefined2 local_f4;
  undefined2 local_f2;
  undefined2 local_f0;
  undefined2 local_ee;
  char local_ec;
  undefined1 local_eb;
  undefined1 local_e9 [13];
  undefined2 local_dc;
  undefined2 uStack_da;
  undefined2 local_d8;
  short local_d6;
  uint local_a8;
  undefined1 local_a4 [6];
  undefined2 local_9e;
  undefined1 local_86;
  byte local_85;
  int *local_44;
  undefined4 local_40;
  int local_3c;
  int local_38;
  int local_34;
  int local_30;
  int *local_2c;
  undefined4 local_28;
  int local_24;
  int local_20;
  int local_1c;
  int *local_18;
  int *local_14;
  int local_10;
  undefined4 local_c;
  uint local_8;
  
  if (param_1[0x85] != 0) {
    return;
  }
  if (param_2 == (int *)0x0) {
    return;
  }
  if ((999 < (uint)param_2[8]) && (iVar3 = FUN_00516105(), iVar3 != 0)) {
    return;
  }
  if (((((*(byte *)((int)param_1 + 0x44a) & 0xf) == 0xf) &&
       (local_1c = DAT_0067cf38[0x13], '\0' < *(char *)((int)param_1 + 0x79b))) &&
      ('\0' < *(char *)(local_1c + 0x79b))) &&
     (*(char *)((int)param_1 + 0x79b) == *(char *)(local_1c + 0x79b))) {
    return;
  }
  iVar3 = (**(code **)(*param_1 + 0x50))();
  if (iVar3 == 1) {
    return;
  }
  iVar3 = (**(code **)(*param_2 + 0x50))();
  if (iVar3 == 1) {
    return;
  }
  if (DAT_0067cf38[9] != 30000) {
    return;
  }
  local_14 = DAT_0067cf38;
  if (DAT_0067cf38[0x9c51] != 0) {
    return;
  }
  if (DAT_0067cf38[0x9c50] != 0) {
    return;
  }
  if (DAT_0067cf38[0x9c4e] != 0) {
    return;
  }
  if (DAT_013b7220 != 0) {
    return;
  }
  if (((DAT_005d0404 == 0) && (*(char *)((int)param_2 + 0x249) == '\x01')) &&
     (((short)DAT_013b71e8[0x3c4] != (short)param_2[0x1a0] && ((short)param_2[0x1a0] != 0)))) {
    if (DAT_005d03f8 == 0) {
      iVar3 = __ftol();
      if (((iVar3 >> 7 < 0x11) || (iVar3 = __ftol(), 0x13 < iVar3 >> 7)) ||
         (iVar3 = __ftol(), iVar3 >> 7 < 0x1e)) {
        bVar1 = false;
      }
      else {
        bVar1 = true;
      }
      if (bVar1) goto LAB_0051ab62;
      bVar1 = true;
    }
    else {
LAB_0051ab62:
      bVar1 = false;
    }
    if (bVar1) {
      return;
    }
    if (('\0' < *(char *)((int)param_1 + 0x79b)) &&
       (*(char *)((int)param_2 + 0x79b) == *(char *)((int)param_1 + 0x79b))) {
      return;
    }
  }
  if (DAT_005d0404 != 0) goto LAB_0051ad24;
  if (DAT_005d03f8 == 0) {
    iVar3 = __ftol();
    if (((iVar3 >> 7 < 0x11) || (iVar3 = __ftol(), 0x13 < iVar3 >> 7)) ||
       (iVar3 = __ftol(), iVar3 >> 7 < 0x1e)) {
      bVar1 = false;
    }
    else {
      bVar1 = true;
    }
    if (bVar1) goto LAB_0051ac2e;
    bVar1 = true;
  }
  else {
LAB_0051ac2e:
    bVar1 = false;
  }
  if (bVar1) {
    if (((short)param_2[0x1a0] != 0) && ((short)DAT_013b71e8[0x3c4] != (short)param_2[0x1a0])) {
      return;
    }
    if (((((char)local_14[0x9bad] == '\x01') && ((short)DAT_013b71e8[0x3c4] != 0)) &&
        ((short)DAT_013b71e8[0x3c4] != (short)param_2[0x1a0])) && ((uint)param_2[8] < 1000)) {
      return;
    }
  }
  else if (('\0' < *(char *)((int)param_1 + 0x79b)) &&
          (*(char *)((int)param_2 + 0x79b) == *(char *)((int)param_1 + 0x79b))) {
    return;
  }
LAB_0051ad24:
  local_8 = (**(code **)(*DAT_0092e654 + 8))();
  local_14[0x9ba4] = (int)param_2;
  local_18 = DAT_013b71e8 + 0x1bb;
  local_10 = 0;
  if ((local_18 != (int *)0x0) && ((DAT_013b71e8[0x272] & 0x20000000U) != 0)) {
    local_10 = 1;
  }
  local_c = 0;
  if (local_8 <= local_14[0x9d18] + 1000U) {
    return;
  }
  local_30 = __ftol();
  local_3c = __ftol();
  if (local_14[0xa1e5] != 0) {
    local_30 = local_14[0xa1e5];
    local_3c = local_14[0xa1e6];
  }
  local_20 = __ftol();
  local_24 = __ftol();
  local_38 = FUN_005563a0(local_30,local_3c,local_20,local_24);
  local_34 = FUN_0054eb20(DAT_013b71e8 + 0x1bb,0x1b);
  local_34 = local_34 + local_10;
  FUN_00556416(local_30,local_3c,&local_20,&local_24,local_14 + 0x25,8);
  if ((param_2[0x93] == 0x38) && (*(short *)((int)param_2 + 0x1d2) == 0)) {
    local_38 = local_38 + -0xc;
    if (local_38 < 0) {
      local_38 = 0;
    }
    local_20 = __ftol();
    local_24 = __ftol();
  }
  local_28 = 3;
  local_2c = DAT_013b71e8 + 0x1bb;
  DAT_0092ec98 = (DAT_0092ec98 + 1) % 3;
  if (((local_34 < local_38) || (iVar3 = __ftol(), local_20 != iVar3)) ||
     (iVar3 = __ftol(), local_24 != iVar3)) {
    if ((char)local_14[0x9bad] != '\x01') {
      return;
    }
    if ((param_1[0x1d0] != 0) && (param_1[0x1d0] != 1)) {
      return;
    }
    if (DAT_005d03f4 != 0) {
      return;
    }
    uVar2 = GetKeyState(0x10);
    if ((int)(uint)uVar2 >> 8 != 0) {
      return;
    }
    local_134 = __ftol();
    local_138 = __ftol();
    local_20 = __ftol();
    local_24 = __ftol();
    local_120 = FUN_005563a0(local_134,local_138,local_20,local_24);
    local_12c = 1;
    local_130 = 1;
    if (local_20 < local_30) {
      local_12c = -1;
    }
    if (local_24 < local_3c) {
      local_130 = -1;
    }
    while (local_34 < local_120) {
      if (local_134 != local_20) {
        local_134 = local_134 + local_12c;
      }
      if (local_138 != local_24) {
        local_138 = local_138 + local_130;
      }
      local_120 = FUN_005563a0(local_134,local_138,local_20,local_24);
    }
    local_14[0x9d6c] = local_134;
    local_14[0x9d6d] = local_138;
    FUN_00520216(local_14[0x9d6c],local_14[0x9d6d],0,0);
    return;
  }
  _memset(local_108,0,0x60);
  local_104 = 0x39d;
  local_102 = (undefined2)param_1[8];
  local_fc = (undefined2)param_1[8];
  local_f8 = __ftol();
  local_f6 = __ftol();
  local_ee = 0xffff;
  local_f0 = 0xffff;
  local_eb = 0;
  local_ec = (char)DAT_0092ec98 + '\x04';
  iVar3 = FUN_0054cd07(local_2c + 0x23,0x15);
  if (iVar3 == 0x65) {
    local_f0 = 0x97;
  }
  else {
    iVar3 = FUN_0054cd07(local_2c + 0x23,0x15);
    if (iVar3 == 0x66) {
      local_f0 = 0x98;
      if (*(short *)(&DAT_00d44990 + (short)local_2c[0x23] * 0x8c) == 0x367) {
        local_eb = 0;
      }
      else if (*(short *)(&DAT_00d44990 + (short)local_2c[0x23] * 0x8c) == 0x368) {
        local_eb = 1;
      }
    }
    else {
      iVar3 = FUN_0054cd07(local_2c + 0x23,0x15);
      if (iVar3 == 0x67) {
        local_f0 = 0x99;
        if (*(short *)(&DAT_00d44990 + (short)local_2c[0x23] * 0x8c) == 0x369) {
          local_eb = 0;
        }
        else if (*(short *)(&DAT_00d44990 + (short)local_2c[0x23] * 0x8c) == 0x36a) {
          local_eb = 1;
        }
        else if (*(short *)(&DAT_00d44990 + (short)local_2c[0x23] * 0x8c) == 0x36b) {
          local_eb = 2;
        }
        else if (*(short *)(&DAT_00d44990 + (short)local_2c[0x23] * 0x8c) == 0x36c) {
          local_eb = 3;
        }
        else if (*(short *)(&DAT_00d44990 + (short)local_2c[0x23] * 0x8c) == 0x36d) {
          local_eb = 4;
        }
        else if (*(short *)(&DAT_00d44990 + (short)local_2c[0x23] * 0x8c) == 0x37c) {
          local_eb = 5;
        }
        else if (*(short *)(&DAT_00d44990 + (short)local_2c[0x23] * 0x8c) == 0x38b) {
          local_eb = 6;
        }
        else if (*(short *)(&DAT_00d44990 + (short)local_2c[0x23] * 0x8c) == 0x38c) {
          local_eb = 7;
        }
        else if (*(short *)(&DAT_00d44990 + (short)local_2c[0x23] * 0x8c) == 0x38d) {
          local_eb = 8;
        }
        else if (*(short *)(&DAT_00d44990 + (short)local_2c[0x23] * 0x8c) == 0x25) {
          local_eb = 9;
        }
        else if (*(short *)(&DAT_00d44990 + (short)local_2c[0x23] * 0x8c) == 0x2ff) {
          local_eb = 10;
        }
        else if (*(short *)(&DAT_00d44990 + (short)local_2c[0x23] * 0x8c) == 0xafe) {
          local_eb = 0xb;
        }
      }
      else {
        iVar3 = FUN_0054cd07(local_2c + 0x23,0x15);
        if (iVar3 == 0x68) {
          local_f0 = 0x68;
          if (*(short *)(&DAT_00d44990 + (short)local_2c[0x23] * 0x8c) == 0x36e) {
            local_eb = 0;
          }
          else if (*(short *)(&DAT_00d44990 + (short)local_2c[0x23] * 0x8c) == 0x36f) {
            local_eb = 1;
          }
        }
      }
    }
  }
  _local_dc = CONCAT22(uStack_da,(short)param_2[8]);
  local_44 = (int *)(**(code **)(*DAT_013b71e8 + 0x34))((short)param_2[8]);
  if (local_44 == (int *)0x0) {
    return;
  }
  local_a8 = (uint)*(byte *)((int)DAT_013b71e8 + 0x9d2);
  _local_dc = CONCAT22(0xfffe,local_dc);
  local_fa = DAT_005d0400;
  FUN_00555911(DAT_013b71e8 + 0x1bb,0,&DAT_005d0400,local_e9);
  local_f4 = __ftol();
  local_f2 = __ftol();
  local_40 = 0x60;
  if (((char)local_2c[5] == '\x03') && ((local_2c[0xb7] & 0x200000U) != 0)) {
    local_104 = 0x39e;
    local_40 = 0x34;
  }
  if (((char)local_2c[5] == '\x03') && ((local_2c[0xb7] & 0x40U) != 0)) {
    local_10c = __ftol();
    local_110 = __ftol();
    if (local_10c < 1) {
      if (local_10c < 0) {
        local_10c = -1;
      }
    }
    else {
      local_10c = 1;
    }
    if (local_110 < 1) {
      if (local_110 < 0) {
        local_110 = -1;
      }
    }
    else {
      local_110 = 1;
    }
    iVar3 = __ftol();
    iVar4 = __ftol();
    local_114 = *(int **)(local_14[0xd] + 0x10);
    while (local_114[3] != 0) {
      if ((((local_114 == (int *)local_14[0x13]) || (local_114 == local_44)) ||
          (iVar5 = __ftol(), iVar5 != iVar3 + local_10c)) ||
         (iVar5 = __ftol(), iVar5 != iVar4 + local_110)) {
        local_114 = (int *)local_114[3];
      }
      else if ((uint)local_114[8] < 1000) {
        iVar5 = (**(code **)(*local_114 + 0x54))();
        if ((iVar5 == 0) || ((char)local_114[0x92] == '\x01')) {
          local_114 = (int *)local_114[3];
        }
        else if (((DAT_005d0404 == 0) && ((short)DAT_013b71e8[0x3c4] != (short)local_114[0x1a0])) &&
                (DAT_005d03f8 == 0)) {
          local_114 = (int *)local_114[3];
        }
        else if (((DAT_005d0404 == 0) && ('\0' < *(char *)((int)param_1 + 0x79b))) &&
                ((*(char *)((int)param_1 + 0x79b) == *(char *)((int)local_114 + 0x79b) &&
                 (0 < DAT_005d03f8)))) {
          local_114 = (int *)local_114[3];
        }
        else if (((DAT_005d0404 == 0) && (0 < DAT_005d03f8)) &&
                ((*(char *)((int)param_1 + 0x79b) == '\x03' &&
                 (((uint)local_114[8] < 1000 ||
                  (('\0' < *(char *)((int)local_114 + 0x79b) &&
                   (*(char *)((int)local_114 + 0x79b) != '\x04')))))))) {
          local_114 = (int *)local_114[3];
        }
        else {
          if (((DAT_005d0404 != 0) || (DAT_005d03f8 < 1)) ||
             (iVar5 = (**(code **)(*local_114 + 0x58))(), iVar5 != 0)) {
LAB_0051b774:
            local_d8 = (undefined2)local_114[8];
            local_d6 = -2;
            break;
          }
          local_114 = (int *)local_114[3];
        }
      }
      else {
        if (((DAT_005d0404 != 0) || (*(char *)((int)local_114 + 0x249) != '\x01')) ||
           ((((short)DAT_013b71e8[0x3c4] == (short)local_114[0x1a0] ||
             ((short)local_114[0x1a0] == 0)) && ((short)local_114[0x1a0] != 0)))) goto LAB_0051b774;
        local_114 = (int *)local_114[3];
      }
    }
    if (((local_2c[0xb7] & 0x200000U) == 0x200000) || (local_d6 == -2)) {
      local_104 = 0x39e;
      local_40 = 0x34;
    }
    else {
      local_104 = 0x39d;
      local_40 = 0x30;
    }
  }
  FUN_0055f2dd(local_108,local_40);
  FUN_0058f220(local_a4,local_108,local_40);
  local_9e = (undefined2)local_14[8];
  local_86 = 1;
  if ((local_2c[0xb7] & 0x20000000U) != 0) {
    local_85 = local_85 | 4;
  }
  (**(code **)(*local_14 + 4))(0x39d,local_a4);
  local_14[0x9d18] = local_8;
  return;
}

