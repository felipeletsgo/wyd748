// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c51fd | Name: FUN_004c51fd


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004c51fd(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  int unaff_EBP;
  undefined4 *puVar4;
  float10 fVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int *piVar9;
  
  *(undefined4 *)(unaff_EBP + -0xb8) = *(undefined4 *)(*(int *)(param_1 + 0xc0) + 0x2f4);
  if (((*(int *)(unaff_EBP + -0xb8) == 0) || (*(int *)(*(int *)(unaff_EBP + -0xb8) + 0x19c) == 0))
     || (*(int *)(*(int *)(unaff_EBP + -0xb8) + 0x160) != 0x65)) {
    if (*(int *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0xc0) + 0x350) == 0) {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x16,2);
    }
    else {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
    }
  }
  else {
    (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
  }
  if (((*(int *)(unaff_EBP + -0xb8) != 0) && (*(int *)(*(int *)(unaff_EBP + -0xb8) + 0x198) != 0))
     && (*(int *)(*(int *)(unaff_EBP + -0xb8) + 0x160) == 0x65)) {
    *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0xb8) + 0x198) + 0x350) = 1;
  }
  if ((*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0xc0) != 0) &&
     (**(int **)(*(int *)(unaff_EBP + -0x2a8) + 0xc0) == 0x55)) {
    (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
  }
  iVar1 = (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x11c))
                    (DAT_005ccf9c[0xa8f9],4,0,
                     *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 4) + 0x28),0,
                     *(undefined4 *)(*(int *)(unaff_EBP + -0x2a8) + 0xcc));
  if (iVar1 < 0) {
    uVar2 = 0;
  }
  else {
    (**(code **)(*DAT_005ccf9c + 0x34))(0,1,1);
    (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
    if ((((*(int *)(DAT_0067cf38 + 0x7c) ==
           *(int *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0xc0) + 0x2f4)) &&
         (*(int *)(DAT_0067cf38 + 0x7c) != 0)) && (*(int *)(DAT_013b71e8 + 0x1e428) == 0)) &&
       ((**(int **)(*(int *)(unaff_EBP + -0x2a8) + 0xc0) != 0xb ||
        (*(int *)(*(int *)(unaff_EBP + -0x2a8) + 8) != 2)))) {
      FUN_00430f10();
      FUN_00430f10();
      FUN_00430f10();
      iVar1 = *(int *)(DAT_013b71e8 + 0x1b084);
      *(undefined4 *)(unaff_EBP + -0x158) = *(undefined4 *)(iVar1 + 0x48);
      *(undefined4 *)(unaff_EBP + -0x154) = *(undefined4 *)(iVar1 + 0x4c);
      *(undefined4 *)(unaff_EBP + -0x150) = *(undefined4 *)(iVar1 + 0x50);
      CSimpleArray<>();
      *(undefined4 *)(unaff_EBP + -0x108) =
           *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0x14) + 0x30);
      *(undefined4 *)(unaff_EBP + -0x104) =
           *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0x14) + 0x34);
      *(undefined4 *)(unaff_EBP + -0x100) =
           *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 0x14) + 0x38);
      FUN_00493db0(unaff_EBP + -0x164,unaff_EBP + -0x158);
      fVar5 = (float10)FUN_004b3aa0();
      *(float *)(unaff_EBP + -0xfc) = (float)fVar5;
      *(float *)(unaff_EBP + -0xfc) = (float)(fVar5 * (float10)_DAT_005a46f8);
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
        piVar9 = DAT_005ccf9c + 0xa98b;
        iVar1 = unaff_EBP + -0x2a4;
        iVar8 = unaff_EBP + -0x148;
        iVar7 = unaff_EBP + -0x264;
        uVar2 = *(undefined4 *)
                 (*(int *)(unaff_EBP + -0x2a8) + 0x14 + *(int *)(unaff_EBP + -0x14c) * 4);
        iVar6 = unaff_EBP + -0x224;
        FUN_004c5b90(unaff_EBP + -0x1e4,unaff_EBP + -0x1a4);
        FUN_004c5b90(iVar6,uVar2);
        FUN_004c5b90(iVar7,iVar8);
        puVar3 = (undefined4 *)FUN_004c5b90(iVar1,piVar9);
        puVar4 = (undefined4 *)(unaff_EBP + -0xf8);
        for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
          *puVar4 = *puVar3;
          puVar3 = puVar3 + 1;
          puVar4 = puVar4 + 1;
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
      iVar1 = (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x11c))
                        (DAT_005ccf9c[0xa8f9],4,0,
                         *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x2a8) + 4) + 0x28),0,
                         *(undefined4 *)(*(int *)(unaff_EBP + -0x2a8) + 0xcc));
      if (iVar1 < 0) {
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

