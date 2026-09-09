// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 013c6100 | Name: FUN_013c6100


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_013c6100(void)

{
  ushort uVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  uint uVar4;
  uint *puVar5;
  int iVar6;
  int unaff_EBP;
  undefined4 *puVar7;
  float10 fVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int *piVar12;
  
  iVar6 = *(int *)(*(int *)(unaff_EBP + -0x2a8) + 0xc0);
  if (*(char *)(iVar6 + 0x351) == '\x7f') {
LAB_013c6161:
    *(undefined4 *)(unaff_EBP + -0xb8) =
         *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0xc0) + 0x2f4);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
    if (((*(int *)(unaff_EBP + -0xb8) != 0) && (*(int *)(*(int *)(unaff_EBP + -0xb8) + 0x198) != 0))
       && (*(int *)(*(int *)(unaff_EBP + -0xb8) + 0x160) == 0x65)) {
      *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0xb8) + 0x198) + 0x350) = 1;
    }
    if ((*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0xc0) != 0) &&
       (**(int **)(*(int *)(unaff_EBP + -0x2a8) + 0xc0) == 0x55)) {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
    }
    iVar6 = (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x11c))
                      (DAT_005ccf9c[0xa8f9],4,0,
                       *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 4) + 0x28),0,
                       *(undefined4 *)(*(int *)(unaff_EBP + -0x2a8) + 0xcc));
    if (iVar6 < 0) {
      uVar2 = 0;
    }
    else {
      (**(code **)(*DAT_005ccf9c + 0x34))(0,1,1);
      (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
      if (((*(int *)(DAT_0067cf38 + 0x7c) ==
            *(int *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0xc0) + 0x2f4)) &&
          (*(int *)(DAT_0067cf38 + 0x7c) != 0)) &&
         ((*(int *)(DAT_013b71e8 + 0x1e428) == 0 &&
          ((**(int **)(*(int *)(unaff_EBP + -0x2a8) + 0xc0) != 0xb ||
           (*(int *)(*(int *)(unaff_EBP + -0x2a8) + 8) != 2)))))) {
        FUN_00430f10();
        FUN_00430f10();
        FUN_00430f10();
        iVar6 = *(int *)(DAT_013b71e8 + 0x1b084);
        *(undefined4 *)(unaff_EBP + -0x158) = *(undefined4 *)(iVar6 + 0x48);
        *(undefined4 *)(unaff_EBP + -0x154) = *(undefined4 *)(iVar6 + 0x4c);
        *(undefined4 *)(unaff_EBP + -0x150) = *(undefined4 *)(iVar6 + 0x50);
        CSimpleArray<>();
        *(undefined4 *)(unaff_EBP + -0x108) =
             *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0x14) + 0x30);
        *(undefined4 *)(unaff_EBP + -0x104) =
             *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0x14) + 0x34);
        *(undefined4 *)(unaff_EBP + -0x100) =
             *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0x14) + 0x38);
        FUN_00493db0(unaff_EBP + -0x164,unaff_EBP + -0x158);
        fVar8 = (float10)FUN_004b3aa0();
        *(float *)(unaff_EBP + -0xfc) = (float)fVar8;
        *(float *)(unaff_EBP + -0xfc) = (float)(fVar8 * (float10)_DAT_005a46f8);
        thunk_FUN_005611dd(unaff_EBP + -0x164,unaff_EBP + -0x164);
        thunk_FUN_0056239d(unaff_EBP + -0x148,*(float *)(unaff_EBP + -0x164) * _DAT_005a4214,
                           *(float *)(unaff_EBP + -0x160) * _DAT_005a4214,
                           *(float *)(unaff_EBP + -0x15c) * _DAT_005a4214);
        if ((**(int **)(*(int *)(unaff_EBP + -0x2a8) + 0xc0) == 0x29) &&
           (*(short *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0xc0) + 0x248) == 0)) {
          thunk_FUN_0056230d(unaff_EBP + -0x1a4,_DAT_005a46f4 + *(float *)(unaff_EBP + -0xfc),
                             _DAT_005a46f4 + *(float *)(unaff_EBP + -0xfc),
                             _DAT_005a46f4 + *(float *)(unaff_EBP + -0xfc));
        }
        else {
          thunk_FUN_0056230d(unaff_EBP + -0x1a4,_DAT_005a3660 + *(float *)(unaff_EBP + -0xfc),
                             _DAT_005a3660 + *(float *)(unaff_EBP + -0xfc),
                             _DAT_005a3660 + *(float *)(unaff_EBP + -0xfc));
        }
        *(undefined4 *)(unaff_EBP + -0x14c) = 0;
        while (*(uint *)(unaff_EBP + -0x14c) < *(uint *)(*(int *)(unaff_EBP + -0x2a8) + 0xc)) {
          piVar12 = DAT_005ccf9c + 0xa98b;
          iVar6 = unaff_EBP + -0x2a4;
          iVar11 = unaff_EBP + -0x148;
          iVar10 = unaff_EBP + -0x264;
          uVar2 = *(undefined4 *)
                   (*(int *)(unaff_EBP + -0x2a8) + 0x14 + *(int *)(unaff_EBP + -0x14c) * 4);
          iVar9 = unaff_EBP + -0x224;
          FUN_004c5b90(unaff_EBP + -0x1e4,unaff_EBP + -0x1a4);
          FUN_004c5b90(iVar9,uVar2);
          FUN_004c5b90(iVar10,iVar11);
          puVar3 = (undefined4 *)FUN_004c5b90(iVar6,piVar12);
          puVar7 = (undefined4 *)(unaff_EBP + -0xf8);
          for (iVar6 = 0x10; iVar6 != 0; iVar6 = iVar6 + -1) {
            *puVar7 = *puVar3;
            puVar3 = puVar3 + 1;
            puVar7 = puVar7 + 1;
          }
          thunk_FUN_00561ca4(unaff_EBP + -0xf8,unaff_EBP + -0xf8);
          (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x13c))
                    (DAT_005ccf9c[0xa8f9],*(int *)(unaff_EBP + -0x14c) * 3 + 9,unaff_EBP + -0xf8,3);
          *(int *)(unaff_EBP + -0x14c) = *(int *)(unaff_EBP + -0x14c) + 1;
        }
        if (*(int *)(DAT_0067cf38 + 0x7c) ==
            *(int *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0xc0) + 0x2f4)) {
          *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0xc0) + 0x288) = 0x3f800000;
          *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0xc0) + 0x28c) = 0x3f800000;
          *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0xc0) + 0x290) = 0x3f800000;
          *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0xc0) + 0x2a8) = 0x3f800000;
          *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0xc0) + 0x2ac) = 0x3f800000;
          *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0xc0) + 0x2b0) = 0x3f800000;
          *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0xc0) + 0x2b8) = 0x3f800000;
          *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0xc0) + 700) = 0x3f800000;
          *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0xc0) + 0x2c0) = 0x3f800000;
        }
        (**(code **)(*DAT_005ccf9c + 0x30))
                  (0x3c,*(undefined4 *)(*(int *)(DAT_0067cf38 + 0x7c) + 0x5e4));
        uVar2 = FUN_004b9a4b(*(undefined4 *)(*(int *)(unaff_EBP + -0x2a8) + 200),180000);
        (**(code **)(*DAT_005ccf9c + 0x38))(0,uVar2);
        (**(code **)(*DAT_005ccf9c + 0x30))(9,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
        (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,0);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,1,2);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,2,3);
        (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,5,2);
        (**(code **)(*DAT_005ccf9c + 0x34))(1,4,1);
        iVar6 = (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x11c))
                          (DAT_005ccf9c[0xa8f9],4,0,
                           *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 4) + 0x28),0,
                           *(undefined4 *)(*(int *)(unaff_EBP + -0x2a8) + 0xcc));
        if (iVar6 < 0) {
          return 0;
        }
        (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
        (**(code **)(*DAT_005ccf9c + 0x30))(9,2);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,2,2);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,5,2);
        (**(code **)(*DAT_005ccf9c + 0x34))(1,4,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
      }
      if (DAT_005ccf9c[0xaa24] != 0) {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x99,0);
      }
      if (0 < *(short *)(*(int *)(unaff_EBP + -0x2a8) + 0xc4)) {
        if (*(char *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0xc0) + 0x307) == '\x01') {
          (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
        }
        (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,1);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
        (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
      }
      (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,3,1);
      uVar2 = 1;
    }
    return uVar2;
  }
  uVar1 = *(ushort *)(iVar6 + 0x356);
  if (((uVar1 & 0x4000) != 0) && (uVar4 = uVar1 & 0x3fff, uVar4 < 0x12f)) {
    puVar5 = (uint *)(&DAT_013c2800 + uVar4 * 0x30);
    iVar6 = 0xc;
    uVar4 = 0;
    do {
      uVar4 = uVar4 | *puVar5;
      puVar5 = puVar5 + 1;
      iVar6 = iVar6 + -1;
    } while (iVar6 != 0);
    if (uVar4 != 0) goto LAB_013c6161;
  }
  uVar2 = FUN_004c51fd();
  return uVar2;
}

