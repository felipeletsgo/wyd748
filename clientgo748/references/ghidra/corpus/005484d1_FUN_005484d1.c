// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005484d1 | Name: FUN_005484d1


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_005484d1(int param_1,undefined4 param_2)

{
  bool bVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  undefined1 local_a4 [12];
  float local_98;
  float local_94;
  float local_90;
  uint local_8c;
  int local_88;
  int local_84;
  int local_80;
  int local_7c;
  int local_78;
  int local_74;
  int local_70;
  int local_6c;
  int local_68;
  uint local_64;
  uint local_60;
  float local_5c;
  uint local_58;
  uint local_54;
  uint local_50;
  uint local_4c;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  int local_30;
  uint local_2c;
  int local_28;
  undefined1 local_24 [16];
  uint local_14;
  int local_c;
  undefined *local_8;
  
  if (DAT_005d03f0 == 0) {
    local_30 = FUN_004b14fd(*(undefined4 *)(param_1 + 0x24),1,180000);
    if (*(int *)(DAT_0067cf38 + 0x48) != 0) {
      if ((*(int *)(param_1 + 0x164) == 0) || (9 < *(int *)(param_1 + 0x164))) {
        *(undefined4 *)(*(int *)(DAT_0067cf38 + 0x48) + 0x164) = 0;
      }
      else {
        *(undefined4 *)(*(int *)(DAT_0067cf38 + 0x48) + 0x164) = 1;
      }
      if ((*(int *)(param_1 + 0x160) == 0x44) || (*(int *)(local_30 + 0x298) == 0x44)) {
        *(undefined4 *)(*(int *)(DAT_0067cf38 + 0x48) + 0x164) = 1;
      }
    }
    if (local_30 == 0) {
      uVar2 = 0;
    }
    else {
      (**(code **)(**(int **)(local_30 + 0x318) + 0x34))(*(undefined4 *)(local_30 + 0x318),local_24)
      ;
      (**(code **)(**(int **)(local_30 + 0x318) + 0x2c))
                (*(undefined4 *)(local_30 + 0x318),0,0,&local_c,0);
      local_2c = local_14 / 0x18;
      DAT_005b48e8 = 0x428c0000;
      local_8 = &DAT_00808080;
      if (DAT_005ccfa0 == 1) {
        iVar3 = *(int *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x54);
        local_38 = *(undefined4 *)(iVar3 + 0x28);
        local_34 = *(undefined4 *)(iVar3 + 0x2c);
        iVar3 = __ftol();
        if (((iVar3 >> 7 < 0x11) || (iVar3 = __ftol(), 0x13 < iVar3 >> 7)) ||
           (iVar3 = __ftol(), iVar3 >> 7 < 0x1e)) {
          bVar1 = false;
        }
        else {
          bVar1 = true;
        }
        if (bVar1) {
          *(undefined4 *)(DAT_005ccf9c + 0x2a6e4) = 0x3f800000;
          *(undefined4 *)(DAT_005ccf9c + 0x2a6e8) = 0x3f4ccccd;
          *(undefined4 *)(DAT_005ccf9c + 0x2a6ec) = 0x3f4ccccd;
          iVar3 = __ftol();
          if ((iVar3 >> 7 == 0x12) && (iVar3 = __ftol(), iVar3 >> 7 == 0x1e)) {
            *(undefined4 *)(DAT_005ccf9c + 0x2b044) = DAT_005be080;
            *(undefined4 *)(DAT_005ccf9c + 0x2b048) = DAT_005be084;
          }
          else {
            *(undefined4 *)(DAT_005ccf9c + 0x2b044) = DAT_005be078;
            *(undefined4 *)(DAT_005ccf9c + 0x2b048) = DAT_005be07c;
          }
          *(undefined4 *)(DAT_005ccf9c + 0x2a6f4) = 0x3e99999a;
          *(undefined4 *)(DAT_005ccf9c + 0x2a6f8) = 0;
          *(undefined4 *)(DAT_005ccf9c + 0x2a6fc) = 0;
          *(undefined4 *)(DAT_005ccf9c + 0x2a6f0) = 0x3f800000;
          *(undefined4 *)(DAT_005ccf9c + 0x2b034) = 0x160000;
        }
        else {
          iVar3 = __ftol();
          if ((iVar3 >> 7 == 0x1f) && (iVar3 = __ftol(), iVar3 >> 7 == 0x1f)) {
            bVar1 = true;
          }
          else {
            bVar1 = false;
          }
          if (bVar1) {
            *(undefined4 *)(DAT_005ccf9c + 0x2a6e4) = 0x3f800000;
            *(undefined4 *)(DAT_005ccf9c + 0x2a6e8) = 0x3f800000;
            *(undefined4 *)(DAT_005ccf9c + 0x2a6ec) = 0x3f800000;
            *(undefined4 *)(DAT_005ccf9c + 0x2b044) = DAT_005be088;
            *(undefined4 *)(DAT_005ccf9c + 0x2b048) = DAT_005be08c;
            *(undefined4 *)(DAT_005ccf9c + 0x2a6f4) = 0x3f19999a;
            *(undefined4 *)(DAT_005ccf9c + 0x2a6f8) = 0x3f19999a;
            *(undefined4 *)(DAT_005ccf9c + 0x2a6fc) = 0x3f19999a;
            *(undefined4 *)(DAT_005ccf9c + 0x2a6f0) = 0x3f800000;
          }
          else {
            iVar3 = __ftol();
            if ((((iVar3 >> 7 < 0x1c) || (iVar3 = __ftol(), 0x1e < iVar3 >> 7)) ||
                (iVar3 = __ftol(), iVar3 >> 7 < 0x1b)) || (iVar3 = __ftol(), 0x1c < iVar3 >> 7)) {
              bVar1 = false;
            }
            else {
              bVar1 = true;
            }
            if (bVar1) {
              *(undefined4 *)(DAT_005ccf9c + 0x2a6e4) = 0x3f666666;
              *(undefined4 *)(DAT_005ccf9c + 0x2a6e8) = 0x3f666666;
              *(undefined4 *)(DAT_005ccf9c + 0x2a6ec) = 0x3f666666;
              *(undefined4 *)(DAT_005ccf9c + 0x2b044) = DAT_005be090;
              *(undefined4 *)(DAT_005ccf9c + 0x2b048) = DAT_005be094;
              *(undefined4 *)(DAT_005ccf9c + 0x2a6f4) = 0;
              *(undefined4 *)(DAT_005ccf9c + 0x2a6f8) = 0;
              *(undefined4 *)(DAT_005ccf9c + 0x2a6fc) = 0;
              *(undefined4 *)(DAT_005ccf9c + 0x2a6f0) = 0x3f800000;
              *(undefined4 *)(DAT_005ccf9c + 0x2b034) = 0;
            }
            else {
              *(undefined4 *)(DAT_005ccf9c + 0x2a6e4) = 0x3f800000;
              *(undefined4 *)(DAT_005ccf9c + 0x2a6e8) = 0x3f800000;
              *(undefined4 *)(DAT_005ccf9c + 0x2a6ec) = 0x3f800000;
              *(undefined4 *)(DAT_005ccf9c + 0x2a6f0) = 0x3f800000;
              *(undefined4 *)(DAT_005ccf9c + 0x2a6f4) = 0x3ecccccd;
              *(undefined4 *)(DAT_005ccf9c + 0x2a6f8) = 0x3ecccccd;
              *(undefined4 *)(DAT_005ccf9c + 0x2a6fc) = 0x3ecccccd;
              *(undefined4 *)(DAT_005ccf9c + 0x2b034) = 0;
              *(undefined4 *)(DAT_005ccf9c + 0x2b044) = DAT_005be098;
              *(undefined4 *)(DAT_005ccf9c + 0x2b048) = DAT_005be09c;
            }
          }
        }
        *(undefined4 *)(*(int *)(DAT_0067cf38 + 0x48) + 0x164) = 1;
      }
      else if (DAT_005ccfa0 == 2) {
        *(undefined4 *)(DAT_005ccf9c + 0x2b034) = 0;
        *(undefined4 *)(DAT_005ccf9c + 0x2b044) = DAT_005be0a0;
        *(undefined4 *)(DAT_005ccf9c + 0x2b048) = DAT_005be0a4;
        *(undefined4 *)(*(int *)(DAT_0067cf38 + 0x48) + 0x164) = 1;
        iVar3 = *(int *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x54);
        local_40 = *(undefined4 *)(iVar3 + 0x28);
        local_3c = *(undefined4 *)(iVar3 + 0x2c);
        iVar3 = __ftol();
        if (((iVar3 >> 7 == 0xd) || (iVar3 = __ftol(), iVar3 >> 7 == 0xe)) &&
           (iVar3 = __ftol(), iVar3 >> 7 == 0x1c)) {
          bVar1 = true;
        }
        else {
          bVar1 = false;
        }
        if (bVar1) {
          *(undefined4 *)(DAT_005ccf9c + 0x2a6e4) = 0x3f4ccccd;
          *(undefined4 *)(DAT_005ccf9c + 0x2a6e8) = 0x3f4ccccd;
          *(undefined4 *)(DAT_005ccf9c + 0x2a6ec) = 0x3f4ccccd;
          *(undefined4 *)(DAT_005ccf9c + 0x2b044) = DAT_005be0a8;
          *(undefined4 *)(DAT_005ccf9c + 0x2b048) = DAT_005be0ac;
          *(undefined4 *)(DAT_005ccf9c + 0x2a6f4) = 0x3f19999a;
          *(undefined4 *)(DAT_005ccf9c + 0x2a6f8) = 0x3f19999a;
          *(undefined4 *)(DAT_005ccf9c + 0x2a6fc) = 0x3f19999a;
          *(undefined4 *)(DAT_005ccf9c + 0x2a6f0) = 0x3f800000;
        }
        else {
          *(undefined4 *)(DAT_005ccf9c + 0x2a6e4) = 0x3f800000;
          *(undefined4 *)(DAT_005ccf9c + 0x2a6e8) = 0x3f800000;
          *(undefined4 *)(DAT_005ccf9c + 0x2a6ec) = 0x3f800000;
          *(undefined4 *)(DAT_005ccf9c + 0x2a6f0) = 0x3f800000;
          *(undefined4 *)(DAT_005ccf9c + 0x2a6f4) = 0x3e4ccccd;
          *(undefined4 *)(DAT_005ccf9c + 0x2a6f8) = 0x3e99999a;
          *(undefined4 *)(DAT_005ccf9c + 0x2a6fc) = 0x3e4ccccd;
        }
      }
      else if (DAT_005ccfa0 == 5) {
        *(undefined4 *)(DAT_005ccf9c + 0x2a6e4) = 0x3f800000;
        *(undefined4 *)(DAT_005ccf9c + 0x2a6e8) = 0x3f800000;
        *(undefined4 *)(DAT_005ccf9c + 0x2a6ec) = 0x3f800000;
        *(undefined4 *)(DAT_005ccf9c + 0x2a6f0) = 0x3f800000;
        *(undefined4 *)(DAT_005ccf9c + 0x2a6f4) = 0x3ecccccd;
        *(undefined4 *)(DAT_005ccf9c + 0x2a6f8) = 0x3ecccccd;
        *(undefined4 *)(DAT_005ccf9c + 0x2a6fc) = 0x3ecccccd;
        *(undefined4 *)(DAT_005ccf9c + 0x2b034) = 0;
        *(undefined4 *)(DAT_005ccf9c + 0x2b044) = DAT_005be098;
        *(undefined4 *)(DAT_005ccf9c + 0x2b048) = DAT_005be09c;
      }
      else {
        *(undefined4 *)(DAT_005ccf9c + 0x2a6f4) = 0x3ecccccd;
        *(undefined4 *)(DAT_005ccf9c + 0x2a6f8) = 0x3ecccccd;
        *(undefined4 *)(DAT_005ccf9c + 0x2a6fc) = 0x3ecccccd;
        if (*(int *)(DAT_0067cf38 + 0x24) == 0x7534) {
          *(undefined4 *)(DAT_005ccf9c + 0x2b044) = DAT_005be0c0;
          *(undefined4 *)(DAT_005ccf9c + 0x2b048) = DAT_005be0c4;
        }
        if (*(int *)(DAT_0067cf38 + 0x24) == 0x7535) {
          *(undefined4 *)(DAT_005ccf9c + 0x2b044) = DAT_005be0c8;
          *(undefined4 *)(DAT_005ccf9c + 0x2b048) = DAT_005be0cc;
        }
        else if (*(int *)(DAT_0067cf38 + 0x24) == 0x7531) {
          *(undefined4 *)(DAT_005ccf9c + 0x2b044) = DAT_005be0d0;
          *(undefined4 *)(DAT_005ccf9c + 0x2b048) = DAT_005be0d4;
          *(undefined4 *)(DAT_005ccf9c + 0x2b038) = 0;
        }
        else {
          FUN_00430f20(0,0);
          if (*(int *)(DAT_0067cf38 + 0x24) == 30000) {
            FUN_0040bd30(*(int *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x54) + 0x28);
          }
          iVar3 = __ftol();
          if ((((iVar3 >> 7 < 0x1b) || (iVar3 = __ftol(), 0x1e < iVar3 >> 7)) ||
              (iVar3 = __ftol(), iVar3 >> 7 < 0x15)) || (iVar3 = __ftol(), 0x18 < iVar3 >> 7)) {
            bVar1 = false;
          }
          else {
            bVar1 = true;
          }
          if (bVar1) {
            *(undefined4 *)(DAT_005ccf9c + 0x2a6e4) = 0x3f800000;
            *(undefined4 *)(DAT_005ccf9c + 0x2a6e8) = 0x3f800000;
            *(undefined4 *)(DAT_005ccf9c + 0x2a6ec) = 0x3f800000;
            *(undefined4 *)(DAT_005ccf9c + 0x2b044) = DAT_005be0b8;
            *(undefined4 *)(DAT_005ccf9c + 0x2b048) = DAT_005be0bc;
            *(undefined4 *)(DAT_005ccf9c + 0x2a6f4) = 0x3f19999a;
            *(undefined4 *)(DAT_005ccf9c + 0x2a6f8) = 0x3f19999a;
            *(undefined4 *)(DAT_005ccf9c + 0x2a6fc) = 0x3f19999a;
            *(undefined4 *)(DAT_005ccf9c + 0x2a6f0) = 0x3f800000;
            *(undefined4 *)(param_1 + 0x38) = 0x3f666666;
            DAT_005b48e8 = 0x42b40000;
            *(undefined4 *)(DAT_005ccf9c + 0x2b034) = 0xffffffff;
            iVar3 = __ftol();
            if (((iVar3 >> 7 == 0x1d) || (iVar3 = __ftol(), iVar3 >> 7 == 0x1e)) &&
               (iVar3 = __ftol(), iVar3 >> 7 == 0x16)) {
              bVar1 = true;
            }
            else {
              bVar1 = false;
            }
            if (bVar1) {
              *(undefined4 *)(*(int *)(DAT_0067cf38 + 0x48) + 0x164) = 1;
            }
          }
          else {
            *(undefined4 *)(DAT_005ccf9c + 0x2b044) = DAT_005be0b0;
            *(undefined4 *)(DAT_005ccf9c + 0x2b048) = DAT_005be0b4;
            *(undefined4 *)(param_1 + 0x38) = 0x3f000000;
            if (*(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x2c) <= _DAT_005a43f8) {
              iVar3 = __ftol();
              if (iVar3 == 0x18) {
                *(undefined4 *)(DAT_005ccf9c + 0x2b048) = 0x41c00000;
              }
              else {
                *(float *)(DAT_005ccf9c + 0x2b048) =
                     _DAT_005a418c -
                     (_DAT_005a418c - *(float *)(DAT_005ccf9c + 0x2b048)) * _DAT_005a40f4;
              }
            }
            else {
              iVar3 = __ftol();
              if (iVar3 == 0x22) {
                *(undefined4 *)(DAT_005ccf9c + 0x2b048) = 0x42080000;
              }
              else {
                *(float *)(DAT_005ccf9c + 0x2b048) =
                     _DAT_005a565c -
                     (_DAT_005a565c - *(float *)(DAT_005ccf9c + 0x2b048)) * _DAT_005a40f4;
              }
            }
          }
        }
        *(float *)(DAT_005ccf9c + 0x2b044) = *(float *)(DAT_005ccf9c + 0x2b044) + _DAT_005a4104;
        *(float *)(DAT_005ccf9c + 0x2b048) = *(float *)(DAT_005ccf9c + 0x2b048) + _DAT_005a4104;
        if (*(int *)(param_1 + 0x164) / 10 == 1) {
          iVar3 = (**(code **)(*DAT_0092e654 + 8))();
          local_4c = iVar3 - *(int *)(param_1 + 0x168);
          if (local_4c < *(uint *)(param_1 + 0x16c)) {
            local_5c = (float)local_4c / (float)*(int *)(param_1 + 0x16c);
            iVar3 = __ftol();
            local_8 = (undefined *)(iVar3 << 0x18 | 0x888888);
            if (*(int *)(param_1 + 0x164) == 0xd) {
              iVar3 = __ftol();
              local_60 = iVar3 << 0x18 | 0xffffff;
              (**(code **)(*(int *)(param_1 + 0x1f74) + 0x3c))(local_60);
            }
            else if ((*(int *)(param_1 + 0x164) == 10) && (*(int *)(param_1 + 0x160) == 0x43)) {
              iVar3 = __ftol();
              local_64 = iVar3 << 0x18 | 0xffffff;
              (**(code **)(*(int *)(param_1 + 0x1f74) + 0x3c))(local_64);
              if (*(int *)(DAT_0067cf38 + 0x48) != 0) {
                *(float *)(*(int *)(DAT_0067cf38 + 0x48) + 0x168) = local_5c;
              }
            }
            else if ((10 < *(int *)(param_1 + 0x164)) &&
                    (((*(int *)(param_1 + 0x160) == 0x44 || (*(int *)(param_1 + 0x160) == 0x45)) &&
                     (*(int *)(DAT_0067cf38 + 0x48) != 0)))) {
              *(float *)(*(int *)(DAT_0067cf38 + 0x48) + 0x168) = _DAT_005a3660 - local_5c;
            }
            local_54 = __ftol();
            local_54 = local_54 & 0xff;
            local_58 = __ftol();
            local_58 = local_58 & 0xff;
            local_50 = __ftol();
            local_50 = local_50 & 0xff;
            *(uint *)(DAT_005ccf9c + 0x2b034) = local_54 << 0x10 | local_58 << 8 | local_50;
            *(float *)(DAT_005ccf9c + 0x2a6e4) =
                 local_5c * *(float *)(&DAT_005be038 + (*(int *)(param_1 + 0x160) + -0x43) * 0x10) +
                 (_DAT_005a3660 - local_5c) *
                 *(float *)(&DAT_005be038 + (*(int *)(local_30 + 0x298) + -0x43) * 0x10);
            *(float *)(DAT_005ccf9c + 0x2a6e8) =
                 local_5c * *(float *)(&DAT_005be03c + (*(int *)(param_1 + 0x160) + -0x43) * 0x10) +
                 (_DAT_005a3660 - local_5c) *
                 *(float *)(&DAT_005be03c + (*(int *)(local_30 + 0x298) + -0x43) * 0x10);
            *(float *)(DAT_005ccf9c + 0x2a6ec) =
                 local_5c * *(float *)(&DAT_005be040 + (*(int *)(param_1 + 0x160) + -0x43) * 0x10) +
                 (_DAT_005a3660 - local_5c) *
                 *(float *)(&DAT_005be040 + (*(int *)(local_30 + 0x298) + -0x43) * 0x10);
          }
          else {
            FUN_00549bbf(*(int *)(param_1 + 0x164) % 10);
            iVar3 = DAT_005ccf9c;
            iVar4 = (*(int *)(param_1 + 0x164) % 10) * 0x10;
            *(undefined4 *)(DAT_005ccf9c + 0x2a6e4) = *(undefined4 *)(&DAT_005be038 + iVar4);
            *(undefined4 *)(iVar3 + 0x2a6e8) = *(undefined4 *)(&DAT_005be03c + iVar4);
            *(undefined4 *)(iVar3 + 0x2a6ec) = *(undefined4 *)(&DAT_005be040 + iVar4);
            *(undefined4 *)(iVar3 + 0x2a6f0) = *(undefined4 *)(&DAT_005be044 + iVar4);
          }
        }
        else {
          FUN_00549bbf(*(int *)(param_1 + 0x164) % 10);
          iVar3 = DAT_005ccf9c;
          iVar4 = (*(int *)(param_1 + 0x164) % 10) * 0x10;
          *(undefined4 *)(DAT_005ccf9c + 0x2a6e4) = *(undefined4 *)(&DAT_005be038 + iVar4);
          *(undefined4 *)(iVar3 + 0x2a6e8) = *(undefined4 *)(&DAT_005be03c + iVar4);
          *(undefined4 *)(iVar3 + 0x2a6ec) = *(undefined4 *)(&DAT_005be040 + iVar4);
          *(undefined4 *)(iVar3 + 0x2a6f0) = *(undefined4 *)(&DAT_005be044 + iVar4);
        }
      }
      if (*(int *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x90) != 0) {
        *(undefined4 *)(DAT_005ccf9c + 0x2b044) = 0x3f800000;
        *(undefined4 *)(DAT_005ccf9c + 0x2b048) = 0x41200000;
        if (((*(int *)(param_1 + 0x164) == 0) || (DAT_005ccfa0 == 1)) || (DAT_005ccfa0 == 2)) {
          *(undefined4 *)(DAT_005ccf9c + 0x2b034) = 0xff335599;
        }
        else if (*(int *)(param_1 + 0x164) == 1) {
          *(undefined4 *)(DAT_005ccf9c + 0x2b034) = 0xff333333;
        }
        else if (*(int *)(param_1 + 0x164) == 2) {
          *(undefined4 *)(DAT_005ccf9c + 0x2b034) = 0xff441100;
        }
        else if (*(int *)(param_1 + 0x164) == 3) {
          *(undefined4 *)(DAT_005ccf9c + 0x2b034) = 0xff222222;
        }
      }
      if (((*(int *)(param_1 + 0x164) == 0) || (*(int *)(param_1 + 0x164) == 2)) ||
         (*(int *)(param_1 + 0x164) == 0xc)) {
        for (local_68 = 0; local_68 < 2; local_68 = local_68 + 1) {
          local_6c = *(int *)(DAT_0067cf38 + 0x70 + local_68 * 4);
          if (local_6c != 0) {
            local_70 = *(int *)(local_6c + 0x28);
            for (local_74 = 0; local_74 < local_70; local_74 = local_74 + 1) {
              *(undefined4 *)(*(int *)(local_6c + 0x2c + local_74 * 4) + 0x28) = 0;
            }
          }
        }
      }
      else {
        for (local_78 = 0; local_78 < 2; local_78 = local_78 + 1) {
          local_7c = *(int *)(DAT_0067cf38 + 0x70 + local_78 * 4);
          if (local_7c != 0) {
            local_80 = *(int *)(local_7c + 0x28);
            for (local_84 = 0; local_84 < local_80; local_84 = local_84 + 1) {
              *(undefined4 *)(*(int *)(local_7c + 0x2c + local_84 * 4) + 0x28) = 1;
            }
          }
        }
      }
      if (*(int *)(DAT_005ccf9c + 0x2a390) == 1) {
        local_8 = (undefined *)0xffffffff;
      }
      for (local_28 = 0; local_28 < (int)local_2c; local_28 = local_28 + 1) {
        *(undefined **)(local_c + 0xc + local_28 * 0x18) = local_8;
      }
      (**(code **)(**(int **)(local_30 + 0x318) + 0x30))(*(undefined4 *)(local_30 + 0x318));
      uVar2 = FUN_004b14fd(0x15,1,180000);
      *(undefined4 *)(param_1 + 0x170) = uVar2;
      if (*(int *)(param_1 + 0x170) != 0) {
        (**(code **)(**(int **)(*(int *)(param_1 + 0x170) + 0x318) + 0x34))
                  (*(undefined4 *)(*(int *)(param_1 + 0x170) + 0x318),local_24);
        (**(code **)(**(int **)(*(int *)(param_1 + 0x170) + 0x318) + 0x2c))
                  (*(undefined4 *)(*(int *)(param_1 + 0x170) + 0x318),0,0,&local_c,0);
        local_8c = local_14 / 0x18;
        for (local_88 = 0; local_88 < (int)local_8c; local_88 = local_88 + 1) {
          *(undefined4 *)(local_c + 0xc + local_88 * 0x18) = 0xaa8888aa;
        }
        *(undefined4 *)(*(int *)(param_1 + 0x170) + 0x298) = 0xd7;
        (**(code **)(**(int **)(*(int *)(param_1 + 0x170) + 0x318) + 0x30))
                  (*(undefined4 *)(*(int *)(param_1 + 0x170) + 0x318));
      }
      if (((*(int *)(param_1 + 0x164) == 3) || (*(int *)(param_1 + 0x164) == 0xd)) ||
         ((*(int *)(param_1 + 0x164) == 10 && (local_30 != -0x298)))) {
        CSimpleArray<>();
        (**(code **)(**(int **)(DAT_013b71e8 + 0x1b084) + 0x38))(&local_98);
        for (local_28 = 0; local_28 < 0x14; local_28 = local_28 + 1) {
          FUN_004310a0(param_1 + 0x1e8 + local_28 * 0x180);
          *(float *)(param_1 + 0x1e8 + local_28 * 0x180) =
               local_98 + *(float *)(param_1 + 0x1e8 + local_28 * 0x180);
          *(float *)(param_1 + 0x1ec + local_28 * 0x180) =
               local_94 + *(float *)(param_1 + 0x1ec + local_28 * 0x180);
          *(float *)(param_1 + 0x1f0 + local_28 * 0x180) =
               local_90 + *(float *)(param_1 + 0x1f0 + local_28 * 0x180);
          (**(code **)(*(int *)(param_1 + 0x174 + local_28 * 0x180) + 0x20))(param_2);
          FUN_004310a0(local_a4);
        }
        FUN_004310a0(param_1 + 0x1fe8);
        *(float *)(param_1 + 0x1fe8) = local_98 + *(float *)(param_1 + 0x1fe8);
        *(float *)(param_1 + 0x1fec) = local_94 + *(float *)(param_1 + 0x1fec);
        *(float *)(param_1 + 0x1ff0) = local_90 + *(float *)(param_1 + 0x1ff0);
        (**(code **)(*(int *)(param_1 + 0x1f74) + 0x20))(param_2);
        FUN_004310a0(local_a4);
        FUN_004310a0(param_1 + 0x2168);
        *(float *)(param_1 + 0x2168) = local_98 + *(float *)(param_1 + 0x2168);
        *(float *)(param_1 + 0x216c) = local_94 + *(float *)(param_1 + 0x216c);
        *(float *)(param_1 + 0x2170) = local_90 + *(float *)(param_1 + 0x2170);
        (**(code **)(*(int *)(param_1 + 0x20f4) + 0x20))(param_2);
        FUN_004310a0(local_a4);
      }
      if (_DAT_005a3670 < *(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x34)) {
        *(float *)(DAT_005ccf9c + 0x2b048) =
             (*(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x34) - _DAT_005a3670) +
             *(float *)(DAT_005ccf9c + 0x2b048);
      }
      uVar2 = 1;
    }
  }
  else {
    *(undefined4 *)(DAT_005ccf9c + 0x2b034) = 0;
    uVar2 = 0;
  }
  return uVar2;
}

