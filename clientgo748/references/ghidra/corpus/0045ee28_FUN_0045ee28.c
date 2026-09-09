// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0045ee28 | Name: FUN_0045ee28


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall
FUN_0045ee28(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5
            ,undefined4 param_6,int param_7)

{
  bool bVar1;
  int iVar2;
  undefined1 local_94 [4];
  undefined2 local_90;
  undefined2 local_8e;
  undefined4 local_88;
  undefined4 local_84;
  int *local_80;
  int local_7c;
  char local_78;
  char local_74;
  undefined1 local_70 [4];
  undefined2 local_6c;
  undefined2 local_6a;
  undefined4 local_64;
  undefined4 local_60;
  int local_5c;
  int local_58;
  int local_54;
  undefined1 local_50 [4];
  undefined2 local_4c;
  undefined2 local_4a;
  undefined2 local_44;
  undefined1 local_40 [4];
  undefined2 local_3c;
  undefined2 local_3a;
  undefined4 local_34;
  undefined4 local_30;
  int local_2c;
  undefined1 local_28 [4];
  undefined2 local_24;
  undefined2 local_22;
  undefined2 local_1c;
  undefined1 local_18 [4];
  undefined2 local_14;
  undefined2 local_12;
  undefined4 local_c;
  int local_8;
  
  local_8 = *(int *)(param_1 + 0x7c);
  if (*(int *)(*(int *)(param_1 + 0x27b38) + 0x28) == 0) {
    *(undefined4 *)(param_1 + 0x28e60) = 0;
    if ((local_8 == 0) || (*(int *)(local_8 + 0x238) != 1)) {
      if ((1000 < (uint)(param_7 - *(int *)(param_1 + 0x275c0))) &&
         (((*(int *)(param_1 + 0x275c0) != 0 &&
           (*(char *)(*(int *)(param_1 + 0x4c) + 0x247) == '\0')) &&
          (0 < *(short *)(*(int *)(param_1 + 0x4c) + 0x450))))) {
        FUN_0046049d(param_4,param_5,param_6);
      }
    }
    else {
      iVar2 = __ftol();
      if (((iVar2 >> 7 < 2) || (iVar2 = __ftol(), 10 < iVar2 >> 7)) ||
         (iVar2 = __ftol(), 4 < iVar2 >> 7)) {
        bVar1 = false;
      }
      else {
        bVar1 = true;
      }
      if ((!bVar1) && (999 < (uint)(param_7 - *(int *)(param_1 + 0x2747c)))) {
        if (*(char *)(local_8 + 0x724) == '\0') {
          if (((*(uint *)(local_8 + 0x20) < 1000) || (*(short *)(local_8 + 0x250) != 0x33)) ||
             (((iVar2 = __ftol(), iVar2 >> 7 != 0xd && (iVar2 = __ftol(), iVar2 >> 7 != 0xe)) ||
              (iVar2 = __ftol(), iVar2 >> 7 != 0x1c)))) {
            bVar1 = false;
          }
          else {
            bVar1 = true;
          }
          if (bVar1) {
            *(undefined4 *)(param_1 + 0x275d8) = *(undefined4 *)(local_8 + 0x20);
            FUN_0044a3a9(*(int *)(*(int *)(param_1 + 0x27bdc) + 0x28) == 0);
            *(uint *)(*(int *)(param_1 + 0x2877c) + 0x404) = (uint)*(ushort *)(local_8 + 0x20);
            *(int *)(param_1 + 0x2747c) = param_7;
          }
          else if ((*(uint *)(local_8 + 0x20) < 1000) || ((*(byte *)(local_8 + 0x44a) & 0xf) != 1))
          {
            if ((*(uint *)(local_8 + 0x20) < 1000) || ((*(byte *)(local_8 + 0x44a) & 0xf) != 2)) {
              if ((*(uint *)(local_8 + 0x20) < 1000) || ((*(byte *)(local_8 + 0x44a) & 0xf) != 3)) {
                if ((*(uint *)(local_8 + 0x20) < 1000) ||
                   (((*(short *)(local_8 + 0x250) != 0x43 ||
                     (*(int *)(*(int *)(param_1 + 0x40) + 0x20a20) != 0xd)) ||
                    (*(int *)(*(int *)(param_1 + 0x40) + 0x20a24) != 0xd)))) {
                  if (((*(uint *)(local_8 + 0x20) < 1000) || (*(short *)(local_8 + 0x250) != 0x43))
                     || ((*(int *)(*(int *)(param_1 + 0x40) + 0x20a20) != 0x1c ||
                         ((*(int *)(*(int *)(param_1 + 0x40) + 0x20a24) != 0x18 ||
                          (*(int *)(local_8 + 0x20) == 0x409)))))) {
                    if ((*(uint *)(local_8 + 0x20) < 1000) ||
                       (((*(short *)(local_8 + 0x250) != 0x36 ||
                         (*(int *)(*(int *)(param_1 + 0x40) + 0x20a20) != 0x13)) ||
                        (*(int *)(*(int *)(param_1 + 0x40) + 0x20a24) != 0xd)))) {
                      if (((*(uint *)(local_8 + 0x20) < 1000) ||
                          (*(short *)(local_8 + 0x250) != 0x36)) ||
                         ((*(int *)(*(int *)(param_1 + 0x40) + 0x20a20) != 0x19 ||
                          (*(int *)(*(int *)(param_1 + 0x40) + 0x20a24) != 0xd)))) {
                        if ((*(uint *)(local_8 + 0x20) < 1000) ||
                           (*(short *)(local_8 + 0x250) != 0x37)) {
                          if ((*(uint *)(local_8 + 0x20) < 1000) ||
                             (*(short *)(local_8 + 0x250) != 0x38)) {
                            if ((((*(uint *)(local_8 + 0x20) < 1000) ||
                                 (*(short *)(local_8 + 0x250) != 0x44)) ||
                                (*(int *)(*(int *)(param_1 + 0x40) + 0x20a20) != 0x13)) ||
                               (*(int *)(*(int *)(param_1 + 0x40) + 0x20a24) != 0xf)) {
                              if ((*(uint *)(local_8 + 0x20) < 1000) ||
                                 (*(short *)(local_8 + 0x250) != 0x44)) {
                                if ((((*(uint *)(local_8 + 0x20) < 1000) ||
                                     ((*(short *)(local_8 + 0x250) != 0x43 ||
                                      ((*(byte *)(local_8 + 0x44a) & 0xf) != 8)))) ||
                                    (*(int *)(*(int *)(param_1 + 0x40) + 0x20a20) != 0x19)) ||
                                   (*(int *)(*(int *)(param_1 + 0x40) + 0x20a24) != 0xd)) {
                                  if ((*(uint *)(local_8 + 0x20) < 1000) ||
                                     (*(short *)(local_8 + 0x250) != 0x39)) {
                                    if (((*(uint *)(local_8 + 0x20) < 1000) ||
                                        (((*(short *)(local_8 + 0x250) != 0x3f ||
                                          ((*(byte *)(local_8 + 0x44a) & 0xf) != 7)) ||
                                         (*(int *)(*(int *)(param_1 + 0x40) + 0x20a20) != 0x10))))
                                       || (*(int *)(*(int *)(param_1 + 0x40) + 0x20a24) != 0x10)) {
                                      if (((*(uint *)(local_8 + 0x20) < 1000) ||
                                          ((*(byte *)(local_8 + 0x44a) & 0xf) != 4)) &&
                                         (((*(byte *)(local_8 + 0x44a) & 0xf) < 8 ||
                                          (0xf < (*(byte *)(local_8 + 0x44a) & 0xf))))) {
                                        if (((999 < *(uint *)(local_8 + 0x20)) &&
                                            (5 < (*(byte *)(local_8 + 0x44a) & 0xf))) &&
                                           ((*(byte *)(local_8 + 0x44a) & 0xf) < 9)) {
                                          _memset(local_94,0,0x10);
                                          local_90 = 0x28e;
                                          local_8e = *(undefined2 *)
                                                      (*(int *)(param_1 + 0x4c) + 0x20);
                                          *(undefined4 *)(param_1 + 0x26edc) =
                                               *(undefined4 *)(local_8 + 0x20);
                                          local_88 = *(undefined4 *)(param_1 + 0x26edc);
                                          FUN_0055f2dd(local_94,0x10);
                                          *(int *)(param_1 + 0x2747c) = param_7;
                                        }
                                      }
                                      else {
                                        if (((*(byte *)(local_8 + 0x44a) & 0xf) == 0xf) &&
                                           (iVar2 = (**(code **)(**(int **)(param_1 + 0x4c) + 0x50))
                                                              (), iVar2 == 0)) {
                                          if (('\0' < *(char *)(local_8 + 0x79b)) &&
                                             ((('\0' < *(char *)(*(int *)(param_1 + 0x4c) + 0x79b)
                                               && (*(char *)(local_8 + 0x79b) !=
                                                   *(char *)(*(int *)(param_1 + 0x4c) + 0x79b))) &&
                                              (*(char *)(*(int *)(param_1 + 0x4c) + 0x79b) != '\x03'
                                              )))) {
                                            return 1;
                                          }
                                          if ((*(int *)(*(int *)(param_1 + 0x4c) + 0x198) != 0) &&
                                             ((*(ushort *)
                                                (*(int *)(*(int *)(param_1 + 0x4c) + 0x198) + 0x24a)
                                               < 2 || ((7 < *(ushort *)
                                                             (*(int *)(*(int *)(param_1 + 0x4c) +
                                                                      0x198) + 0x24a) &&
                                                       (*(ushort *)
                                                         (*(int *)(*(int *)(param_1 + 0x4c) + 0x198)
                                                         + 0x24a) < 0xf)))))) {
                                            if ((*(short *)(DAT_013b71e8 + 0x798) == 0x6ce) &&
                                               ((*(short *)(DAT_013b71e8 + 0x7a0) < 0x6e0 ||
                                                (0x6e3 < *(short *)(DAT_013b71e8 + 0x7a0))))) {
                                              FUN_00403df2(&DAT_00a3b9f8,4000);
                                              (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
                                              return 1;
                                            }
                                            if (*(short *)(DAT_013b71e8 + 0x798) != 0x6ce) {
                                              return 1;
                                            }
                                            if (*(short *)(DAT_013b71e8 + 0x7a0) < 0x6e0) {
                                              return 1;
                                            }
                                            if (0x6e3 < *(short *)(DAT_013b71e8 + 0x7a0)) {
                                              return 1;
                                            }
                                          }
                                        }
                                        if ((*(byte *)(local_8 + 0x44a) & 0xf) == 0xd) {
                                          iVar2 = FUN_0040c0f0();
                                          if (iVar2 == 0) {
                                            (**(code **)(**(int **)(param_1 + 0x8c) + 0x8c))
                                                      (&DAT_00a382f8,0xd,0);
                                            *(undefined4 *)(*(int *)(param_1 + 0x8c) + 0x1e8) =
                                                 *(undefined4 *)(local_8 + 0x20);
                                            (**(code **)(**(int **)(param_1 + 0x8c) + 0x60))(1);
                                          }
                                        }
                                        else {
                                          if ((*(byte *)(local_8 + 0x44a) & 0xf) == 0xe) {
                                            local_78 = '\0';
                                            local_74 = '\0';
                                            for (local_7c = 0; local_7c < 0x3f;
                                                local_7c = local_7c + 1) {
                                              if ((*(short *)(DAT_013b71e8 + 0x7c8 + local_7c * 8)
                                                   == 0x6cc) &&
                                                 (*(short *)(DAT_013b71e8 + 2000 + local_7c * 8) ==
                                                  0x6cd)) {
                                                local_78 = '\x01';
                                              }
                                              if (*(short *)(DAT_013b71e8 + 0x7c8 + local_7c * 8) ==
                                                  0x2b9) {
                                                local_74 = local_74 + '\x01';
                                              }
                                              if (((local_78 == '\x01') && ('\x13' < local_74)) &&
                                                 (0x12a < *(short *)(*(int *)(param_1 + 0x4c) +
                                                                    0x444))) {
                                                iVar2 = FUN_0040c0f0();
                                                if (iVar2 != 0) {
                                                  return 1;
                                                }
                                                (**(code **)(**(int **)(param_1 + 0x8c) + 0x8c))
                                                          (&DAT_00a3b5f8,0xe9,0);
                                                *(undefined4 *)(*(int *)(param_1 + 0x8c) + 0x1e8) =
                                                     *(undefined4 *)(local_8 + 0x20);
                                                (**(code **)(**(int **)(param_1 + 0x8c) + 0x60))(1);
                                                return 1;
                                              }
                                            }
                                          }
                                          if (((*(byte *)(local_8 + 0x44a) & 0xf) == 0xf) ||
                                             ((*(byte *)(local_8 + 0x44a) & 0xf) == 10)) {
                                            iVar2 = FUN_0040c0f0();
                                            if (iVar2 == 0) {
                                              (**(code **)(**(int **)(param_1 + 0x8c) + 0x8c))
                                                        (&DAT_00a38d78,
                                                         *(byte *)(local_8 + 0x44a) & 0xf,0);
                                              *(undefined4 *)(*(int *)(param_1 + 0x8c) + 0x1e8) =
                                                   *(undefined4 *)(local_8 + 0x20);
                                              (**(code **)(**(int **)(param_1 + 0x8c) + 0x60))(1);
                                            }
                                          }
                                          else if (((*(byte *)(local_8 + 0x44a) & 0xf) == 4) &&
                                                  (*(short *)(local_8 + 0x250) == 0x10f)) {
                                            iVar2 = FUN_0040c0f0();
                                            if (iVar2 == 0) {
                                              (**(code **)(**(int **)(param_1 + 0x8c) + 0x8c))
                                                        (&DAT_00a3ef78,0x10f,0);
                                              *(undefined4 *)(*(int *)(param_1 + 0x8c) + 0x1e8) =
                                                   *(undefined4 *)(local_8 + 0x20);
                                              (**(code **)(**(int **)(param_1 + 0x8c) + 0x60))(1);
                                            }
                                          }
                                          else {
                                            if ((*(uint *)(local_8 + 0x20) < 1000) ||
                                               (((*(byte *)(local_8 + 0x44a) & 0xf) != 9 ||
                                                (*(short *)(local_8 + 0x250) != 0x33)))) {
                                              if ((999 < *(uint *)(local_8 + 0x20)) &&
                                                 ((*(short *)(local_8 + 0x250) == 0x3a &&
                                                  ((*(byte *)(local_8 + 0x44a) & 0xf) == 0xb)))) {
                                                iVar2 = FUN_0040c0f0();
                                                if (iVar2 != 0) {
                                                  return 1;
                                                }
                                                (**(code **)(**(int **)(param_1 + 0x8c) + 0x8c))
                                                          (&DAT_00a38d78,
                                                           (int)*(short *)(local_8 + 0x250),0);
                                                *(undefined4 *)(*(int *)(param_1 + 0x8c) + 0x1e8) =
                                                     *(undefined4 *)(local_8 + 0x20);
                                                (**(code **)(**(int **)(param_1 + 0x8c) + 0x60))(1);
                                                return 1;
                                              }
                                            }
                                            else {
                                              (**(code **)(**(int **)(param_1 + 0x27c78) + 0x60))(1)
                                              ;
                                              local_80 = (int *)(**(code **)(**(int **)(param_1 +
                                                                                       0x28) + 0x48)
                                                                )(0x276);
                                              if (local_80 != (int *)0x0) {
                                                *(undefined4 *)(param_1 + 0x26ee0) = 7;
                                                (**(code **)(*local_80 + 0x80))(&DAT_00a38578,0);
                                                local_84 = (**(code **)(**(int **)(DAT_0067cf38 +
                                                                                  0x28) + 0x48))
                                                                     (0x273);
                                                (**(code **)(**(int **)(param_1 + 0x28) + 0x40))
                                                          (local_84);
                                              }
                                            }
                                            _memset(local_70,0,0x14);
                                            local_6c = 0x28b;
                                            local_6a = *(undefined2 *)
                                                        (*(int *)(param_1 + 0x4c) + 0x20);
                                            local_64 = *(undefined4 *)(local_8 + 0x20);
                                            local_60 = 0;
                                            FUN_0055f2dd(local_70,0x14);
                                            *(int *)(param_1 + 0x2747c) = param_7;
                                          }
                                        }
                                      }
                                    }
                                    else {
                                      FUN_0047fcd7(1);
                                    }
                                  }
                                  else if (DAT_013b736c == 0) {
                                    local_5c = 0;
                                    for (local_58 = 0; local_58 < 0x7e; local_58 = local_58 + 1) {
                                      if (*(int *)(*(int *)(*(int *)(param_1 + 0x28738) + 0x1f0) +
                                                  local_58 * 4) == 0) {
                                        local_5c = local_5c + 1;
                                      }
                                    }
                                    if (local_5c < 5) {
                                      FUN_00403df2(&DAT_00a3cc78,3000);
                                      (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
                                    }
                                    else if (*(uint *)(DAT_005ccf9c + 0x2a504) < 800) {
                                      FUN_00403df2(&DAT_00a3c8f8,3000);
                                      (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
                                    }
                                    else {
                                      FUN_0055ee59(local_5c);
                                    }
                                  }
                                }
                                else {
                                  DAT_005ccf04 = 2;
                                  if (*(int *)(param_1 + 0x27c10) != 0) {
                                    FUN_00449e3c(*(int *)(*(int *)(param_1 + 0x27c10) + 0x28) == 0);
                                  }
                                }
                              }
                              else {
                                FUN_00449b8e(*(int *)(*(int *)(param_1 + 0x27be8) + 0x28) == 0);
                              }
                            }
                            else {
                              FUN_0044a0fb(*(int *)(*(int *)(param_1 + 0x27c38) + 0x28) == 0);
                            }
                          }
                          else {
                            FUN_004498e0(*(int *)(*(int *)(param_1 + 0x27bb4) + 0x28) == 0);
                          }
                        }
                        else {
                          FUN_00449632(*(int *)(*(int *)(param_1 + 0x27b90) + 0x28) == 0);
                        }
                      }
                      else {
                        DAT_005ccf04 = 1;
                        if (*(int *)(param_1 + 0x27b6c) != 0) {
                          FUN_00449384(*(int *)(*(int *)(param_1 + 0x27b6c) + 0x28) == 0);
                        }
                      }
                    }
                    else {
                      DAT_005ccf04 = 0;
                      if (*(int *)(param_1 + 0x27b6c) != 0) {
                        FUN_00449384(*(int *)(*(int *)(param_1 + 0x27b6c) + 0x28) == 0);
                      }
                    }
                  }
                  else {
                    DAT_005ccf04 = 1;
                    if (*(int *)(param_1 + 0x27c10) != 0) {
                      FUN_00449e3c(*(int *)(*(int *)(param_1 + 0x27c10) + 0x28) == 0);
                    }
                  }
                }
                else {
                  DAT_005ccf04 = 0;
                  if (*(int *)(param_1 + 0x27c10) != 0) {
                    FUN_00449e3c(*(int *)(*(int *)(param_1 + 0x27c10) + 0x28) == 0);
                  }
                }
              }
              else if ((*(short *)(DAT_013b71e8 + 0x798) == 0x6ce) &&
                      (iVar2 = FUN_0040c0f0(), iVar2 == 0)) {
                (**(code **)(**(int **)(param_1 + 0x8c) + 0x8c))(&DAT_00a38d78,0x6ce,0);
                *(undefined4 *)(*(int *)(param_1 + 0x8c) + 0x1e8) = *(undefined4 *)(local_8 + 0x20);
                (**(code **)(**(int **)(param_1 + 0x8c) + 0x60))(1);
                _memset(local_40,0,0x14);
                local_3c = 0x28b;
                local_3a = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
                local_34 = *(undefined4 *)(local_8 + 0x20);
                local_30 = 0;
                FUN_0055f2dd(local_40,0x14);
                *(int *)(param_1 + 0x2747c) = param_7;
              }
              else {
                iVar2 = FUN_0040c0f0();
                if (iVar2 == 0) {
                  local_4c = 0x27b;
                  local_4a = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
                  local_44 = *(undefined2 *)(local_8 + 0x20);
                  local_54 = *(int *)(param_1 + 0x28734);
                  *(undefined4 *)(local_54 + 0x404) = *(undefined4 *)(local_8 + 0x20);
                  FUN_0055f2dd(local_50,0x10);
                  *(int *)(param_1 + 0x2747c) = param_7;
                }
              }
            }
            else {
              iVar2 = FUN_0040c0f0();
              if (iVar2 == 0) {
                FUN_004484f3(1);
                _DAT_005d03e8 = *(undefined4 *)(local_8 + 0x20);
                *(int *)(param_1 + 0x2747c) = param_7;
              }
            }
          }
          else {
            iVar2 = FUN_0040c0f0();
            if (iVar2 == 0) {
              if (((*(float *)(local_8 + 0x28) < _DAT_005a4390) ||
                  (_DAT_005a438c < *(float *)(local_8 + 0x28))) ||
                 ((*(float *)(local_8 + 0x2c) < _DAT_005a438c ||
                  (_DAT_005a4388 < *(float *)(local_8 + 0x2c))))) {
                *(undefined4 *)(param_1 + 0x28ec4) = 0;
              }
              else {
                *(undefined4 *)(param_1 + 0x28ec4) = 1;
              }
              local_24 = 0x27b;
              local_22 = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
              local_1c = *(undefined2 *)(local_8 + 0x20);
              local_2c = *(int *)(param_1 + 0x28730);
              *(undefined4 *)(local_2c + 0x404) = *(undefined4 *)(local_8 + 0x20);
              FUN_0055f2dd(local_28,0x10);
              *(int *)(param_1 + 0x2747c) = param_7;
            }
          }
        }
        else {
          _memset(local_18,0,0x10);
          local_14 = 0x39a;
          local_12 = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
          local_c = *(undefined4 *)(local_8 + 0x20);
          FUN_0055f2dd(local_18,0x10);
          *(int *)(param_1 + 0x2747c) = param_7;
        }
      }
    }
  }
  return 1;
}

