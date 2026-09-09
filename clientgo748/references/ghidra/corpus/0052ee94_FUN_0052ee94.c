// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052ee94 | Name: FUN_0052ee94


undefined4 __thiscall FUN_0052ee94(int param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int *local_110;
  int local_10c;
  int local_108;
  undefined1 local_104 [4];
  undefined2 local_100;
  undefined2 local_fe;
  undefined2 local_f8;
  undefined2 local_f6;
  undefined2 local_f4;
  undefined2 local_f2;
  undefined2 local_f0;
  undefined2 local_ee;
  undefined2 local_ec;
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
  undefined1 local_82;
  byte local_81;
  int *local_40;
  undefined4 local_3c;
  int local_38;
  undefined4 local_34;
  int local_30;
  int local_2c;
  int *local_28;
  undefined4 local_24;
  int *local_20;
  undefined4 local_1c;
  int local_18;
  undefined4 local_14;
  int local_10;
  int local_c;
  uint local_8;
  
  if (DAT_0067cf38[9] == 30000) {
    if (*(char *)(param_2 + 0x245) == '\x01') {
      uVar1 = 0;
    }
    else if (*(char *)(param_2 + 0x795) == '\x01') {
      uVar1 = 0;
    }
    else if (*(int *)(param_1 + 0x214) == 0) {
      if (param_2 == 0) {
        uVar1 = 0;
      }
      else if (*(uint *)(param_2 + 0x20) < 1000) {
        uVar1 = 0;
      }
      else {
        iVar2 = FUN_00516105();
        if (iVar2 == 0) {
          if (*(char *)(param_2 + 0x249) == '\x01') {
            uVar1 = 0;
          }
          else if (((((*(byte *)(param_1 + 0x44a) & 0xf) == 0xf) &&
                    (local_38 = DAT_0067cf38[0x13], '\0' < *(char *)(param_1 + 0x79b))) &&
                   ('\0' < *(char *)(local_38 + 0x79b))) &&
                  (*(char *)(param_1 + 0x79b) == *(char *)(local_38 + 0x79b))) {
            uVar1 = 0;
          }
          else {
            local_20 = DAT_0067cf38;
            local_8 = (**(code **)(*DAT_0092e654 + 8))();
            local_28 = DAT_013b71e8 + 0x1bb;
            local_18 = 0;
            if ((local_28 != (int *)0x0) && ((DAT_013b71e8[0x272] & 0x20000000U) != 0)) {
              local_18 = 1;
            }
            local_14 = 0;
            if (local_8 < local_20[0x9d18] + 1000U) {
              uVar1 = 0;
            }
            else {
              local_24 = __ftol();
              local_34 = __ftol();
              local_c = __ftol();
              local_10 = __ftol();
              local_30 = FUN_005563a0(local_24,local_34,local_c,local_10);
              local_2c = FUN_0054eb20(DAT_013b71e8 + 0x1bb,0x1b);
              local_2c = local_2c + local_18;
              FUN_00556416(local_24,local_34,&local_c,&local_10,local_20 + 0x25,8);
              local_1c = 3;
              DAT_0092ec9c = (DAT_0092ec9c + 1) % 3;
              if (((local_2c < local_30) || (iVar2 = __ftol(), local_c != iVar2)) ||
                 (iVar2 = __ftol(), local_10 != iVar2)) {
                uVar1 = 0;
              }
              else {
                _memset(local_104,0,0x60);
                local_100 = 0x39d;
                local_fe = *(undefined2 *)(param_1 + 0x20);
                local_f8 = *(undefined2 *)(param_1 + 0x20);
                local_f4 = __ftol();
                local_f2 = __ftol();
                local_ea = 0xffff;
                local_ec = 0xffff;
                local_e7 = 0;
                local_e8 = (char)DAT_0092ec9c + '\x04';
                iVar2 = FUN_0054cd07(local_28 + 0x23,0x15);
                if (iVar2 == 0x65) {
                  local_ec = 0x97;
                }
                else {
                  iVar2 = FUN_0054cd07(local_28 + 0x23,0x15);
                  if (iVar2 == 0x66) {
                    local_ec = 0x98;
                    if (*(short *)(&DAT_00d44990 + (short)local_28[0x23] * 0x8c) == 0x367) {
                      local_e7 = 0;
                    }
                    else if (*(short *)(&DAT_00d44990 + (short)local_28[0x23] * 0x8c) == 0x368) {
                      local_e7 = 1;
                    }
                  }
                  else {
                    iVar2 = FUN_0054cd07(local_28 + 0x23,0x15);
                    if (iVar2 == 0x67) {
                      local_ec = 0x99;
                      if (*(short *)(&DAT_00d44990 + (short)local_28[0x23] * 0x8c) == 0x369) {
                        local_e7 = 0;
                      }
                      else if (*(short *)(&DAT_00d44990 + (short)local_28[0x23] * 0x8c) == 0x36a) {
                        local_e7 = 1;
                      }
                      else if (*(short *)(&DAT_00d44990 + (short)local_28[0x23] * 0x8c) == 0x36b) {
                        local_e7 = 2;
                      }
                      else if (*(short *)(&DAT_00d44990 + (short)local_28[0x23] * 0x8c) == 0x36c) {
                        local_e7 = 3;
                      }
                      else if (*(short *)(&DAT_00d44990 + (short)local_28[0x23] * 0x8c) == 0x36d) {
                        local_e7 = 4;
                      }
                      else if (*(short *)(&DAT_00d44990 + (short)local_28[0x23] * 0x8c) == 0x37c) {
                        local_e7 = 5;
                      }
                      else if (*(short *)(&DAT_00d44990 + (short)local_28[0x23] * 0x8c) == 0x38b) {
                        local_e7 = 6;
                      }
                      else if (*(short *)(&DAT_00d44990 + (short)local_28[0x23] * 0x8c) == 0x38c) {
                        local_e7 = 7;
                      }
                      else if (*(short *)(&DAT_00d44990 + (short)local_28[0x23] * 0x8c) == 0x38d) {
                        local_e7 = 8;
                      }
                      else if (*(short *)(&DAT_00d44990 + (short)local_28[0x23] * 0x8c) == 0x25) {
                        local_e7 = 9;
                      }
                      else if (*(short *)(&DAT_00d44990 + (short)local_28[0x23] * 0x8c) == 0x2ff) {
                        local_e7 = 10;
                      }
                      else if (*(short *)(&DAT_00d44990 + (short)local_28[0x23] * 0x8c) == 0xafe) {
                        local_e7 = 0xb;
                      }
                    }
                    else {
                      iVar2 = FUN_0054cd07(local_28 + 0x23,0x15);
                      if (iVar2 == 0x68) {
                        local_ec = 0x68;
                        if (*(short *)(&DAT_00d44990 + (short)local_28[0x23] * 0x8c) == 0x36e) {
                          local_e7 = 0;
                        }
                        else if (*(short *)(&DAT_00d44990 + (short)local_28[0x23] * 0x8c) == 0x36f)
                        {
                          local_e7 = 1;
                        }
                      }
                    }
                  }
                }
                _local_d8 = CONCAT22(uStack_d6,*(undefined2 *)(param_2 + 0x20));
                local_40 = (int *)(**(code **)(*DAT_013b71e8 + 0x34))
                                            (*(undefined2 *)(param_2 + 0x20));
                if (local_40 == (int *)0x0) {
                  uVar1 = 0;
                }
                else {
                  local_a4 = (uint)*(byte *)((int)DAT_013b71e8 + 0x9d2);
                  _local_d8 = CONCAT22(0xfffe,local_d8);
                  local_f6 = DAT_005d0400;
                  FUN_00555911(DAT_013b71e8 + 0x1bb,0,&DAT_005d0400,local_e5);
                  local_f0 = __ftol();
                  local_ee = __ftol();
                  local_3c = 0x60;
                  if (((char)local_28[5] == '\x03') && ((local_28[0xb7] & 0x200000U) != 0)) {
                    local_100 = 0x39e;
                    local_3c = 0x34;
                  }
                  if (((char)local_28[5] == '\x03') && ((local_28[0xb7] & 0x40U) != 0)) {
                    local_108 = __ftol();
                    local_10c = __ftol();
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
                    iVar2 = __ftol();
                    iVar3 = __ftol();
                    local_110 = *(int **)(local_20[0xd] + 0x10);
                    while (local_110[3] != 0) {
                      if ((((local_110 == (int *)local_20[0x13]) || (local_110 == local_40)) ||
                          (iVar4 = __ftol(), iVar4 != iVar2 + local_108)) ||
                         (iVar4 = __ftol(), iVar4 != iVar3 + local_10c)) {
                        local_110 = (int *)local_110[3];
                      }
                      else if ((uint)local_110[8] < 1000) {
                        iVar4 = (**(code **)(*local_110 + 0x54))();
                        if ((iVar4 == 0) || ((char)local_110[0x92] == '\x01')) {
                          local_110 = (int *)local_110[3];
                        }
                        else if (((DAT_005d0404 == 0) &&
                                 ((short)DAT_013b71e8[0x3c4] != (short)local_110[0x1a0])) &&
                                (DAT_005d03f8 == 0)) {
                          local_110 = (int *)local_110[3];
                        }
                        else if (((DAT_005d0404 == 0) && ('\0' < *(char *)(param_1 + 0x79b))) &&
                                ((*(char *)(param_1 + 0x79b) == *(char *)((int)local_110 + 0x79b) &&
                                 (0 < DAT_005d03f8)))) {
                          local_110 = (int *)local_110[3];
                        }
                        else if (((DAT_005d0404 == 0) && (0 < DAT_005d03f8)) &&
                                ((*(char *)(param_1 + 0x79b) == '\x03' &&
                                 (((uint)local_110[8] < 1000 ||
                                  (('\0' < *(char *)((int)local_110 + 0x79b) &&
                                   (*(char *)((int)local_110 + 0x79b) != '\x04')))))))) {
                          local_110 = (int *)local_110[3];
                        }
                        else {
                          if (((DAT_005d0404 != 0) || (DAT_005d03f8 < 1)) ||
                             (iVar4 = (**(code **)(*local_110 + 0x58))(), iVar4 != 0)) {
LAB_0052f9a7:
                            local_d4 = (undefined2)local_110[8];
                            local_d2 = -2;
                            break;
                          }
                          local_110 = (int *)local_110[3];
                        }
                      }
                      else {
                        if (((DAT_005d0404 != 0) || (*(char *)((int)local_110 + 0x249) != '\x01'))
                           || ((((short)DAT_013b71e8[0x3c4] == (short)local_110[0x1a0] ||
                                ((short)local_110[0x1a0] == 0)) && ((short)local_110[0x1a0] != 0))))
                        goto LAB_0052f9a7;
                        local_110 = (int *)local_110[3];
                      }
                    }
                    if (((local_28[0xb7] & 0x200000U) == 0x200000) || (local_d2 == -2)) {
                      local_100 = 0x39e;
                      local_3c = 0x34;
                    }
                    else {
                      local_100 = 0x39d;
                      local_3c = 0x30;
                    }
                  }
                  FUN_0055f2dd(local_104,local_3c);
                  FUN_0058f220(local_a0,local_104,local_3c);
                  local_9a = (undefined2)local_20[8];
                  local_82 = 1;
                  if ((local_28[0xb7] & 0x20000000U) != 0) {
                    local_81 = local_81 | 4;
                  }
                  (**(code **)(*local_20 + 4))(0x39d,local_a0);
                  local_20[0x9d18] = local_8;
                  uVar1 = 1;
                }
              }
            }
          }
        }
        else {
          uVar1 = 0;
        }
      }
    }
    else {
      uVar1 = 0;
    }
  }
  else {
    DAT_005d03f4 = 0;
    uVar1 = 0;
  }
  return uVar1;
}

