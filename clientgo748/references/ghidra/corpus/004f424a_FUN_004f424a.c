// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f424a | Name: FUN_004f424a


void __fastcall FUN_004f424a(int param_1)

{
  byte bVar1;
  short sVar2;
  ushort uVar3;
  undefined4 uVar4;
  void *pvVar5;
  int iVar6;
  undefined4 *puVar7;
  undefined4 local_1c8;
  undefined4 local_1c0;
  int local_170;
  undefined4 local_16c;
  undefined1 local_168 [128];
  int local_e8;
  undefined1 local_e4;
  undefined4 local_e3;
  int local_64;
  byte local_60;
  undefined3 uStack_5f;
  undefined1 local_5c [64];
  undefined4 local_1c;
  int local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a16ef;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  uVar4 = FUN_0054cd07(param_1 + 0x160,0x26);
  *(undefined4 *)(param_1 + 0x1b8) = uVar4;
  local_1c = FUN_0054e06c(param_1 + 0x160);
  local_18 = FUN_00417215(param_1 + 0x160);
  if (*(int *)(param_1 + 0x1b8) == 10) {
    *(undefined4 *)(param_1 + 0x1c8) = 1;
  }
  if (local_18 != -0x555501) {
    *(undefined4 *)(param_1 + 0x1c8) = 1;
  }
  sVar2 = FUN_0054cd07(param_1 + 0x160,0x38);
  uVar3 = FUN_0054cd07(param_1 + 0x160,0x39);
  uVar3 = sVar2 << 8 | uVar3;
  local_14 = CONCAT22(local_14._2_2_,uVar3);
  if (*(int *)(param_1 + 0x1b8) == 2) {
    bVar1 = FUN_0054cd07(param_1 + 0x160,0x24);
    local_64 = (uint)bVar1 << 8;
    local_60 = bVar1;
    bVar1 = FUN_0054cd07(param_1 + 0x160,0x25);
    _local_60 = CONCAT31(uStack_5f,bVar1);
    local_64 = local_64 + (uint)bVar1;
    FUN_0058f078(local_5c,&DAT_00a361f8,local_64);
    if (*(int *)(param_1 + 0x1ac) != 0) {
      (**(code **)(**(int **)(param_1 + 0x1ac) + 0x80))(local_5c,0);
    }
  }
  else if (uVar3 == 0) {
    _memset(local_168,0,0x80);
    if (-1 < *(short *)(param_1 + 0x160)) {
      FUN_0058f078(local_168,&DAT_005bb414,&DAT_00d44950 + *(short *)(param_1 + 0x160) * 0x8c);
    }
    if (*(int *)(param_1 + 0x1ac) != 0) {
      (**(code **)(**(int **)(param_1 + 0x1ac) + 0x80))(local_168,0);
      (**(code **)(**(int **)(param_1 + 0x1ac) + 0x84))(0xffaaaaff);
    }
  }
  else {
    local_e4 = DAT_0092e7cc;
    puVar7 = &local_e3;
    for (iVar6 = 0x1f; iVar6 != 0; iVar6 = iVar6 + -1) {
      *puVar7 = 0;
      puVar7 = puVar7 + 1;
    }
    *(undefined2 *)puVar7 = 0;
    *(undefined1 *)((int)puVar7 + 2) = 0;
    *(uint *)(param_1 + 0x248) = local_14 & 0xfff;
    *(undefined4 *)(param_1 + 0x240) = 0;
    *(int *)(param_1 + 0x244) = (int)(local_14 & 0xffff) >> 0xc;
    *(undefined2 *)(param_1 + 0x2d8) = 1;
    if ((DAT_0067cf38 != 0) && (local_e8 = DAT_0067cf38, DAT_0067cf38 != 0)) {
      FUN_0047dee5(param_1 + 0x23c);
    }
    FUN_0058ee30(&local_e4,&DAT_00d44950 + *(short *)(param_1 + 0x160) * 0x8c);
    if (*(int *)(param_1 + 0x1ac) != 0) {
      (**(code **)(**(int **)(param_1 + 0x1ac) + 0x80))(&local_e4,0);
      if ((*(short *)(param_1 + 0x160) < 0xc49) || (0xc4d < *(short *)(param_1 + 0x160))) {
        (**(code **)(**(int **)(param_1 + 0x1ac) + 0x84))(0xffaaaaaa);
      }
      else {
        (**(code **)(**(int **)(param_1 + 0x1ac) + 0x84))(0xffffffaa);
      }
    }
  }
  pvVar5 = operator_new(0x124);
  local_8 = 0;
  if (pvVar5 == (void *)0x0) {
    local_1c0 = 0;
  }
  else {
    local_1c0 = FUN_004cb900(2,0,0x3f4ccccd,0x3f4ccccd,0x3f4ccccd,0,0);
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0x1c0) = local_1c0;
  if (*(int *)(param_1 + 0x1c0) != 0) {
    *(int *)(*(int *)(param_1 + 0x1c0) + 0x1c) = param_1;
    *(undefined4 *)(*(int *)(param_1 + 0x1c0) + 0x108) = 1;
    *(undefined4 *)(*(int *)(param_1 + 0x1c0) + 0x6c) = 1;
    *(undefined4 *)(*(int *)(param_1 + 0x1c0) + 0x104) = 2;
    *(undefined4 *)(*(int *)(param_1 + 0x1c0) + 0x10c) = 0x3dcccccd;
    if ((*(int *)(param_1 + 0x1b8) < 1) || (5 < *(int *)(param_1 + 0x1b8))) {
      if (*(int *)(param_1 + 0x1b8) == 10) {
        (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff006600);
        FUN_0054ac09(*(undefined4 *)(param_1 + 0x1c0));
      }
      else if (*(short *)(param_1 + 0x160) == 0x2eb) {
        if (*(int *)(param_1 + 0x1ac) != 0) {
          (**(code **)(**(int **)(param_1 + 0x1ac) + 0x84))(0xffffaa00);
        }
        (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff884400);
        FUN_0054ac09(*(undefined4 *)(param_1 + 0x1c0));
      }
      else if (((*(short *)(param_1 + 0x160) < 0x309) || (0x310 < *(short *)(param_1 + 0x160))) &&
              ((*(short *)(param_1 + 0x160) < 0xc62 || (0xc7f < *(short *)(param_1 + 0x160))))) {
        if ((*(short *)(param_1 + 0x160) != 0xf4a) && (*(short *)(param_1 + 0x160) != 0xf4b)) {
          if ((*(short *)(param_1 + 0x160) < 0xcee) || (0xdde < *(short *)(param_1 + 0x160))) {
            if (*(short *)(param_1 + 0x160) == 0x140f) {
              (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff664466);
              FUN_0054ac09(*(undefined4 *)(param_1 + 0x1c0));
              if (*(int *)(param_1 + 0x1ac) != 0) {
                (**(code **)(**(int **)(param_1 + 0x1ac) + 0x84))(0xffffffaa);
              }
            }
            else if ((*(short *)(param_1 + 0x160) < 0xfac) || (0xfd2 < *(short *)(param_1 + 0x160)))
            {
              if (*(short *)(param_1 + 0x160) == 0x14c4) {
                (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff444488);
                FUN_0054ac09(*(undefined4 *)(param_1 + 0x1c0));
                if (*(int *)(param_1 + 0x1ac) != 0) {
                  (**(code **)(**(int **)(param_1 + 0x1ac) + 0x84))(0xffffffaa);
                }
              }
              else if (((*(short *)(param_1 + 0x160) < 0x1005) ||
                       (0x1036 < *(short *)(param_1 + 0x160))) ||
                      (*(short *)(param_1 + 0x160) == 0x102f)) {
                if (((*(short *)(param_1 + 0x160) < 0xfaa) || (0xfab < *(short *)(param_1 + 0x160)))
                   && ((*(short *)(param_1 + 0x160) < 0xfba || (0xfbd < *(short *)(param_1 + 0x160))
                       ))) {
                  if (*(short *)(param_1 + 0x160) == 0x311) {
                    (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff555533);
                    FUN_0054ac09(*(undefined4 *)(param_1 + 0x1c0));
                  }
                  else if ((*(short *)(param_1 + 0x160) < 0x314) ||
                          (0x31a < *(short *)(param_1 + 0x160))) {
                    if (*(short *)(param_1 + 0x160) == 0x295) {
                      (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff660000);
                      FUN_0054ac09(*(undefined4 *)(param_1 + 0x1c0));
                    }
                    else if (*(short *)(param_1 + 0x160) == 0x296) {
                      (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff000066);
                      FUN_0054ac09(*(undefined4 *)(param_1 + 0x1c0));
                    }
                    else if (*(short *)(param_1 + 0x160) == 0x297) {
                      (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff006600);
                      FUN_0054ac09(*(undefined4 *)(param_1 + 0x1c0));
                    }
                    else if (*(short *)(param_1 + 0x160) == 0x29b) {
                      *(undefined4 *)(*(int *)(param_1 + 0x1c0) + 0xe0) = 0x3fb33333;
                      *(undefined4 *)(*(int *)(param_1 + 0x1c0) + 0xe4) = 0x3fb33333;
                      *(undefined4 *)(*(int *)(param_1 + 0x1c0) + 0xe8) = 0x3fb33333;
                      (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff000066);
                      FUN_0054ac09(*(undefined4 *)(param_1 + 0x1c0));
                    }
                    else if ((*(short *)(param_1 + 0x160) < 0x1b4) ||
                            (0x1b9 < *(short *)(param_1 + 0x160))) {
                      if ((*(short *)(param_1 + 0x160) < 0x6d0) ||
                         (0x6d7 < *(short *)(param_1 + 0x160))) {
                        if ((*(short *)(param_1 + 0x160) < 0x13f6) ||
                           (0x140d < *(short *)(param_1 + 0x160))) {
                          if (*(int *)(param_1 + 0x1c8) == 1) {
                            if (*(short *)(param_1 + 0x160) == 0x1a3) {
                              (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff774400);
                              FUN_0054ac09(*(undefined4 *)(param_1 + 0x1c0));
                            }
                            else if (*(short *)(param_1 + 0x160) == 0x1a4) {
                              (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff440077);
                              FUN_0054ac09(*(undefined4 *)(param_1 + 0x1c0));
                            }
                            else if (*(short *)(param_1 + 0x160) == 0x2f1) {
                              (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff660000);
                              FUN_0054ac09(*(undefined4 *)(param_1 + 0x1c0));
                            }
                            else {
                              *(undefined4 *)(*(int *)(param_1 + 0x1c0) + 0x10c) = 0x3d4ccccd;
                              local_16c = 0xff003300;
                              if (local_18 == -0x56) {
                                local_16c = 0xff555533;
                              }
                              else if (local_18 == -0x5600) {
                                local_16c = 0xff884400;
                              }
                              (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(local_16c);
                              FUN_0054ac09(*(undefined4 *)(param_1 + 0x1c0));
                            }
                            if (*(int *)(param_1 + 0x1ac) != 0) {
                              (**(code **)(**(int **)(param_1 + 0x1ac) + 0x84))(local_18);
                            }
                          }
                          else {
                            if (*(int *)(param_1 + 0x1ac) != 0) {
                              (**(code **)(**(int **)(param_1 + 0x1ac) + 0x84))(local_18);
                            }
                            if (*(int *)(param_1 + 0x1c0) != 0) {
                              if (*(undefined4 **)(param_1 + 0x1c0) != (undefined4 *)0x0) {
                                (**(code **)**(undefined4 **)(param_1 + 0x1c0))(1);
                              }
                              *(undefined4 *)(param_1 + 0x1c0) = 0;
                            }
                          }
                        }
                        else {
                          uVar4 = FUN_00430f50(0xbf000000,0,0xbf4ccccd);
                          FUN_004310a0(uVar4);
                          uVar4 = FUN_00430f50(0x3f666666,0,0xbf4ccccd);
                          FUN_004310a0(uVar4);
                          uVar4 = FUN_00430f50(0x3f666666,0,0x3f000000);
                          FUN_004310a0(uVar4);
                          uVar4 = FUN_00430f50(0xbf000000,0,0x3f000000);
                          FUN_004310a0(uVar4);
                          *(undefined4 *)(*(int *)(param_1 + 0x1c0) + 0xe0) = 0x3f800000;
                          *(undefined4 *)(*(int *)(param_1 + 0x1c0) + 0xe4) = 0x3f800000;
                          *(undefined4 *)(*(int *)(param_1 + 0x1c0) + 0xe8) = 0x3f800000;
                          *(undefined4 *)(*(int *)(param_1 + 0x1c0) + 0x10c) = 0xbcf5c28f;
                          (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff774400);
                          FUN_0054ac09(*(undefined4 *)(param_1 + 0x1c0));
                        }
                      }
                    }
                    else {
                      *(undefined4 *)(*(int *)(param_1 + 0x1c0) + 0xe0) = 0x3fb33333;
                      *(undefined4 *)(*(int *)(param_1 + 0x1c0) + 0xe4) = 0x3fb33333;
                      *(undefined4 *)(*(int *)(param_1 + 0x1c0) + 0xe8) = 0x3fb33333;
                      if (*(short *)(param_1 + 0x160) < 0x1b7) {
                        (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff000066);
                      }
                      else {
                        (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff660000);
                      }
                      FUN_0054ac09(*(undefined4 *)(param_1 + 0x1c0));
                    }
                  }
                  else {
                    (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff555533);
                    FUN_0054ac09(*(undefined4 *)(param_1 + 0x1c0));
                  }
                }
                else {
                  (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff666666);
                  FUN_0054ac09(*(undefined4 *)(param_1 + 0x1c0));
                  if (*(int *)(param_1 + 0x1ac) != 0) {
                    (**(code **)(**(int **)(param_1 + 0x1ac) + 0x84))(0xffffffaa);
                  }
                }
              }
              else {
                (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff444488);
                FUN_0054ac09(*(undefined4 *)(param_1 + 0x1c0));
                if (*(int *)(param_1 + 0x1ac) != 0) {
                  (**(code **)(**(int **)(param_1 + 0x1ac) + 0x84))(0xffffffaa);
                }
              }
            }
            else {
              (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff666644);
              FUN_0054ac09(*(undefined4 *)(param_1 + 0x1c0));
              if (*(int *)(param_1 + 0x1ac) != 0) {
                (**(code **)(**(int **)(param_1 + 0x1ac) + 0x84))(0xffffffaa);
              }
            }
          }
          else {
            (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff664466);
            FUN_0054ac09(*(undefined4 *)(param_1 + 0x1c0));
            if (*(int *)(param_1 + 0x1ac) != 0) {
              (**(code **)(**(int **)(param_1 + 0x1ac) + 0x84))(0xffffffaa);
            }
          }
        }
      }
      else {
        (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff660000);
        FUN_0054ac09(*(undefined4 *)(param_1 + 0x1c0));
        if (*(int *)(param_1 + 0x1ac) != 0) {
          (**(code **)(**(int **)(param_1 + 0x1ac) + 0x84))(0xffffffaa);
        }
      }
    }
    else {
      if (*(int *)(param_1 + 0x1b8) == 1) {
        iVar6 = FUN_0054cd07(param_1 + 0x160,4);
        if (iVar6 < 1) {
          (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff000066);
        }
        else {
          *(undefined4 *)(param_1 + 0x1c4) = 1;
          if (*(short *)(param_1 + 0x160) == 0x6cb) {
            (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xffaa8888);
          }
          else {
            (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff660000);
          }
        }
      }
      else if (*(int *)(param_1 + 0x1b8) == 2) {
        (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff666600);
      }
      else if (*(int *)(param_1 + 0x1b8) == 3) {
        (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff666600);
      }
      else if (*(int *)(param_1 + 0x1b8) == 4) {
        if (*(int *)(param_1 + 0x1ac) != 0) {
          (**(code **)(**(int **)(param_1 + 0x1ac) + 0x84))(0xffffffaa);
        }
        (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff774400);
      }
      else if (*(int *)(param_1 + 0x1b8) == 5) {
        if (*(int *)(param_1 + 0x1ac) != 0) {
          (**(code **)(**(int **)(param_1 + 0x1ac) + 0x84))(0xffffffaa);
        }
        (**(code **)(**(int **)(param_1 + 0x1c0) + 0x3c))(0xff440077);
      }
      FUN_0054ac09(*(undefined4 *)(param_1 + 0x1c0));
    }
  }
  if (*(short *)(param_1 + 0x160) == 0x6bf) {
    for (local_170 = 0; local_170 < 0x1c; local_170 = local_170 + 1) {
      if (*(int *)(param_1 + 0x1cc + local_170 * 4) != 0) {
        (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x1cc + local_170 * 4));
        *(undefined4 *)(param_1 + 0x1cc + local_170 * 4) = 0;
      }
      pvVar5 = operator_new(0x180);
      local_8 = 1;
      if (pvVar5 == (void *)0x0) {
        local_1c8 = 0;
      }
      else {
        local_1c8 = FUN_004c8f70(0x38,0,0x3fc00000,0x3fc00000,0x3fc00000,0,1,0x50);
      }
      local_8 = 0xffffffff;
      *(undefined4 *)(param_1 + 0x1cc + local_170 * 4) = local_1c8;
      if (*(int *)(param_1 + 0x1cc + local_170 * 4) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x1cc + local_170 * 4) + 0x104) = 2;
        if (local_170 < 0xe) {
          (**(code **)(**(int **)(param_1 + 0x1cc + local_170 * 4) + 0x3c))(0xffaaaa00);
        }
        else if (local_170 < 0x17) {
          (**(code **)(**(int **)(param_1 + 0x1cc + local_170 * 4) + 0x3c))(0xff0088ff);
        }
        else {
          (**(code **)(**(int **)(param_1 + 0x1cc + local_170 * 4) + 0x3c))(0xffff5500);
        }
        *(undefined4 *)(*(int *)(param_1 + 0x1cc + local_170 * 4) + 0x6c) = 1;
        FUN_0054ac09(*(undefined4 *)(param_1 + 0x1cc + local_170 * 4));
      }
    }
  }
  *(undefined4 *)(param_1 + 0x2dc) =
       *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x1ac) + 0x870) + 8);
  FUN_0053e314();
  ExceptionList = local_10;
  return;
}

