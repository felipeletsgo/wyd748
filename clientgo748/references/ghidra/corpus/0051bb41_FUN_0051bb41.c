// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0051bb41 | Name: FUN_0051bb41


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0051bb41(int param_1,short param_2,short param_3)

{
  undefined4 uVar1;
  short local_24 [4];
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  short local_c [4];
  
  if (*(int *)(param_1 + 0x214) == 0) {
    *(undefined1 *)(param_1 + 0x7b2) = 0;
    _memset(local_24,0,8);
    _memset(local_c,0,8);
    *(short *)(param_1 + 0x66c) = param_2;
    local_24[0] = param_2;
    *(short *)(param_1 + 0x66e) = param_3;
    local_c[0] = param_3;
    uVar1 = FUN_0054cd07(local_24,0x15);
    *(undefined4 *)(param_1 + 0x160) = uVar1;
    uVar1 = FUN_0054cd07(local_c,0x15);
    *(undefined4 *)(param_1 + 0x164) = uVar1;
    local_18 = FUN_0054cd07(local_24,0x11);
    local_1c = FUN_0054cd07(local_c,0x11);
    if ((((local_c[0] < 0) || (local_24[0] < 0)) || (0x1963 < local_c[0])) || (0x1963 < local_24[0])
       ) {
      FUN_00431159(s_Check_Weapon___L____d_R____d_005bb774,(int)local_24[0],(int)local_c[0]);
    }
    else if ((-1 < *(short *)(&DAT_00d44990 + local_c[0] * 0x8c)) &&
            (-1 < *(short *)(&DAT_00d44990 + local_24[0] * 0x8c))) {
      local_14 = FUN_004b14fd((int)*(short *)(&DAT_00d44990 + local_c[0] * 0x8c),0,180000);
      local_10 = FUN_004b14fd((int)*(short *)(&DAT_00d44990 + local_24[0] * 0x8c),0,180000);
      if ((local_14 == 0) || (local_10 == 0)) {
        FUN_00431159(s_NULL_Mesh_Check_Weapon___L____d_R_005bb794,(int)local_24[0],(int)local_c[0]);
      }
      else {
        *(undefined4 *)(param_1 + 0x678) = *(undefined4 *)(local_14 + 0x354);
        *(undefined4 *)(param_1 + 0x67c) = *(undefined4 *)(local_10 + 0x354);
        *(undefined4 *)(param_1 + 0x670) = 0;
        *(undefined4 *)(param_1 + 0x674) = 0;
        if (*(int *)(param_1 + 0x60) != 0) {
          if (((*(int *)(param_1 + 0x24c) == 0x1a) || (*(int *)(param_1 + 0x24c) == 0x21)) ||
             (*(int *)(param_1 + 0x24c) == 0x28)) {
            FUN_0051bb1a(0);
            if (*(short *)(param_1 + 0x66e) != 0) {
              *(undefined4 *)(param_1 + 0x670) = 1;
            }
            if (*(short *)(param_1 + 0x66c) != 0) {
              *(undefined4 *)(param_1 + 0x674) = 1;
            }
            if (*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) {
              *(undefined4 *)(param_1 + 0x678) = 0x3cf5c28f;
              *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28ec) = 0x3cf5c28f;
              *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28e8) = 0x3cf5c28f;
              *(char *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f6) =
                   (char)*(undefined2 *)(&DAT_00d449da + local_c[0] * 0x8c) * '\r';
              *(char *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f6) =
                   *(char *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f6) +
                   *(char *)(param_1 + 0x1f8);
            }
            if (*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0) {
              *(undefined4 *)(param_1 + 0x67c) = 0x3cf5c28f;
              *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28ec) = 0x3cf5c28f;
              *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28e8) = 0x3cf5c28f;
              *(char *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f6) =
                   (char)*(undefined2 *)(&DAT_00d449da + local_24[0] * 0x8c) * '\r';
              *(char *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f6) =
                   *(char *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f6) +
                   *(char *)(param_1 + 0x1f9);
            }
          }
          else {
            if (*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) {
              *(float *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28ec) =
                   *(float *)(param_1 + 0x678) - _DAT_005a4214;
              *(char *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f6) =
                   (char)*(undefined2 *)(&DAT_00d449da + local_c[0] * 0x8c) * '\r';
              *(char *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f6) =
                   *(char *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f6) +
                   *(char *)(param_1 + 0x1f8);
            }
            if (*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0) {
              *(float *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28ec) =
                   *(float *)(param_1 + 0x67c) - _DAT_005a4214;
              *(char *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f6) =
                   (char)*(undefined2 *)(&DAT_00d449da + local_24[0] * 0x8c) * '\r';
              *(char *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f6) =
                   *(char *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f6) +
                   *(char *)(param_1 + 0x1f9);
            }
            if (*(char *)(param_1 + 0x787) == '\x01') {
              if ((*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) &&
                 (*(short *)(param_1 + 0x66e) != 0)) {
                *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f1) = 1;
              }
              if ((*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0) &&
                 (*(short *)(param_1 + 0x66c) != 0)) {
                *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f1) = 1;
              }
            }
            else {
              if (*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) {
                *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f1) = 0;
              }
              if (*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0) {
                *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f1) = 0;
              }
            }
            if (*(int *)(param_1 + 0x24c) == 0x24) {
              if (((*(int *)(param_1 + 0x160) == 1) || (*(int *)(param_1 + 0x160) == 0x3d)) ||
                 (*(int *)(param_1 + 0x160) == 0x1f)) {
                FUN_0051bb1a(0xb);
              }
              else if (*(int *)(param_1 + 0x160) == 0xb) {
                FUN_0051bb1a(0xf);
              }
              else if (*(int *)(param_1 + 0x160) == 0xd) {
                FUN_0051bb1a(0xc);
              }
              else if (((*(int *)(param_1 + 0x160) == 0x15) || (*(int *)(param_1 + 0x160) == 0x16))
                      || (*(int *)(param_1 + 0x160) == 0x17)) {
                FUN_0051bb1a(0xd);
              }
              else if ((*(int *)(param_1 + 0x160) == 3) || (*(int *)(param_1 + 0x160) == 0x3f)) {
                FUN_0051bb1a(0xe);
              }
              *(undefined4 *)(param_1 + 0x674) = 1;
            }
            else if (*(int *)(param_1 + 0x24c) == 0x25) {
              FUN_0051bb1a(0xb);
              *(undefined4 *)(param_1 + 0x674) = 1;
            }
            else if (*(int *)(param_1 + 0x24c) == 0x3c) {
              if ((*(int *)(param_1 + 0x160) == 0x29) && (local_1c == 0)) {
                *(undefined1 *)(*(int *)(param_1 + 0x60) + 0x305) = 1;
                FUN_0051bb1a(0);
                *(undefined4 *)(param_1 + 0x670) = 1;
              }
              else if ((*(int *)(param_1 + 0x160) == 0xb) && (local_1c == 0x80)) {
                FUN_0051bb1a(1);
                *(undefined4 *)(param_1 + 0x670) = 1;
              }
              else if ((*(int *)(param_1 + 0x160) == 0xb) && (local_1c == 0xc0)) {
                FUN_0051bb1a(4);
                *(undefined4 *)(param_1 + 0x670) = 1;
                *(undefined4 *)(param_1 + 0x674) = 1;
              }
              else {
                *(undefined4 *)(param_1 + 0x674) = 1;
                *(undefined4 *)(param_1 + 0x670) = 1;
                FUN_0051bb1a(0);
              }
            }
            else if (*(int *)(param_1 + 0x24c) == 0x3d) {
              FUN_0051bb1a(1);
              *(undefined4 *)(param_1 + 0x674) = 1;
            }
            else if (*(int *)(param_1 + 0x24c) == 0x3e) {
              if ((((*(int *)(param_1 + 0x160) == 1) || (*(int *)(param_1 + 0x160) == 0xb)) ||
                  (*(int *)(param_1 + 0x160) == 0x3d)) ||
                 (((*(int *)(param_1 + 0x160) == 2 || (*(int *)(param_1 + 0x160) == 0xc)) ||
                  ((*(int *)(param_1 + 0x160) == 0x3e || (*(int *)(param_1 + 0x160) == 0x1f)))))) {
                FUN_0051bb1a(1);
                *(undefined4 *)(param_1 + 0x674) = 1;
                *(undefined4 *)(param_1 + 0x670) = 1;
              }
              else {
                FUN_0051bb1a(0);
              }
            }
            else {
              if (local_1c == 0x80) {
                *(undefined1 *)(param_1 + 0x7b2) = 1;
              }
              if (*(int *)(param_1 + 0x5c) == 0) {
                if (*(char *)(param_1 + 0x79c) == '\0') {
                  if ((*(int *)(param_1 + 0x160) == 0) && (local_1c == 0x80)) {
                    FUN_0051bb1a(2);
                  }
                  else if ((local_18 == 0xc0) && (local_1c == 0xc0)) {
                    FUN_0051bb1a(4);
                    *(undefined4 *)(param_1 + 0x670) = 1;
                    *(undefined4 *)(param_1 + 0x674) = 1;
                  }
                  else if ((((*(int *)(param_1 + 0x160) == 1) || (*(int *)(param_1 + 0x160) == 0xb))
                           || (*(int *)(param_1 + 0x160) == 0x3d)) ||
                          (*(int *)(param_1 + 0x160) == 0x1f)) {
                    if (*(int *)(param_1 + 0x164) == 0) {
                      FUN_0051bb1a(1);
                      *(undefined4 *)(param_1 + 0x674) = 1;
                    }
                    else if (local_1c == 0x80) {
                      FUN_0051bb1a(1);
                      *(undefined4 *)(param_1 + 0x674) = 1;
                    }
                    else if (local_1c == 0xc4) {
                      FUN_0051bb1a(4);
                      *(undefined4 *)(param_1 + 0x670) = 1;
                      *(undefined4 *)(param_1 + 0x674) = 1;
                    }
                  }
                  else if (((*(int *)(param_1 + 0x160) == 2) || (*(int *)(param_1 + 0x160) == 0xc))
                          || (*(int *)(param_1 + 0x160) == 0x3e)) {
                    if (local_1c == 0x80) {
                      FUN_0051bb1a(3);
                      *(undefined4 *)(param_1 + 0x674) = 1;
                    }
                    else if (*(int *)(param_1 + 0x164) == 0) {
                      FUN_0051bb1a(5);
                      *(undefined4 *)(param_1 + 0x674) = 1;
                    }
                    else if (local_1c == 0xc4) {
                      FUN_0051bb1a(4);
                      *(undefined4 *)(param_1 + 0x670) = 1;
                      *(undefined4 *)(param_1 + 0x674) = 1;
                    }
                  }
                  else if ((*(int *)(param_1 + 0x160) == 3) || (*(int *)(param_1 + 0x160) == 0x3f))
                  {
                    FUN_0051bb1a(6);
                    *(undefined4 *)(param_1 + 0x674) = 1;
                  }
                  else if (*(int *)(param_1 + 0x160) == 0xd) {
                    if (local_18 == 0x40) {
                      FUN_0051bb1a(7);
                      *(undefined4 *)(param_1 + 0x674) = 1;
                    }
                  }
                  else if (((*(int *)(param_1 + 0x160) == 0x15) ||
                           (*(int *)(param_1 + 0x160) == 0x16)) ||
                          (*(int *)(param_1 + 0x160) == 0x17)) {
                    if (local_18 == 0x40) {
                      FUN_0051bb1a(8);
                      *(undefined4 *)(param_1 + 0x674) = 1;
                    }
                  }
                  else if ((*(int *)(param_1 + 0x160) == 0x66) ||
                          (*(int *)(param_1 + 0x160) == 0x67)) {
                    if (local_18 == 0x40) {
                      FUN_0051bb1a(10);
                      *(undefined4 *)(param_1 + 0x674) = 1;
                    }
                  }
                  else if (*(int *)(param_1 + 0x160) == 0x68) {
                    if (local_18 == 0x40) {
                      FUN_0051bb1a(9);
                      *(undefined4 *)(param_1 + 0x674) = 1;
                    }
                  }
                  else if (*(int *)(param_1 + 0x160) == 0x65) {
                    FUN_0051bb1a(3);
                    *(undefined4 *)(param_1 + 0x674) = 1;
                  }
                  else if ((*(int *)(param_1 + 0x160) == 0x20) ||
                          (*(int *)(param_1 + 0x160) == 0x21)) {
                    FUN_0051bb1a(5);
                    *(undefined4 *)(param_1 + 0x674) = 1;
                  }
                  else if (*(int *)(param_1 + 0x160) == 0x29) {
                    *(undefined4 *)(param_1 + 0x674) = 1;
                    *(undefined4 *)(param_1 + 0x670) = 1;
                    FUN_0051bb1a(0);
                  }
                  else {
                    FUN_0051bb1a(0);
                  }
                }
                else if ((*(int *)(param_1 + 0x160) == 0) && (local_1c == 0x80)) {
                  FUN_0051bb1a(3);
                }
                else if ((local_18 == 0xc0) && (local_1c == 0xc0)) {
                  FUN_0051bb1a(2);
                  *(undefined4 *)(param_1 + 0x670) = 1;
                  *(undefined4 *)(param_1 + 0x674) = 1;
                }
                else if (((((((*(int *)(param_1 + 0x160) == 1) || (*(int *)(param_1 + 0x160) == 0xb)
                             ) || (*(int *)(param_1 + 0x160) == 0x3d)) ||
                           ((*(int *)(param_1 + 0x160) == 2 || (*(int *)(param_1 + 0x160) == 0xc))))
                          || (*(int *)(param_1 + 0x160) == 0x3e)) ||
                         (((*(int *)(param_1 + 0x160) == 2 || (*(int *)(param_1 + 0x160) == 0xc)) ||
                          (*(int *)(param_1 + 0x160) == 0x3e)))) ||
                        (*(int *)(param_1 + 0x160) == 0x1f)) {
                  FUN_0051bb1a(1);
                  if (*(int *)(param_1 + 0x164) == 0) {
                    *(undefined4 *)(param_1 + 0x674) = 1;
                  }
                  else if (local_1c == 0x80) {
                    *(undefined4 *)(param_1 + 0x674) = 1;
                  }
                }
                else if (((((*(int *)(param_1 + 0x160) == 0x15) ||
                           (*(int *)(param_1 + 0x160) == 0x16)) ||
                          (*(int *)(param_1 + 0x160) == 0x17)) ||
                         ((*(int *)(param_1 + 0x160) == 0xd || (*(int *)(param_1 + 0x160) == 3))))
                        || ((*(int *)(param_1 + 0x160) == 0x3f ||
                            ((*(int *)(param_1 + 0x160) == 0x20 ||
                             (*(int *)(param_1 + 0x160) == 0x21)))))) {
                  if (local_18 == 0x40) {
                    FUN_0051bb1a(4);
                    *(undefined4 *)(param_1 + 0x674) = 1;
                  }
                }
                else if (*(int *)(param_1 + 0x160) == 0x65) {
                  FUN_0051bb1a(5);
                  *(undefined4 *)(param_1 + 0x674) = 1;
                }
                else if ((*(int *)(param_1 + 0x160) == 0x66) || (*(int *)(param_1 + 0x160) == 0x67))
                {
                  if (local_18 == 0x40) {
                    FUN_0051bb1a(1);
                    *(undefined4 *)(param_1 + 0x674) = 1;
                  }
                }
                else if (*(int *)(param_1 + 0x160) == 0x29) {
                  *(undefined4 *)(param_1 + 0x674) = 1;
                  *(undefined4 *)(param_1 + 0x670) = 1;
                  FUN_0051bb1a(0);
                }
                else {
                  FUN_0051bb1a(0);
                }
                if (*(int *)(param_1 + 0x160) == 0x29) {
                  *(undefined4 *)(param_1 + 0x674) = 1;
                  *(undefined4 *)(param_1 + 0x670) = 1;
                }
              }
              else if (*(int *)(param_1 + 0x5c) == 1) {
                if (*(char *)(param_1 + 0x79c) == '\0') {
                  if ((*(int *)(param_1 + 0x160) == 0) && (local_1c == 0x80)) {
                    FUN_0051bb1a(2);
                  }
                  else if ((local_18 == 0xc0) && (local_1c == 0xc0)) {
                    FUN_0051bb1a(4);
                    *(undefined4 *)(param_1 + 0x670) = 1;
                    *(undefined4 *)(param_1 + 0x674) = 1;
                  }
                  else if ((*(int *)(param_1 + 0x160) == 3) || (*(int *)(param_1 + 0x160) == 0x3f))
                  {
                    FUN_0051bb1a(10);
                    *(undefined4 *)(param_1 + 0x674) = 1;
                  }
                  else if ((((*(int *)(param_1 + 0x160) == 1) || (*(int *)(param_1 + 0x160) == 0xb))
                           || (*(int *)(param_1 + 0x160) == 0x3d)) ||
                          (((*(int *)(param_1 + 0x160) == 2 || (*(int *)(param_1 + 0x160) == 0xc))
                           || ((*(int *)(param_1 + 0x160) == 0x3e ||
                               (*(int *)(param_1 + 0x160) == 0x1f)))))) {
                    if (*(int *)(param_1 + 0x164) == 0) {
                      FUN_0051bb1a(1);
                      *(undefined4 *)(param_1 + 0x674) = 1;
                    }
                    else if (local_1c == 0x80) {
                      FUN_0051bb1a(3);
                      *(undefined4 *)(param_1 + 0x674) = 1;
                    }
                    else if (local_1c == 0xc4) {
                      FUN_0051bb1a(4);
                      *(undefined4 *)(param_1 + 0x670) = 1;
                      *(undefined4 *)(param_1 + 0x674) = 1;
                    }
                  }
                  else if (((*(int *)(param_1 + 0x160) == 0x15) ||
                           (*(int *)(param_1 + 0x160) == 0x16)) ||
                          (*(int *)(param_1 + 0x160) == 0x17)) {
                    FUN_0051bb1a(5);
                    *(undefined4 *)(param_1 + 0x674) = 1;
                  }
                  else if ((*(int *)(param_1 + 0x160) == 0x66) ||
                          (*(int *)(param_1 + 0x160) == 0x67)) {
                    if (local_18 == 0x40) {
                      FUN_0051bb1a(3);
                      *(undefined4 *)(param_1 + 0x674) = 1;
                    }
                  }
                  else if (*(int *)(param_1 + 0x160) == 0x1f) {
                    FUN_0051bb1a(3);
                    *(undefined4 *)(param_1 + 0x674) = 1;
                  }
                  else if (*(int *)(param_1 + 0x160) == 0xd) {
                    FUN_0051bb1a(7);
                    *(undefined4 *)(param_1 + 0x674) = 1;
                  }
                  else if ((*(int *)(param_1 + 0x160) == 0x20) ||
                          (*(int *)(param_1 + 0x160) == 0x21)) {
                    if (local_18 == 0x40) {
                      FUN_0051bb1a(9);
                      *(undefined4 *)(param_1 + 0x674) = 1;
                    }
                  }
                  else if (*(int *)(param_1 + 0x160) == 0x65) {
                    if (local_18 == 0x40) {
                      FUN_0051bb1a(6);
                      *(undefined4 *)(param_1 + 0x674) = 1;
                    }
                  }
                  else if (*(int *)(param_1 + 0x160) == 0x29) {
                    *(undefined4 *)(param_1 + 0x674) = 1;
                    *(undefined4 *)(param_1 + 0x670) = 1;
                    FUN_0051bb1a(0);
                  }
                  else {
                    FUN_0051bb1a(0);
                  }
                }
                else if ((*(int *)(param_1 + 0x160) == 0) && (local_1c == 0x80)) {
                  FUN_0051bb1a(3);
                }
                else if ((local_18 == 0xc0) && (local_1c == 0xc0)) {
                  FUN_0051bb1a(2);
                  *(undefined4 *)(param_1 + 0x670) = 1;
                  *(undefined4 *)(param_1 + 0x674) = 1;
                }
                else if (((((*(int *)(param_1 + 0x160) == 1) || (*(int *)(param_1 + 0x160) == 0xb))
                          || (*(int *)(param_1 + 0x160) == 0x3d)) ||
                         (((*(int *)(param_1 + 0x160) == 2 || (*(int *)(param_1 + 0x160) == 0xc)) ||
                          (*(int *)(param_1 + 0x160) == 0x3e)))) ||
                        (*(int *)(param_1 + 0x160) == 0x1f)) {
                  FUN_0051bb1a(1);
                  if (*(int *)(param_1 + 0x164) == 0) {
                    *(undefined4 *)(param_1 + 0x674) = 1;
                  }
                  else if (local_1c == 0x80) {
                    *(undefined4 *)(param_1 + 0x674) = 1;
                  }
                }
                else if (((*(int *)(param_1 + 0x160) == 0x15) || (*(int *)(param_1 + 0x160) == 0x16)
                         ) || ((*(int *)(param_1 + 0x160) == 0x17 ||
                               ((((*(int *)(param_1 + 0x160) == 0xd ||
                                  (*(int *)(param_1 + 0x160) == 3)) ||
                                 (*(int *)(param_1 + 0x160) == 0x3f)) ||
                                ((*(int *)(param_1 + 0x160) == 0x20 ||
                                 (*(int *)(param_1 + 0x160) == 0x21)))))))) {
                  if (local_18 == 0x40) {
                    FUN_0051bb1a(4);
                    *(undefined4 *)(param_1 + 0x674) = 1;
                  }
                }
                else if (*(int *)(param_1 + 0x160) == 0x65) {
                  FUN_0051bb1a(5);
                  *(undefined4 *)(param_1 + 0x674) = 1;
                }
                else if ((*(int *)(param_1 + 0x160) == 0x66) || (*(int *)(param_1 + 0x160) == 0x67))
                {
                  if (local_18 == 0x40) {
                    FUN_0051bb1a(1);
                    *(undefined4 *)(param_1 + 0x674) = 1;
                  }
                }
                else if (*(int *)(param_1 + 0x160) == 0x29) {
                  *(undefined4 *)(param_1 + 0x674) = 1;
                  *(undefined4 *)(param_1 + 0x670) = 1;
                  FUN_0051bb1a(0);
                }
                else {
                  FUN_0051bb1a(0);
                }
              }
              else if (*(int *)(param_1 + 0x5c) == 2) {
                if (*(char *)(param_1 + 0x79c) == '\0') {
                  if (*(int *)(param_1 + 0x160) == 0x65) {
                    FUN_0051bb1a(1);
                    *(undefined4 *)(param_1 + 0x674) = 0;
                  }
                  else if (*(int *)(param_1 + 0x160) == 0xc) {
                    FUN_0051bb1a(2);
                    *(undefined4 *)(param_1 + 0x674) = 1;
                  }
                  else if (((local_1c == 0x80) || (*(int *)(param_1 + 0x160) == 1)) ||
                          (*(int *)(param_1 + 0x160) == 0xb)) {
                    FUN_0051bb1a(5);
                    *(undefined4 *)(param_1 + 0x674) = 1;
                  }
                  else if ((((*(int *)(param_1 + 0x160) == 0xb) ||
                            (*(int *)(param_1 + 0x160) == 0xc)) ||
                           (*(int *)(param_1 + 0x160) == 0xd)) ||
                          (*(int *)(param_1 + 0x160) == 0x15)) {
                    FUN_0051bb1a(3);
                    *(undefined4 *)(param_1 + 0x674) = 1;
                  }
                  else if (((*(int *)(param_1 + 0x160) == 0x1f) ||
                           (*(int *)(param_1 + 0x160) == 0x20)) ||
                          (*(int *)(param_1 + 0x160) == 0x21)) {
                    FUN_0051bb1a(4);
                    *(undefined4 *)(param_1 + 0x674) = 1;
                  }
                  else {
                    FUN_0051bb1a(0);
                    if (local_18 == 0) {
                      *(undefined4 *)(param_1 + 0x674) = 0;
                    }
                    else {
                      *(undefined4 *)(param_1 + 0x674) = 1;
                    }
                  }
                }
                else if (*(int *)(param_1 + 0x160) == 0x65) {
                  FUN_0051bb1a(1);
                  *(undefined4 *)(param_1 + 0x674) = 0;
                }
                else if ((((*(int *)(param_1 + 0x160) == 1) || (*(int *)(param_1 + 0x160) == 0xb))
                         || (*(int *)(param_1 + 0x160) == 0xc)) ||
                        (((*(int *)(param_1 + 0x160) == 0xd || (*(int *)(param_1 + 0x160) == 0x1f))
                         || ((*(int *)(param_1 + 0x160) == 0x20 ||
                             (*(int *)(param_1 + 0x160) == 0x21)))))) {
                  FUN_0051bb1a(5);
                  *(undefined4 *)(param_1 + 0x674) = 1;
                }
                else {
                  FUN_0051bb1a(0);
                  if (local_18 == 0) {
                    *(undefined4 *)(param_1 + 0x674) = 0;
                  }
                  else {
                    *(undefined4 *)(param_1 + 0x674) = 1;
                  }
                }
              }
              else if (*(int *)(param_1 + 0x5c) == 3) {
                if (*(char *)(param_1 + 0x79c) == '\0') {
                  if (*(short *)(param_1 + 0x1d2) == 0) {
                    FUN_0051bb1a(0);
                  }
                  else if (*(short *)(param_1 + 0x1d2) == 1) {
                    FUN_0051bb1a(1);
                  }
                }
                else {
                  FUN_0051bb1a(0);
                }
              }
              else if (*(int *)(param_1 + 0x5c) == 4) {
                if (((*(int *)(param_1 + 0x160) == 1) || (*(int *)(param_1 + 0x160) == 0xb)) ||
                   (*(int *)(param_1 + 0x160) == 0x3d)) {
                  FUN_0051bb1a(1);
                  *(undefined4 *)(param_1 + 0x674) = 1;
                }
                else if (((*(int *)(param_1 + 0x160) == 0x15) || (*(int *)(param_1 + 0x160) == 0x16)
                         ) || ((*(int *)(param_1 + 0x160) == 0x17 ||
                               (*(int *)(param_1 + 0x160) == 0xd)))) {
                  FUN_0051bb1a(2);
                  *(undefined4 *)(param_1 + 0x674) = 1;
                }
                else if ((*(int *)(param_1 + 0x160) == 0x66) || (*(int *)(param_1 + 0x160) == 0x67))
                {
                  FUN_0051bb1a(3);
                  *(undefined4 *)(param_1 + 0x674) = 0;
                }
                else {
                  FUN_0051bb1a(0);
                  if (local_18 == 0) {
                    *(undefined4 *)(param_1 + 0x674) = 0;
                  }
                  else {
                    *(undefined4 *)(param_1 + 0x674) = 1;
                  }
                }
              }
              if ((*(int *)(param_1 + 0x160) == 0x29) && (*(int *)(param_1 + 0x60) != 0)) {
                *(undefined1 *)(*(int *)(param_1 + 0x60) + 0x305) = 1;
              }
              else if (*(int *)(param_1 + 0x60) != 0) {
                *(undefined1 *)(*(int *)(param_1 + 0x60) + 0x305) = 0;
              }
              if (*(int *)(param_1 + 0x5c) == 0xb) {
                *(undefined1 *)(*(int *)(param_1 + 0x60) + 0x305) = 1;
              }
              else if (*(int *)(param_1 + 0x5c) == 10) {
                *(undefined1 *)(*(int *)(param_1 + 0x60) + 0x304) = 1;
                FUN_0051bb1a(0);
                if (*(short *)(param_1 + 0x66e) != 0) {
                  *(undefined4 *)(param_1 + 0x670) = 1;
                }
                if (*(short *)(param_1 + 0x66c) != 0) {
                  *(undefined4 *)(param_1 + 0x674) = 1;
                }
              }
              if (*(short *)(param_1 + 0x7ae) == 0x103a) {
                FUN_0051bb1a(0xb);
              }
              if (*(int *)(param_1 + 0x740) < 0xe) {
                FUN_0051dba4(1,1);
              }
            }
          }
        }
      }
    }
  }
  return;
}

