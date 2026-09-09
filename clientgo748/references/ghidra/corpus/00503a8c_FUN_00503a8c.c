// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00503a8c | Name: FUN_00503a8c


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00503a8c(int *param_1)

{
  undefined4 uVar1;
  int iVar2;
  uint uVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  undefined4 uStack_58;
  undefined4 uStack_54;
  undefined4 uStack_50;
  uint uStack_4c;
  int local_c;
  int local_8;
  
  if (param_1[0x85] == 0) {
    if ((char)param_1[5] == '\x01') {
      uVar1 = 0;
    }
    else if (param_1[0x18] == 0) {
      uVar1 = 0;
    }
    else {
      iVar2 = (**(code **)(*param_1 + 0x4c))();
      if ((iVar2 == 0) || (*(char *)((int)param_1 + 0x795) == '\x01')) {
        for (local_c = 0; local_c < 7; local_c = local_c + 1) {
          if (param_1[local_c + 0x169] != 0) {
            *(undefined4 *)(param_1[local_c + 0x169] + 0x28) = 0;
          }
        }
        for (local_c = 0; local_c < 7; local_c = local_c + 1) {
          if (param_1[local_c + 0x15b] != 0) {
            *(undefined4 *)(param_1[local_c + 0x15b] + 0x28) = 0;
          }
        }
        for (local_c = 0; local_c < 4; local_c = local_c + 1) {
          if (param_1[local_c + 0x170] != 0) {
            *(undefined4 *)(param_1[local_c + 0x170] + 0x48) = 0;
          }
        }
        for (local_c = 0; local_c < 2; local_c = local_c + 1) {
          if (param_1[local_c + 0x155] != 0) {
            *(undefined4 *)(param_1[local_c + 0x155] + 0x28) = 0;
          }
        }
        FUN_0050494d();
        uVar1 = 1;
      }
      else if (param_1[0x93] == 0x2d) {
        uVar1 = 1;
      }
      else {
        for (local_8 = 0; local_8 < 7; local_8 = local_8 + 1) {
          if (param_1[local_8 + 0x169] != 0) {
            *(undefined4 *)(param_1[local_8 + 0x169] + 0x28) = 1;
          }
        }
        for (local_8 = 0; local_8 < 7; local_8 = local_8 + 1) {
          if (param_1[local_8 + 0x15b] != 0) {
            *(undefined4 *)(param_1[local_8 + 0x15b] + 0x28) = 1;
          }
        }
        for (local_8 = 0; local_8 < 4; local_8 = local_8 + 1) {
          if (param_1[local_8 + 0x170] != 0) {
            *(undefined4 *)(param_1[local_8 + 0x170] + 0x48) = 1;
          }
        }
        for (local_8 = 0; local_8 < 2; local_8 = local_8 + 1) {
          if (param_1[local_8 + 0x155] != 0) {
            *(undefined4 *)(param_1[local_8 + 0x155] + 0x28) = 1;
          }
        }
        if (DAT_005ccf9c[0xa944] == 0x20) {
          (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xaa);
        }
        else {
          (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xff);
        }
        if ((*(char *)((int)param_1 + 0x795) == '\x01') &&
           (*(int **)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x54) != param_1)) {
          uVar1 = 1;
        }
        else if (*(char *)((int)param_1 + 0x78d) == '\x01') {
          FUN_0050494d();
          uVar1 = 1;
        }
        else {
          if (*(char *)((int)param_1 + 0x795) == '\x01') {
            (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
            (**(code **)(*DAT_005ccf9c + 0x30))(0x14,9);
            *(undefined1 *)(param_1[0x18] + 0x307) = 0;
          }
          else if ((DAT_0092e664 == 1) && (*(char *)((int)param_1 + 0x796) == '\x01')) {
            iVar2 = param_1[0x18];
            *(undefined4 *)(iVar2 + 0x288) = 0x3f800000;
            *(undefined4 *)(iVar2 + 0x28c) = 0x3f333333;
            *(undefined4 *)(iVar2 + 0x290) = 0;
            *(undefined4 *)(iVar2 + 0x294) = 0x3f800000;
            *(undefined4 *)(param_1[0x18] + 0x2a8) = 0x3f800000;
            *(undefined4 *)(param_1[0x18] + 0x2ac) = 0x3f800000;
            *(undefined4 *)(param_1[0x18] + 0x2b0) = 0x3f800000;
            *(undefined4 *)(param_1[0x18] + 0x2b8) = 0x3f800000;
            *(undefined4 *)(param_1[0x18] + 700) = 0x3f333333;
            *(undefined4 *)(param_1[0x18] + 0x2c0) = 0;
          }
          else {
            (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,0);
            (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
            (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
            *(undefined1 *)(param_1[0x18] + 0x307) = 1;
          }
          if ((param_1[0x93] == 0x20) && ((short)param_1[0x75] == 2)) {
            (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
            (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
          }
          else if (param_1[0x93] == 0x3b) {
            (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
            (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
            (**(code **)(*DAT_005ccf9c + 0x30))(0x14,5);
          }
          else {
            (**(code **)(*DAT_005ccf9c + 0x30))(0x19,7);
          }
          if ((((DAT_0067cf38 != 0) && (*(int *)(DAT_0067cf38 + 0x24) == 30000)) &&
              (param_1[0x18] != 0)) && ((char)param_1[499] == '\x01')) {
            *(undefined4 *)(param_1[0x18] + 0x2b8) = 0x3f333333;
            *(undefined4 *)(param_1[0x18] + 700) = 0x3f333333;
            *(undefined4 *)(param_1[0x18] + 0x2c0) = 0x3f666666;
          }
          if (((char)param_1[0x1e7] != '\0') && (param_1[0x67] != 0)) {
            FUN_004be1ef(0,0x3f800000,0);
            puVar4 = (undefined4 *)(param_1[0x67] + 0x74);
            puVar5 = (undefined4 *)(param_1[0x18] + 0x34);
            for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
              *puVar5 = *puVar4;
              puVar4 = puVar4 + 1;
              puVar5 = puVar5 + 1;
            }
            FUN_00430f50(_DAT_005a4adc * (float)param_1[0x1e9],_DAT_005a3660 / (float)param_1[0x1e9]
                         ,_DAT_005a4158 * (float)param_1[0x1e9]);
            uVar1 = FUN_013d3000();
            return uVar1;
          }
          FUN_004be1ef(0,0x3f800000,0);
          uVar1 = FUN_00430f50(*(undefined4 *)(param_1[0x18] + 0xc),
                               *(undefined4 *)(param_1[0x18] + 0x10),
                               *(undefined4 *)(param_1[0x18] + 0x14));
          FUN_004310a0(uVar1);
          if ((*(char *)((int)param_1 + 0x795) == '\x01') ||
             (*(char *)((int)param_1 + 0x796) == '\x01')) {
            (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,0);
            (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
            (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
          }
          if (('\0' < *(char *)((int)param_1 + 0x79b)) && (param_1[0x66] != 0)) {
            uStack_50 = 0x3f800000;
            if (param_1[0x93] == 1) {
              uStack_4c = 0;
            }
            else if (param_1[0x93] == 2) {
              uStack_4c = 1;
            }
            else if (param_1[0x93] == 4) {
              uStack_4c = 2;
            }
            else if (param_1[0x93] == 8) {
              uStack_4c = 3;
            }
            else if (param_1[0x93] == 0x26) {
              uStack_4c = 1;
            }
            else if (param_1[0x93] == 0x19) {
              uStack_4c = 5;
            }
            else if (param_1[0x93] == 0x1a) {
              uStack_4c = 6;
            }
            else if (param_1[0x93] == 0x21) {
              uStack_4c = 7;
            }
            else if (param_1[0x93] == 0x28) {
              uStack_4c = 8;
            }
            else {
              uStack_4c = 0;
            }
            if (uStack_4c < 4) {
              if (param_1[0x17] == 1) {
                uStack_50 = 0x3f666666;
              }
              uVar3 = (uint)*(ushort *)((int)param_1 + 0x1da);
              uStack_58 = *(undefined4 *)(uStack_4c * 0x50 + 0x5bb488 + uVar3 * 4);
              if (0x27 < uVar3) {
                uStack_58 = *(undefined4 *)(&DAT_005bb528 + uVar3 * 4 + uStack_4c * 0x50);
              }
              uStack_54 = 0;
              if ((0xc7c < (short)param_1[0x1f8]) && ((short)param_1[0x1f8] < 0xc80)) {
                uStack_58 = 0x3df5c28f;
                uStack_54 = 0x3d23d70a;
              }
              puVar4 = (undefined4 *)(param_1[0x18] + 0x74);
              puVar5 = (undefined4 *)(param_1[0x66] + 0x34);
              for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
                *puVar5 = *puVar4;
                puVar4 = puVar4 + 1;
                puVar5 = puVar5 + 1;
              }
              FUN_004be1ef(uStack_58,uStack_50,uStack_54);
            }
            else if (uStack_4c == 6) {
              puVar4 = (undefined4 *)(param_1[0x18] + 0x74);
              puVar5 = (undefined4 *)(param_1[0x66] + 0x34);
              for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
                *puVar5 = *puVar4;
                puVar4 = puVar4 + 1;
                puVar5 = puVar5 + 1;
              }
              FUN_004be1ef(0x3d8f5c29,0x3f99999a,0xbe4ccccd);
            }
            else if (uStack_4c == 7) {
              puVar4 = (undefined4 *)(param_1[0x18] + 0x74);
              puVar5 = (undefined4 *)(param_1[0x66] + 0x34);
              for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
                *puVar5 = *puVar4;
                puVar4 = puVar4 + 1;
                puVar5 = puVar5 + 1;
              }
              FUN_004be1ef(0,0x3f99999a,0);
            }
            else if (uStack_4c == 8) {
              puVar4 = (undefined4 *)(param_1[0x18] + 0x74);
              puVar5 = (undefined4 *)(param_1[0x66] + 0x34);
              for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
                *puVar5 = *puVar4;
                puVar4 = puVar4 + 1;
                puVar5 = puVar5 + 1;
              }
              FUN_004be1ef(0x3c23d70a,0x3f800000,0);
            }
            else {
              puVar4 = (undefined4 *)(param_1[0x18] + 0x74);
              puVar5 = (undefined4 *)(param_1[0x66] + 0x34);
              for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
                *puVar5 = *puVar4;
                puVar4 = puVar4 + 1;
                puVar5 = puVar5 + 1;
              }
              FUN_004be1ef(0x3d8f5c29,0x3f99999a,0xbdf5c28f);
            }
          }
          FUN_00500ebe();
          if (param_1[0x12] == 1) {
            FUN_00504a80();
          }
          uVar1 = 1;
        }
      }
    }
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

