// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0046087b | Name: FUN_0046087b


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_0046087b(int *param_1,uint param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  uint in_stack_00000014;
  uint local_140;
  int local_134;
  int local_130;
  int local_12c;
  int local_128;
  int local_124;
  int local_120;
  uint local_11c;
  int *local_110;
  int local_10c;
  int local_108;
  undefined1 local_104 [4];
  undefined4 local_100;
  undefined2 local_f8;
  undefined2 local_f6;
  ushort local_f4;
  ushort uStack_f2;
  undefined2 local_f0;
  undefined2 local_ee;
  short local_ec;
  undefined2 local_ea;
  char local_e8;
  undefined1 local_e7;
  undefined1 local_e5 [13];
  undefined2 local_d8;
  undefined2 uStack_d6;
  undefined2 local_d4;
  short local_d2;
  uint local_a4;
  undefined1 local_a0 [6];
  undefined2 local_9a;
  undefined2 local_92;
  undefined1 local_82;
  byte local_81;
  undefined4 local_40;
  undefined4 local_3c;
  int local_38;
  int local_34;
  int local_30;
  int local_2c;
  undefined4 local_28;
  int local_24;
  int local_20 [3];
  int local_14;
  int *local_10;
  int *local_c;
  int local_8;
  
  local_c = (int *)param_1[0x1f];
  local_8 = 0;
  local_10 = DAT_013b71e8 + 0x1bb;
  if ((local_10 != (int *)0x0) && ((DAT_013b71e8[0x272] & 0x20000000U) != 0)) {
    local_8 = 1;
  }
  if ((uint)*(ushort *)((int)param_1 + 0x275ae) == *(uint *)(param_1[0x13] + 0x20)) {
    return 0;
  }
  if (local_c != (int *)0x0) {
    if ((char)local_c[0x92] == '\x01') {
      return 0;
    }
    if (((short)local_c[0x1a0] != 0) &&
       ((*(short *)(param_1[0x13] + 0x680) == (short)local_c[0x1a0] ||
        (*(short *)((int)DAT_013b71e8 + 0xf12) == (short)local_c[0x1a0])))) {
      return 0;
    }
    if (((uint)local_c[8] < 1000) &&
       ((iVar1 = (**(code **)(*local_c + 0x54))(), iVar1 == 0 ||
        (iVar1 = (**(code **)(*(int *)param_1[0x13] + 0x54))(), iVar1 == 0)))) {
      return 0;
    }
    if ((DAT_005d0404 == 0) && (*(char *)((int)local_c + 0x249) == '\x01')) {
      local_14 = 0;
      if (('\0' < *(char *)(param_1[0x13] + 0x79b)) &&
         (*(char *)(param_1[0x13] + 0x79b) == *(char *)((int)local_c + 0x79b))) {
        local_14 = 1;
      }
      if ((((short)local_c[0x1a0] != 0) && ((short)DAT_013b71e8[0x3c4] != (short)local_c[0x1a0])) &&
         (DAT_005d03f8 == 0)) {
        if ((short)DAT_013b71e8[0x3c4] != (short)local_c[0x1a0]) {
          return 0;
        }
        if ((*(char *)(param_1[0x13] + 0x79b) == *(char *)((int)local_c + 0x79b)) &&
           (iVar1 = (**(code **)(*local_c + 0x58))(), iVar1 == 0)) {
          return 0;
        }
      }
      if ((short)local_c[0x1a0] == 0) {
        if (DAT_005d03f8 == 0) {
          return 1;
        }
        if (local_14 != 0) {
          return 0;
        }
      }
    }
  }
  if ((local_c == (int *)0x0) || (local_c[0x8e] != 1)) {
    param_1[0x1f] = 0;
    iVar1 = FUN_00455ea7(in_stack_00000014);
    if (iVar1 != 1) {
      return 0;
    }
    return 1;
  }
  local_140 = (uint)((uint)local_c[8] < 1000);
  local_20[2] = local_140;
  if (local_140 == 0) {
    iVar1 = FUN_00516105();
    if (iVar1 != 0) {
      return 0;
    }
    if ((*(byte *)((int)local_c + 0x44a) & 0xf) == 0xf) {
      if ((*(char *)(param_1[0x13] + 0x79b) == '\0') || (*(char *)(param_1[0x13] + 0x79b) == '\x03')
         ) {
        return 0;
      }
      if ((('\0' < *(char *)((int)local_c + 0x79b)) && ('\0' < *(char *)(param_1[0x13] + 0x79b))) &&
         (*(char *)((int)local_c + 0x79b) == *(char *)(param_1[0x13] + 0x79b))) {
        return 0;
      }
    }
  }
  if ((char)local_c[0x1c9] != '\0') {
    return 0;
  }
  iVar1 = (**(code **)(*(int *)param_1[0x13] + 0x50))();
  if ((iVar1 == 1) || (iVar1 = (**(code **)(*local_c + 0x50))(), iVar1 == 1)) {
    return 0;
  }
  if ((((local_20[2] == 0) && ((char)local_c[0x92] == '\0')) &&
      (*(char *)((int)local_c + 0x249) == '\0')) ||
     (iVar1 = (**(code **)(*local_c + 0x54))(), iVar1 != 0)) {
    if ((((*(char *)(param_1[0x13] + 0x79b) == '\0') && ((uint)local_c[8] < 1000)) &&
        (iVar1 = (**(code **)(*local_c + 0x54))(), iVar1 == 1)) &&
       ((DAT_005d0404 == 0 && ((short)DAT_013b71e8[0x3c4] != (short)local_c[0x1a0])))) {
      return 0;
    }
    if ((DAT_005d0404 == 0) && ((uint)local_c[8] < 1000)) {
      if (DAT_005d03f8 == 0) {
        if ((short)DAT_013b71e8[0x3c4] != (short)local_c[0x1a0]) {
          return 0;
        }
      }
      else if (*(char *)(param_1[0x13] + 0x79b) == *(char *)((int)local_c + 0x79b)) {
        return 0;
      }
    }
    if ((((DAT_005d0404 == 0) && (*(char *)((int)local_c + 0x249) == '\x01')) &&
        ((short)DAT_013b71e8[0x3c4] != (short)local_c[0x1a0])) &&
       (((short)local_c[0x1a0] != 0 && (iVar1 = (**(code **)(*local_c + 0x58))(), iVar1 == 0)))) {
      return 0;
    }
    if (((DAT_005d0404 == 0) && (0 < DAT_005d03f8)) &&
       (('\0' < *(char *)(param_1[0x13] + 0x79b) &&
        (*(char *)(param_1[0x13] + 0x79b) == *(char *)((int)local_c + 0x79b))))) {
      return 0;
    }
    if ((((DAT_005d0404 == 0) && (0 < DAT_005d03f8)) && (*(char *)(param_1[0x13] + 0x79b) == '\x03')
        ) && (((uint)local_c[8] < 1000 ||
              (('\0' < *(char *)((int)local_c + 0x79b) &&
               (*(char *)((int)local_c + 0x79b) != '\x04')))))) {
      return 0;
    }
    if (((DAT_005d0404 == 0) && (0 < DAT_005d03f8)) &&
       ((iVar1 = (**(code **)(*local_c + 0x58))(), iVar1 == 0 && ((uint)local_c[8] < 1000)))) {
      return 0;
    }
    local_20[1] = 0;
    if (param_1[0x9d18] + 1000U < in_stack_00000014) {
      local_2c = __ftol();
      local_38 = __ftol();
      if (param_1[0xa1e5] != 0) {
        local_2c = param_1[0xa1e5];
        local_38 = param_1[0xa1e6];
      }
      local_20[0] = __ftol();
      local_24 = __ftol();
      local_34 = FUN_005563a0(local_2c,local_38,local_20[0],local_24);
      local_30 = FUN_0054eb20(DAT_013b71e8 + 0x1bb,0x1b);
      local_30 = local_30 + local_8;
      FUN_00556416(local_2c,local_38,local_20,&local_24,param_1 + 0x25,8);
      if ((local_c[0x93] == 0x38) && (*(short *)((int)local_c + 0x1d2) == 0)) {
        local_34 = local_34 + -0xc;
        if (local_34 < 0) {
          local_34 = 0;
        }
        local_20[0] = __ftol();
        local_24 = __ftol();
      }
      local_28 = 3;
      DAT_005d0410 = (DAT_005d0410 + 1) % 3;
      if (local_c != (int *)0x0) {
        *(int *)(param_1[0x13] + 0x7d4) = local_c[8];
      }
      if (((local_30 < local_34) || (iVar1 = __ftol(), local_20[0] != iVar1)) ||
         (iVar1 = __ftol(), local_24 != iVar1)) {
        if ((local_30 <= local_34) && ((param_2 & 4) != 4)) {
          if ((3 < *(int *)(param_1[0x13] + 0x740)) && (*(int *)(param_1[0x13] + 0x740) < 10)) {
            local_11c = *(uint *)(&DAT_0065a4a8 +
                                 *(int *)(*(int *)(param_1[0x13] + 0x60) + 0x2d0) * 4 +
                                 *(int *)(param_1[0x13] + 0x5c) * 900);
            if (2 < local_11c) {
              local_11c = local_11c - 2;
            }
            if ((*(int *)(DAT_005d0504 + 0x1c) != 0) &&
               (in_stack_00000014 <
                *(int *)(param_1[0x13] + 0x600) +
                *(int *)(*(int *)(param_1[0x13] + 0x60) + 0x2e4) * local_11c * 4)) {
              return 1;
            }
          }
          if (param_1[0x10] != 0) {
            FUN_004fa8a1(param_1[0x9b9f]);
            iVar1 = __ftol();
            iVar2 = __ftol();
            if (iVar1 == iVar2) {
              iVar1 = __ftol();
              iVar2 = __ftol();
              if (iVar1 == iVar2) {
                return 1;
              }
            }
            param_1[0x9ba6] = 0;
            *(int **)(param_1[0x13] + 0x768) = local_c;
            param_1[0x9d35] = in_stack_00000014;
            local_120 = __ftol();
            local_124 = __ftol();
            if (param_1[0xa1e5] != 0) {
              local_120 = param_1[0xa1e5];
              local_124 = param_1[0xa1e6];
            }
            if (local_c != (int *)0x0) {
              local_20[0] = __ftol();
              local_24 = __ftol();
              local_128 = FUN_005563a0(local_120,local_124,local_20[0],local_24);
              local_130 = 1;
              local_134 = 1;
              if (local_20[0] < local_120) {
                local_130 = -1;
              }
              if (local_24 < local_124) {
                local_134 = -1;
              }
              local_12c = 0;
              while (local_30 < local_128) {
                if (500 < local_12c) {
                  return 1;
                }
                if (local_120 != local_20[0]) {
                  local_120 = local_120 + local_130;
                }
                if (local_124 != local_24) {
                  local_124 = local_124 + local_134;
                }
                local_128 = FUN_005563a0(local_120,local_124,local_20[0],local_24);
                local_12c = local_12c + 1;
              }
              param_1[0x9d6c] = local_120;
              param_1[0x9d6d] = local_124;
            }
          }
        }
      }
      else {
        _memset(local_104,0,0x60);
        local_100 = CONCAT22(*(undefined2 *)(param_1[0x13] + 0x20),0x39d);
        local_f8 = *(undefined2 *)(param_1[0x13] + 0x20);
        local_f4 = *(ushort *)(param_1 + 0xa1e5);
        uStack_f2 = *(ushort *)(param_1 + 0xa1e6);
        local_ea = 0xffff;
        local_ec = -1;
        local_e7 = 0;
        local_e8 = (char)DAT_005d0410 + '\x04';
        if (*(int *)(param_1[0x13] + 0x24c) == 0x21) {
          if (local_34 < 2) {
            local_e8 = '\x04';
          }
          else {
            local_e8 = '\x05';
            local_ec = 0x69;
            local_e7 = 2;
          }
        }
        iVar1 = FUN_0054cd07(local_10 + 0x23,0x15);
        if (iVar1 == 0x65) {
          local_ec = 0x97;
          local_e7 = 0;
        }
        else {
          iVar1 = FUN_0054cd07(local_10 + 0x23,0x15);
          if (iVar1 == 0x66) {
            local_ec = 0x98;
            if (*(short *)(&DAT_00d44990 + (short)local_10[0x23] * 0x8c) == 0x367) {
              local_e7 = 0;
            }
            else if (*(short *)(&DAT_00d44990 + (short)local_10[0x23] * 0x8c) == 0x368) {
              local_e7 = 1;
            }
          }
          else {
            iVar1 = FUN_0054cd07(local_10 + 0x23,0x15);
            if (iVar1 == 0x67) {
              local_ec = 0x99;
              if (*(short *)(&DAT_00d44990 + (short)local_10[0x23] * 0x8c) == 0x369) {
                local_e7 = 0;
              }
              else if (*(short *)(&DAT_00d44990 + (short)local_10[0x23] * 0x8c) == 0x36a) {
                local_e7 = 1;
              }
              else if (*(short *)(&DAT_00d44990 + (short)local_10[0x23] * 0x8c) == 0x36b) {
                local_e7 = 2;
              }
              else if (*(short *)(&DAT_00d44990 + (short)local_10[0x23] * 0x8c) == 0x36c) {
                local_e7 = 3;
              }
              else if (*(short *)(&DAT_00d44990 + (short)local_10[0x23] * 0x8c) == 0x36d) {
                local_e7 = 4;
              }
              else if (*(short *)(&DAT_00d44990 + (short)local_10[0x23] * 0x8c) == 0x37c) {
                local_e7 = 5;
              }
              else if (*(short *)(&DAT_00d44990 + (short)local_10[0x23] * 0x8c) == 0x38b) {
                local_e7 = 6;
              }
              else if (*(short *)(&DAT_00d44990 + (short)local_10[0x23] * 0x8c) == 0x38c) {
                local_e7 = 7;
              }
              else if (*(short *)(&DAT_00d44990 + (short)local_10[0x23] * 0x8c) == 0x38d) {
                local_e7 = 8;
              }
              else if (*(short *)(&DAT_00d44990 + (short)local_10[0x23] * 0x8c) == 0x25) {
                local_e7 = 9;
              }
              else if (*(short *)(&DAT_00d44990 + (short)local_10[0x23] * 0x8c) == 0x2ff) {
                local_e7 = 10;
              }
              else if (*(short *)(&DAT_00d44990 + (short)local_10[0x23] * 0x8c) == 0xafe) {
                local_e7 = 0xb;
              }
            }
            else {
              iVar1 = FUN_0054cd07(local_10 + 0x23,0x15);
              if (iVar1 == 0x68) {
                local_ec = 0x68;
                if (*(short *)(&DAT_00d44990 + (short)local_10[0x23] * 0x8c) == 0x36e) {
                  local_e7 = 0;
                }
                else if (*(short *)(&DAT_00d44990 + (short)local_10[0x23] * 0x8c) == 0x36f) {
                  local_e7 = 1;
                }
              }
            }
          }
        }
        if ((char)param_1[0x9bad] == '\x01') {
          param_1[0x9ba4] = (int)local_c;
        }
        _local_d8 = CONCAT22(uStack_d6,(short)local_c[8]);
        local_40 = (**(code **)(*DAT_013b71e8 + 0x34))((short)local_c[8]);
        local_a4 = (uint)*(byte *)((int)local_10 + 0x2e6);
        _local_d8 = CONCAT22(0xfffe,local_d8);
        local_f6 = DAT_005d0400;
        FUN_00555911(local_10,0,&DAT_005d0400,local_e5);
        local_f0 = __ftol();
        local_ee = __ftol();
        local_3c = 0x30;
        if (((char)local_10[5] == '\x03') && ((local_10[0xb7] & 0x200000U) != 0)) {
          local_100 = CONCAT22(local_100._2_2_,0x39e);
          local_3c = 0x34;
        }
        if (((char)local_10[5] == '\x03') && ((local_10[0xb7] & 0x40U) != 0)) {
          local_108 = __ftol();
          iVar1 = __ftol();
          local_108 = local_108 - iVar1;
          local_10c = __ftol();
          iVar1 = __ftol();
          local_10c = local_10c - iVar1;
          if (local_108 < 1) {
            if (local_108 < 0) {
              local_108 = -1;
            }
          }
          else {
            local_108 = 1;
          }
          if (local_10c < 1) {
            if (local_10c < 0) {
              local_10c = -1;
            }
          }
          else {
            local_10c = 1;
          }
          iVar1 = __ftol();
          iVar2 = __ftol();
          local_110 = *(int **)(param_1[0xd] + 0x10);
          while (local_110[3] != 0) {
            if ((((local_110 == (int *)param_1[0x13]) || (local_110 == local_c)) ||
                (iVar3 = __ftol(), iVar3 != iVar1 + local_108)) ||
               (iVar3 = __ftol(), iVar3 != iVar2 + local_10c)) {
              local_110 = (int *)local_110[3];
            }
            else if ((uint)local_110[8] < 1000) {
              iVar3 = (**(code **)(*local_110 + 0x54))();
              if ((iVar3 == 0) || ((char)local_110[0x92] == '\x01')) {
                local_110 = (int *)local_110[3];
              }
              else if (((DAT_005d0404 == 0) &&
                       ((short)DAT_013b71e8[0x3c4] != (short)local_110[0x1a0])) &&
                      (DAT_005d03f8 == 0)) {
                local_110 = (int *)local_110[3];
              }
              else if (((DAT_005d0404 == 0) && ('\0' < *(char *)(param_1[0x13] + 0x79b))) &&
                      ((*(char *)(param_1[0x13] + 0x79b) == *(char *)((int)local_110 + 0x79b) &&
                       (0 < DAT_005d03f8)))) {
                local_110 = (int *)local_110[3];
              }
              else if (((DAT_005d0404 == 0) && (0 < DAT_005d03f8)) &&
                      ((*(char *)(param_1[0x13] + 0x79b) == '\x03' &&
                       (((uint)local_110[8] < 1000 ||
                        (('\0' < *(char *)((int)local_110 + 0x79b) &&
                         (*(char *)((int)local_110 + 0x79b) != '\x04')))))))) {
                local_110 = (int *)local_110[3];
              }
              else {
                if (((DAT_005d0404 != 0) || (DAT_005d03f8 < 1)) ||
                   (iVar3 = (**(code **)(*local_110 + 0x58))(), iVar3 != 0)) {
LAB_00461993:
                  local_d4 = (undefined2)local_110[8];
                  local_d2 = -2;
                  break;
                }
                local_110 = (int *)local_110[3];
              }
            }
            else {
              if (((DAT_005d0404 != 0) || (*(char *)((int)local_110 + 0x249) != '\x01')) ||
                 ((((short)DAT_013b71e8[0x3c4] == (short)local_110[0x1a0] ||
                   ((short)local_110[0x1a0] == 0)) && ((short)local_110[0x1a0] != 0))))
              goto LAB_00461993;
              local_110 = (int *)local_110[3];
            }
          }
          if (((local_10[0xb7] & 0x200000U) == 0x200000) || (local_d2 == -2)) {
            local_100 = CONCAT22(local_100._2_2_,0x39e);
            local_3c = 0x34;
          }
          else {
            local_100 = CONCAT22(local_100._2_2_,0x39d);
            local_3c = 0x30;
          }
        }
        if ((*(int *)(param_1[0x13] + 0x240) != 0) &&
           (((local_ec == -1 || (local_ec == 0x68)) || (0x96 < local_ec)))) {
          *(undefined2 *)((int)param_1 + 0x28786) = *(undefined2 *)(param_1[0x13] + 0x20);
          *(undefined2 *)(param_1 + 0xa1e1) = 0x2cb;
          param_1[0xa1e7] = (uint)local_f4;
          param_1[0xa1e8] = (uint)uStack_f2;
          FUN_0055f2dd(param_1 + 0xa1e0,0x24);
        }
        FUN_0055f2dd(local_104,local_3c);
        FUN_0058f220(local_a0,local_104,0x60);
        local_9a = (undefined2)param_1[8];
        local_82 = 1;
        if (local_8 != 0) {
          local_81 = local_81 | 8;
        }
        local_92 = local_f6;
        (**(code **)(*param_1 + 4))(local_100 & 0xffff,local_a0);
        param_1[0x9d18] = in_stack_00000014;
      }
    }
  }
  return 1;
}

