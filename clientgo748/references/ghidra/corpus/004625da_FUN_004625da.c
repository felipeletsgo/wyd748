// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004625da | Name: FUN_004625da


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int __thiscall FUN_004625da(int param_1,int param_2,uint param_3,int param_4,int param_5)

{
  POINT pt;
  bool bVar1;
  int iVar2;
  BOOL BVar3;
  undefined4 uVar4;
  undefined4 *puVar5;
  undefined1 local_f0 [12];
  float local_e4;
  undefined1 local_e0 [4];
  undefined2 local_dc;
  undefined2 local_da;
  short local_d4;
  undefined4 local_d0;
  int local_cc;
  undefined1 local_c8 [12];
  float local_bc;
  int local_b8;
  int local_b4;
  undefined1 local_b0 [128];
  LONG local_30;
  LONG local_2c;
  char local_28;
  int local_24;
  int local_20;
  int local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  uint local_8;
  
  local_8 = (**(code **)(*DAT_0092e654 + 8))();
  if (local_8 < DAT_013b7220 + 6000U) {
    iVar2 = 1;
  }
  else if (local_8 < *(int *)(param_1 + 0x27138) + 6000U) {
    iVar2 = 1;
  }
  else if (local_8 < *(int *)(param_1 + 0x2713c) + 6000U) {
    iVar2 = 1;
  }
  else if (local_8 < *(int *)(param_1 + 0x27140) + 6000U) {
    iVar2 = 1;
  }
  else if (local_8 < *(int *)(param_1 + 0x27148) + 6000U) {
    iVar2 = 1;
  }
  else if (local_8 < *(int *)(param_1 + 0x27144) + 6000U) {
    iVar2 = 1;
  }
  else if (local_8 < *(int *)(param_1 + 0x2714c) + 6000U) {
    iVar2 = 1;
  }
  else if (local_8 < *(int *)(param_1 + 0x27150) + 6000U) {
    iVar2 = 1;
  }
  else if (local_8 < *(int *)(param_1 + 0x2747c) + 1000U) {
    iVar2 = 1;
  }
  else if (*(int *)(param_1 + 0x26e1c) == 1) {
    iVar2 = 0;
  }
  else {
    iVar2 = FUN_0049ab73(param_2,param_3,param_4,param_5);
    if (iVar2 == 1) {
      iVar2 = 1;
    }
    else if (DAT_013b736c == 1) {
      iVar2 = 1;
    }
    else if (*(int *)(param_1 + 0x54) == 1) {
      iVar2 = 1;
    }
    else {
      for (local_1c = 0; local_1c < 8; local_1c = local_1c + 1) {
        if ((*(int *)(*(int *)(param_1 + 0x28) + 0x134 + local_1c * 4) != 0) &&
           (iVar2 = FUN_0040c0f0(), iVar2 != 0)) {
          if (*(int *)(*(int *)(param_1 + 0x28) + 0x134 + local_1c * 4) != *(int *)(param_1 + 0x8c))
          {
            return 1;
          }
          if ((*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) != 0x259) &&
             (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) != 0x39f)) {
            return 1;
          }
        }
      }
      if (DAT_005b892c != 2) {
        iVar2 = FUN_0040c0f0();
        if (iVar2 != 0) {
          return 1;
        }
        iVar2 = FUN_0040c0f0();
        if (iVar2 != 0) {
          return 1;
        }
        iVar2 = FUN_0040c0f0();
        if (iVar2 != 0) {
          return 1;
        }
        if ((*(int *)(param_1 + 0x28c10) != 0) && (iVar2 = FUN_0040c0f0(), iVar2 != 0)) {
          return 1;
        }
      }
      if (((param_4 != *(int *)(param_1 + 0x275d0)) || (param_5 != *(int *)(param_1 + 0x275d4))) ||
         ((*(int *)(DAT_005d0504 + 0x1c) == 0 && (*(int *)(DAT_005d0504 + 0x20) == 0)))) {
        *(int *)(param_1 + 0x275d0) = param_4;
        *(int *)(param_1 + 0x275d4) = param_5;
        *(uint *)(param_1 + 0x275cc) = local_8;
      }
      if ((((*(int *)(*(int *)(param_1 + 0x8c) + 0x28) == 1) &&
           (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 99)) &&
          (0 < *(short *)(DAT_013b71e8 + 0x738))) &&
         (*(char *)(*(int *)(param_1 + 0x4c) + 0x245) == '\0')) {
        (**(code **)(**(int **)(param_1 + 0x8c) + 0x60))(0);
      }
      local_20 = DAT_013b71e8 + 0x6ec;
      if (((*(char *)(param_1 + 0x27458) == '\0') && (*(short *)(DAT_013b71e8 + 0x7b0) != 0x301)) &&
         ((((*(short *)(DAT_013b71e8 + 0x738) < 1 ||
            (*(char *)(*(int *)(param_1 + 0x4c) + 0x245) == '\x01')) &&
           (((param_2 == 0x204 && ((param_3 & 8) != 8)) && (0 < param_4)))) &&
          (((0 < param_5 &&
            (param_4 < *(int *)(DAT_005ccf9c + 0x2a504) - *(int *)(DAT_005ccf9c + 0x2a5e4))) &&
           (param_5 < *(int *)(DAT_005ccf9c + 0x2a508) - *(int *)(DAT_005ccf9c + 0x2a5e8))))))) {
        for (local_24 = 0; local_24 < 0x14; local_24 = local_24 + 1) {
          local_28 = *(char *)(DAT_013b71e8 + local_24 + 0xef6);
          if (local_28 == 'c') {
            *(undefined1 *)(DAT_013b71e8 + 0xef5) = 99;
            FUN_004470b9();
            (**(code **)(**(int **)(param_1 + 0x8c) + 0x8c))(&DAT_00a3b2f8,99,0);
            (**(code **)(**(int **)(param_1 + 0x8c) + 0x60))(1);
            break;
          }
        }
        iVar2 = 1;
      }
      else {
        if (((*(short *)(DAT_013b71e8 + 0x738) < 1) ||
            (*(char *)(*(int *)(param_1 + 0x4c) + 0x245) == '\x01')) && (param_2 == 0x201)) {
          local_30 = __ftol();
          local_2c = __ftol();
          iVar2 = FUN_0040c0f0();
          if ((iVar2 == 0) &&
             ((iVar2 = (**(code **)(**(int **)(param_1 + 0x4c) + 0x50))(), iVar2 == 0 ||
              (pt.y = local_2c, pt.x = local_30, BVar3 = PtInRect((RECT *)&DAT_005b4d40,pt),
              BVar3 == 1)))) {
            if ((local_30 >> 7 == 0x12) && (local_2c >> 7 == 0x1e)) {
              bVar1 = true;
            }
            else {
              bVar1 = false;
            }
            if (!bVar1) {
              (**(code **)(**(int **)(param_1 + 0x8c) + 0x8c))(&DAT_00a34ef8,0xb,0);
              (**(code **)(**(int **)(param_1 + 0x8c) + 0x60))(1);
              return 1;
            }
            if (*(char *)(*(int *)(param_1 + 0x4c) + 0x245) == '\x01') {
              (**(code **)(**(int **)(param_1 + 0x8c) + 0x8c))(&DAT_00a34ef8,0xb,0);
              (**(code **)(**(int **)(param_1 + 0x8c) + 0x60))(1);
              return 1;
            }
          }
        }
        if (*(int *)(*(int *)(param_1 + 0x4c) + 0x740) == 0xc) {
          iVar2 = 1;
        }
        else if (((((param_3 & 4) == 4) && (param_2 == 0x204)) && (*(int *)(param_1 + 0x7c) != 0))
                && (*(uint *)(*(int *)(param_1 + 0x7c) + 0x20) < 1000)) {
          _memset(local_b0,0,0x80);
          FUN_0058f078(local_b0,&DAT_005b7da8,*(int *)(param_1 + 0x7c) + 0x168);
          (**(code **)(**(int **)(param_1 + 0x27b2c) + 0x90))(local_b0);
          *(uint *)(param_1 + 0x27460) = local_8;
          iVar2 = 1;
        }
        else {
          FUN_0049bcb8(&local_18);
          local_c = *(undefined4 *)(param_1 + 0x27b34);
          if ((0x22 < *(short *)(*(int *)(param_1 + 0x4c) + 0x444)) &&
             (*(short *)(*(int *)(param_1 + 0x4c) + 0x444) < 1000)) {
            uVar4 = __ftol();
            uVar4 = __ftol(uVar4);
            iVar2 = FUN_004b3962(uVar4);
            if (iVar2 == 1) {
              FUN_00403df2(&DAT_00a34ff8,1000);
              (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
              return 1;
            }
          }
          if (*(int *)(*(int *)(param_1 + 0x4c) + 0x23c) == 1) {
            iVar2 = 1;
          }
          else {
            if ((*(char *)(*(int *)(param_1 + 0x4c) + 0x246) == '\x01') &&
               (((*(int *)(*(int *)(param_1 + 0x4c) + 0x740) == 0 ||
                 (*(int *)(*(int *)(param_1 + 0x4c) + 0x740) == 1)) ||
                (*(int *)(*(int *)(param_1 + 0x4c) + 0x740) == 0xe)))) {
              *(undefined1 *)(*(int *)(param_1 + 0x4c) + 0x246) = 0;
              FUN_004fa8a1(*(undefined4 *)(param_1 + 0x26e7c));
            }
            if (*(char *)(*(int *)(param_1 + 0x4c) + 0x246) == '\x01') {
              iVar2 = 1;
            }
            else if (*(char *)(*(int *)(param_1 + 0x4c) + 0x774) == '\x01') {
              iVar2 = 1;
            }
            else {
              if (param_2 == 0x200) {
                FUN_004560be(param_4,param_5);
              }
              if (param_2 == 0x201) {
                if (*(char *)(param_1 + 0x275bc) == '\x01') {
                  *(uint *)(param_1 + 0x275c0) = local_8;
                }
                *(undefined1 *)(param_1 + 0x275bc) = 0;
              }
              if (0 < *(short *)(DAT_013b71e8 + 0x738)) {
                local_b8 = *(int *)(DAT_005ccf9c + 0x2a504) - *(int *)(DAT_005ccf9c + 0x2a5e4);
                local_b4 = *(int *)(DAT_005ccf9c + 0x2a508) - *(int *)(DAT_005ccf9c + 0x2a5e8);
                if ((((0 < param_4) && (0 < param_5)) && (param_4 < local_b8)) &&
                   (param_5 < local_b4)) {
                  if (((param_2 == 0x201) && ((param_3 & 8) != 8)) &&
                     ((*(int *)(DAT_013b71e0 + 0x10c) == 0 &&
                      (*(int *)(*(int *)(param_1 + 0x28718) + 0x28) == 1)))) {
                    if ((param_3 & 4) == 4) {
                      FUN_004595ec(param_4,param_5,local_18,local_14,local_10,local_8,0,0);
                    }
                    else {
                      FUN_004595ec(param_4,param_5,local_18,local_14,local_10,local_8,1,0);
                    }
                  }
                  if ((param_2 == 0x205) && ((param_3 & 8) == 8)) {
                    if ((uint)*(ushort *)(param_1 + 0x275ae) ==
                        *(uint *)(*(int *)(param_1 + 0x4c) + 0x20)) {
                      return 1;
                    }
                    if ((*(int *)(param_1 + 0x7c) != 0) &&
                       (*(int *)(*(int *)(param_1 + 0x7c) + 0x5e4) != -0x77ff0100)) {
                      FUN_004561e0(local_8);
                    }
                    return 1;
                  }
                  if ((param_2 == 0x204) && ((param_3 & 8) != 8)) {
                    if ((param_3 & 4) != 4) {
                      iVar2 = FUN_004595ec(param_4,param_5,local_18,local_14,local_10,local_8,1,0);
                      return iVar2;
                    }
                    iVar2 = FUN_004595ec(param_4,param_5,local_18,local_14,local_10,local_8,0,0);
                    return iVar2;
                  }
                  if (param_2 == 0x202) {
                    *(undefined1 *)(param_1 + 0x275bc) = 1;
                    *(undefined4 *)(param_1 + 0x27688) = 0;
                    if (*(char *)(*(int *)(param_1 + 0x4c) + 0x795) == '\x01') {
                      return 1;
                    }
                    if ((uint)*(ushort *)(param_1 + 0x275ae) ==
                        *(uint *)(*(int *)(param_1 + 0x4c) + 0x20)) {
                      return 1;
                    }
                    iVar2 = FUN_0045ee28(param_4,param_5,local_18,local_14,local_10,local_8);
                    return iVar2;
                  }
                  if (((param_2 == 0x201) && (*(char *)(*(int *)(param_1 + 0x4c) + 0x795) == '\0'))
                     && ((*(int *)(param_1 + 0x27688) == 0 || ((param_3 & 4) == 4)))) {
                    local_cc = FUN_0046087b(param_3,local_18,local_14,local_10,local_8);
                    if (local_cc == 0) {
                      local_cc = FUN_0045e762(*(undefined4 *)(param_1 + 0x7c));
                    }
                    puVar5 = (undefined4 *)FUN_00430f20(local_18,local_10);
                    iVar2 = FUN_0049c70f(*puVar5,puVar5[1]);
                    local_bc = (float)iVar2 * _DAT_005a4214;
                    FUN_00430f50(local_18,local_bc + _DAT_005a40ec,local_10);
                    FUN_004310a0(local_c8);
                    FUN_004310a0(local_c8);
                    FUN_004310a0(local_c8);
                    *(float *)(*(int *)(param_1 + 0x26e8c) + 0x78) =
                         *(float *)(*(int *)(param_1 + 0x26e8c) + 0x78) - _DAT_005a4158;
                    if (local_cc != 0) {
                      return local_cc;
                    }
                  }
                  if ((((param_2 == 0x201) && ((param_3 & 4) != 4)) &&
                      (*(int *)(DAT_005ccec0 + 0x1e8) == 0)) && (*(int *)(param_1 + 0x28e58) == 0))
                  {
                    if ((*(int *)(*(int *)(param_1 + 0x4c) + 0x740) == 0x18) ||
                       (*(int *)(*(int *)(param_1 + 0x4c) + 0x740) == 0x1a)) {
                      if (*(int *)(*(int *)(param_1 + 0x4c) + 0x73c) != -1) {
                        return 1;
                      }
                      _memset(local_e0,0,0x14);
                      local_da = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
                      local_dc = 0x36a;
                      if (*(int *)(*(int *)(param_1 + 0x4c) + 0x740) == 0x18) {
                        local_d4 = 0x19;
                      }
                      else {
                        local_d4 = 0x1b;
                      }
                      local_d0 = 0;
                      *(int *)(*(int *)(param_1 + 0x4c) + 0x73c) = (int)local_d4;
                      FUN_0055f2dd(local_e0,0x14);
                      *(uint *)(param_1 + 0x2746c) = local_8;
                    }
                    else {
                      uVar4 = __ftol();
                      iVar2 = FUN_005910c4(uVar4);
                      if (iVar2 < 0xf) {
                        uVar4 = __ftol();
                        iVar2 = FUN_005910c4(uVar4);
                        if (0xf < iVar2) {
                          *(undefined4 *)(param_1 + 0x27688) = 1;
                          puVar5 = (undefined4 *)FUN_00430f20(local_18,local_10);
                          iVar2 = FUN_0049c70f(*puVar5,puVar5[1]);
                          local_e4 = (float)iVar2 * _DAT_005a4214;
                          FUN_00430f50(local_18,local_e4 + _DAT_005a40ec,local_10);
                          FUN_004310a0(local_f0);
                          FUN_004310a0(local_f0);
                          FUN_004310a0(local_f0);
                          *(float *)(*(int *)(param_1 + 0x26e8c) + 0x78) =
                               *(float *)(*(int *)(param_1 + 0x26e8c) + 0x78) - _DAT_005a4158;
                          FUN_00461ed8(local_18,local_14,local_10,local_8);
                        }
                      }
                    }
                  }
                  else if (((param_2 == 0x201) && (iVar2 = FUN_0040c0f0(), iVar2 == 1)) &&
                          (*(int *)(DAT_005ccec0 + 0x1e8) != 0)) {
                    if ((4999 < **(short **)(*(int *)(DAT_005ccec0 + 0x1e8) + 0x670)) &&
                       (**(short **)(*(int *)(DAT_005ccec0 + 0x1e8) + 0x670) < 0x13e8)) {
                      return 1;
                    }
                    FUN_0046247e(local_8);
                    return 1;
                  }
                }
              }
              iVar2 = 0;
            }
          }
        }
      }
    }
  }
  return iVar2;
}

