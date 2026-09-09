// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004a4582 | Name: FUN_004a4582


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall
FUN_004a4582(int param_1,int param_2,undefined4 param_3,int param_4,int param_5)

{
  short sVar1;
  bool bVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  int *piVar6;
  undefined4 uVar7;
  int *local_2d4 [17];
  int local_290;
  int local_28c;
  int local_288;
  int local_284;
  undefined4 local_280;
  undefined *local_27c [4];
  int *local_26c;
  int *local_268;
  undefined *local_264 [4];
  undefined *local_254 [4];
  undefined *local_244 [4];
  int *local_234;
  int *local_230;
  int local_22c [5];
  undefined1 local_218 [128];
  int local_198;
  int *local_194;
  int *local_190;
  undefined1 local_18c [128];
  int *local_10c;
  undefined *local_108 [4];
  undefined *local_f8;
  int *local_f4;
  int *local_f0;
  int *local_ec;
  undefined *local_e8 [4];
  undefined *local_d8;
  undefined4 local_d4;
  int local_d0;
  int *local_cc;
  int *local_c8;
  int *local_c4;
  int *local_c0;
  int local_bc;
  int *local_b8;
  int *local_b4;
  int *local_b0;
  short local_ac;
  undefined2 uStack_aa;
  int *local_a8;
  int *local_a4;
  int *local_a0;
  int *local_9c;
  undefined1 local_98 [128];
  int *local_18;
  int local_14;
  undefined4 local_10;
  int local_c;
  int local_8;
  
  iVar3 = FUN_0049ab73(param_2,param_3,param_4,param_5);
  if (iVar3 == 1) {
    uVar4 = 1;
  }
  else {
    iVar3 = FUN_0040c0f0();
    if (iVar3 == 1) {
      uVar4 = 1;
    }
    else {
      iVar3 = FUN_0040c0f0();
      if (iVar3 == 1) {
        uVar4 = 1;
      }
      else {
        iVar3 = FUN_0040c0f0();
        if (iVar3 == 1) {
          uVar4 = 1;
        }
        else if (*(int *)(param_1 + 0x54) == 1) {
          uVar4 = 1;
        }
        else if ((*(int *)(param_1 + 0x26e68) == 0) ||
                (*(int *)(*(int *)(param_1 + 0x26e68) + 0x28) != 1)) {
          if ((param_2 == 0x201) &&
             ((((0 < param_4 && (0 < param_5)) &&
               (param_4 < *(int *)(DAT_005ccf9c + 0x2a504) - *(int *)(DAT_005ccf9c + 0x2a5e4))) &&
              (param_5 < *(int *)(DAT_005ccf9c + 0x2a508) - *(int *)(DAT_005ccf9c + 0x2a5e8))))) {
            local_8 = *(int *)(param_1 + 0x7c);
            if (*(int *)(param_1 + 0x26ef4) == 1) {
              if ((DAT_005b892c == 2) && (iVar3 = FUN_0040c0f0(), iVar3 == 1)) {
                return 1;
              }
              local_c = (int)*(char *)(DAT_013b71e8 + 0xef4);
              for (local_14 = 0; local_14 < 4; local_14 = local_14 + 1) {
                if (*(int *)(param_1 + 0x26e30 + local_14 * 4) != 0) {
                  *(undefined4 *)(*(int *)(param_1 + 0x26e30 + local_14 * 4) + 0x234) = 0;
                }
              }
              for (local_14 = 0; local_14 < 4; local_14 = local_14 + 1) {
                if ((((local_8 != 0) && (*(int *)(param_1 + 0x26e30 + local_14 * 4) != 0)) &&
                    (*(int *)(param_1 + 0x26e30 + local_14 * 4) == local_8)) &&
                   (*(int *)(local_8 + 0x238) == 1)) {
                  *(undefined1 *)(DAT_013b71e8 + 0xef4) = (undefined1)local_14;
                  *(undefined4 *)(*(int *)(param_1 + 0x26e30 + local_14 * 4) + 0x234) = 1;
                  local_d4 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x502);
                  iVar3 = __ftol();
                  iVar5 = __ftol();
                  if (iVar3 == iVar5) {
                    if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x39), iVar3 != 0)) {
                      uVar7 = 0;
                      uVar4 = 0;
                      FUN_00429a6d(0x39);
                      FUN_0042ad2b(uVar4,uVar7);
                    }
                    uVar4 = (**(code **)(*DAT_0092e654 + 8))();
                    *(undefined4 *)(param_1 + 0x26e74) = uVar4;
                    FUN_004a24b4(s_UI_SelCamAction2_005b9448);
                  }
                  local_9c = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x506);
                  local_c4 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x508);
                  local_10c = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x509);
                  local_ec = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x521);
                  local_f4 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x522);
                  local_a8 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x523);
                  local_b8 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x524);
                  local_f0 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x525);
                  local_cc = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x526);
                  local_b4 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x527);
                  local_a4 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x529);
                  local_b0 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x52a);
                  local_c0 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x52b);
                  local_c8 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x52c);
                  local_a0 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x520);
                  local_e8[0] = &DAT_00a37df8;
                  local_e8[1] = &DAT_00a37e78;
                  local_e8[2] = &DAT_00a37ef8;
                  local_e8[3] = &DAT_00a37f78;
                  local_d0 = (int)*(short *)(DAT_013b71e8 + 0xc4 + local_14 * 0x80) % 10;
                  if (local_d0 == 1) {
                    local_d0 = (int)*(short *)(DAT_013b71e8 + 0xc4 + local_14 * 0x80) / 10;
                    _DAT_005d04dc = (int)*(short *)(DAT_013b71e8 + 0xc4 + local_14 * 0x80);
                  }
                  else if (local_d0 < 6) {
                    local_d0 = 2;
                  }
                  else {
                    _DAT_005d04dc = (int)*(short *)(DAT_013b71e8 + 0xc4 + local_14 * 0x80);
                    local_d0 = local_d0 + -6;
                  }
                  if (3 < local_d0) {
                    local_d0 = 0;
                  }
                  (**(code **)(*local_9c + 0x80))
                            (*(int *)(param_1 + 0x26e30 + local_14 * 4) + 0x168,1);
                  if (*(short *)(DAT_013b71e8 + 0xc4 + local_14 * 0x80) < 0x28) {
                    (**(code **)(*local_c4 + 0x80))(local_e8[local_d0],0);
                  }
                  else {
                    (**(code **)(*local_c4 + 0x80))(&DAT_00a3d478,0);
                  }
                  FUN_0058f078(local_98,&DAT_005b945c,
                               *(short *)(DAT_013b71e8 + 0x54 + local_14 * 0x1c) + 1);
                  (**(code **)(*local_10c + 0x80))(local_98,0);
                  FUN_0058f078(local_98,&DAT_005b9460,
                               *(undefined4 *)(DAT_013b71e8 + 0x2cc + local_14 * 4));
                  *(undefined1 *)((int)local_ec + 0xcdd) = 1;
                  (**(code **)(*local_ec + 0x80))(local_98,0);
                  FUN_0058f078(local_98,&DAT_005b9468,
                               *(undefined4 *)(DAT_013b71e8 + 0x2dc + local_14 * 4));
                  (**(code **)(*local_f4 + 0x80))(local_98,0);
                  if ((*(short *)(DAT_013b71e8 + 0x13c + local_14 * 0x80) < 0xc7d) ||
                     (0xc7f < *(short *)(DAT_013b71e8 + 0x13c + local_14 * 0x80))) {
                    bVar2 = false;
                  }
                  else {
                    bVar2 = true;
                  }
                  if (bVar2) {
                    local_d8 = (&PTR_DAT_005c1900)
                               [*(short *)(DAT_013b71e8 + 0x54 + local_14 * 0x1c)];
                  }
                  else {
                    local_d8 = *(undefined **)
                                (&DAT_005c12b4 +
                                *(short *)(DAT_013b71e8 + 0x54 + local_14 * 0x1c) * 4);
                  }
                  FUN_0058f078(local_98,&DAT_005b946c,local_d8);
                  (**(code **)(*local_a8 + 0x80))(local_98,0);
                  FUN_0058f078(local_98,&DAT_005b9470,
                               (int)*(short *)(DAT_013b71e8 + 100 + local_14 * 0x1c));
                  (**(code **)(*local_b8 + 0x80))(local_98,0);
                  FUN_0058f078(local_98,&DAT_005b9474,
                               (int)*(short *)(DAT_013b71e8 + 0x66 + local_14 * 0x1c));
                  (**(code **)(*local_f0 + 0x80))(local_98,0);
                  FUN_0058f078(local_98,&DAT_005b9478,
                               (int)*(short *)(DAT_013b71e8 + 0x68 + local_14 * 0x1c));
                  (**(code **)(*local_cc + 0x80))(local_98,0);
                  FUN_0058f078(local_98,&DAT_005b947c,
                               (int)*(short *)(DAT_013b71e8 + 0x6a + local_14 * 0x1c));
                  (**(code **)(*local_b4 + 0x80))(local_98,0);
                  FUN_0058f078(local_98,&DAT_005b9480,
                               *(undefined1 *)(DAT_013b71e8 + 0x6c + local_14 * 0x1c));
                  (**(code **)(*local_a4 + 0x80))(local_98,0);
                  FUN_0058f078(local_98,&DAT_005b9484,
                               *(undefined1 *)(DAT_013b71e8 + 0x6d + local_14 * 0x1c));
                  (**(code **)(*local_b0 + 0x80))(local_98,0);
                  FUN_0058f078(local_98,&DAT_005b9488,
                               *(undefined1 *)(DAT_013b71e8 + 0x6e + local_14 * 0x1c));
                  (**(code **)(*local_c0 + 0x80))(local_98,0);
                  FUN_0058f078(local_98,&DAT_005b948c,
                               *(undefined1 *)(DAT_013b71e8 + 0x6f + local_14 * 0x1c));
                  (**(code **)(*local_c8 + 0x80))(local_98,0);
                  sVar1 = *(short *)(DAT_013b71e8 + 0x2c4 + local_14 * 2);
                  _local_ac = CONCAT22(uStack_aa,sVar1);
                  if (sVar1 == 0) {
                    (**(code **)(*local_a0 + 0x80))(&DAT_005d04e4,0);
                    (**(code **)(*local_a0 + 0x84))(0xffffffff);
                  }
                  else {
                    FUN_0058f078(local_98,DAT_013b71e8 + 0xf18 + local_14 * 0xc);
                    FUN_0058ee20(DAT_013b71e8 + 0x1af70,DAT_013b71e8 + 0xf18 + local_14 * 0xc);
                    (**(code **)(*local_a0 + 0x80))(local_98,0);
                    (**(code **)(*local_a0 + 0x84))(0xffaaaaaa);
                  }
                  local_108[0] = &DAT_00a37ff8;
                  local_108[1] = &DAT_00a38078;
                  local_108[2] = &DAT_00a397f8;
                  local_108[3] = &DAT_00a3e1f8;
                  local_f8 = &DAT_00a380f8;
                  local_bc = FUN_00552f42(*(undefined2 *)(DAT_013b71e8 + 4 + local_14 * 2),
                                          *(undefined2 *)(DAT_013b71e8 + 0xc + local_14 * 2));
                  local_18 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x510);
                  if ((local_bc < 5) && (-1 < local_bc)) {
                    (**(code **)(*local_18 + 0x80))(local_108[local_bc],0);
                  }
                  else {
                    FUN_0058f078(local_18c,s___d___d__005b9490,
                                 *(undefined2 *)(DAT_013b71e8 + 4 + local_14 * 2),
                                 *(undefined2 *)(DAT_013b71e8 + 0xc + local_14 * 2));
                    (**(code **)(*local_18 + 0x80))(local_18c,0);
                  }
                  if (DAT_005b892c == 2) {
                    switch(local_d0) {
                    case 0:
                      (**(code **)(**(int **)(param_1 + 0x26ebc) + 0x80))(&DAT_00a3ba78,0);
                      (**(code **)(**(int **)(param_1 + 0x26ec0) + 0x80))(&DAT_00a3baf8,0);
                      (**(code **)(**(int **)(param_1 + 0x26ec4) + 0x80))(&DAT_00a3bb78,0);
                      (**(code **)(**(int **)(param_1 + 0x26ec8) + 0x80))(&DAT_00a3bbf8,0);
                      break;
                    case 1:
                      (**(code **)(**(int **)(param_1 + 0x26ebc) + 0x80))(&DAT_00a3bc78,0);
                      (**(code **)(**(int **)(param_1 + 0x26ec0) + 0x80))(&DAT_00a3bcf8,0);
                      (**(code **)(**(int **)(param_1 + 0x26ec4) + 0x80))(&DAT_00a3bd78,0);
                      (**(code **)(**(int **)(param_1 + 0x26ec8) + 0x80))(&DAT_00a3bdf8,0);
                      break;
                    case 2:
                      (**(code **)(**(int **)(param_1 + 0x26ebc) + 0x80))(&DAT_00a3be78,0);
                      (**(code **)(**(int **)(param_1 + 0x26ec0) + 0x80))(&DAT_00a3bef8,0);
                      (**(code **)(**(int **)(param_1 + 0x26ec4) + 0x80))(&DAT_00a3bf78,0);
                      (**(code **)(**(int **)(param_1 + 0x26ec8) + 0x80))(&DAT_00a3bff8,0);
                      break;
                    case 3:
                      (**(code **)(**(int **)(param_1 + 0x26ebc) + 0x80))(&DAT_00a3c078,0);
                      (**(code **)(**(int **)(param_1 + 0x26ec0) + 0x80))(&DAT_00a3c0f8,0);
                      (**(code **)(**(int **)(param_1 + 0x26ec4) + 0x80))(&DAT_00a3c178,0);
                      (**(code **)(**(int **)(param_1 + 0x26ec8) + 0x80))(&DAT_00a3c1f8,0);
                      break;
                    default:
                      (**(code **)(**(int **)(param_1 + 0x26ebc) + 0x80))(&DAT_00a3ba78,0);
                      (**(code **)(**(int **)(param_1 + 0x26ec0) + 0x80))(&DAT_00a3baf8,0);
                      (**(code **)(**(int **)(param_1 + 0x26ec4) + 0x80))(&DAT_00a3bb78,0);
                      (**(code **)(**(int **)(param_1 + 0x26ec8) + 0x80))(&DAT_00a3bbf8,0);
                    }
                    FUN_004a218c(local_14,1,1);
                    local_190 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x502);
                    (**(code **)(*local_190 + 0x60))(1);
                    if (*(int *)(*(int *)(param_1 + 0x26e30 + local_14 * 4) + 0x198) == 0) {
                      (**(code **)(**(int **)(param_1 + 0x26ee0) + 0x60))(0);
                    }
                    else if (*(short *)(*(int *)(*(int *)(param_1 + 0x26e30 + local_14 * 4) + 0x198)
                                       + 0x24a) == 0x13) {
                      (**(code **)(**(int **)(param_1 + 0x26ee0) + 0x60))(0);
                    }
                    else {
                      *(uint *)(*(int *)(param_1 + 0x26ee0) + 0xa4) =
                           (uint)*(ushort *)
                                  (*(int *)(*(int *)(param_1 + 0x26e30 + local_14 * 4) + 0x198) +
                                  0x24a);
                      (**(code **)(**(int **)(param_1 + 0x26ee0) + 0x60))(1);
                    }
                    if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x39), iVar3 != 0)) {
                      uVar7 = 0;
                      uVar4 = 0;
                      FUN_00429a6d(0x39);
                      FUN_0042ad2b(uVar4,uVar7);
                    }
                  }
                  else {
                    local_194 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x52d);
                    if ((local_194 == (int *)0x0) || (local_d0 != 0)) {
                      if (local_194 != (int *)0x0) {
                        (**(code **)(*local_194 + 0x60))(1);
                        if (local_d0 == 1) {
                          local_194[0x29] = 0;
                        }
                        if (local_d0 == 2) {
                          local_194[0x29] = 1;
                        }
                        if (local_d0 == 3) {
                          local_194[0x29] = 2;
                        }
                      }
                    }
                    else {
                      (**(code **)(*local_194 + 0x60))(0);
                    }
                  }
                  if (DAT_005d04e0 == 1) {
                    if (*(char *)(DAT_013b71e8 + 0xef4) < '\0') {
                      return 1;
                    }
                    if (*(int *)(param_1 + 0x26e1c) == 1) {
                      return 1;
                    }
                    local_198 = (**(code **)(*DAT_0092e654 + 8))();
                    if ((uint)(local_198 - *(int *)(param_1 + 0x26e70)) < 1000) {
                      return 0;
                    }
                    FUN_0058f078(local_218,&DAT_00a38bf8,
                                 *(int *)(param_1 + 0x26e30 + *(char *)(DAT_013b71e8 + 0xef4) * 4) +
                                 0x168);
                    (**(code **)(**(int **)(param_1 + 0x8c) + 0x8c))(local_218,1,&DAT_00a3a578);
                    (**(code **)(**(int **)(param_1 + 0x8c) + 0x60))(1);
                  }
                  return 1;
                }
              }
              *(undefined4 *)(param_1 + 0x7c) = 0;
              *(undefined1 *)(DAT_013b71e8 + 0xef4) = 0xff;
              local_10 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x502);
              iVar3 = __ftol();
              iVar5 = __ftol();
              if (iVar3 == iVar5) {
                if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x39), iVar3 != 0)) {
                  uVar7 = 0;
                  uVar4 = 0;
                  FUN_00429a6d(0x39);
                  FUN_0042ad2b(uVar4,uVar7);
                }
                uVar4 = (**(code **)(*DAT_0092e654 + 8))();
                *(undefined4 *)(param_1 + 0x26e78) = uVar4;
                FUN_004a24b4(s_UI_SelCamAction3_005b949c);
              }
            }
            else {
              if ((DAT_005b892c == 2) && (iVar3 = FUN_0040c0f0(), iVar3 == 1)) {
                return 1;
              }
              for (local_22c[4] = 0; local_22c[4] < 4; local_22c[4] = local_22c[4] + 1) {
                if (*(int *)(param_1 + 0x26e40 + local_22c[4] * 4) != 0) {
                  *(undefined4 *)(*(int *)(param_1 + 0x26e40 + local_22c[4] * 4) + 0x234) = 0;
                }
              }
              for (local_22c[4] = 0; local_22c[4] < 4; local_22c[4] = local_22c[4] + 1) {
                if (((local_8 != 0) && (*(int *)(param_1 + 0x26e40 + local_22c[4] * 4) != 0)) &&
                   ((*(int *)(param_1 + 0x26e40 + local_22c[4] * 4) == local_8 &&
                    (*(int *)(local_8 + 0x238) == 1)))) {
                  *(undefined4 *)(*(int *)(param_1 + 0x26e40 + local_22c[4] * 4) + 0x234) = 1;
                  *(undefined4 *)(DAT_013b71e8 + 0x1af48) =
                       *(undefined4 *)(param_1 + 0x26e40 + local_22c[4] * 4);
                  local_254[0] = &DAT_005b94b0;
                  local_254[1] = &DAT_005b94b4;
                  local_254[2] = &DAT_005b94b8;
                  local_254[3] = &DAT_005b94bc;
                  local_27c[0] = &DAT_005b94c0;
                  local_27c[1] = &DAT_005b94c4;
                  local_27c[2] = &DAT_005b94c8;
                  local_27c[3] = &DAT_005b94cc;
                  local_264[0] = &DAT_005b94d0;
                  local_264[1] = &DAT_005b94d4;
                  local_264[2] = &DAT_005b94d8;
                  local_264[3] = &DAT_005b94dc;
                  local_244[0] = &DAT_005b94e0;
                  local_244[1] = &DAT_005b94e4;
                  local_244[2] = &DAT_005b94e8;
                  local_244[3] = &DAT_005b94ec;
                  local_22c[0] = 0x3fe66666;
                  local_22c[1] = 0x3fcccccd;
                  local_22c[2] = 0x3f4ccccd;
                  local_22c[3] = 0x3f19999a;
                  local_234 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x61a);
                  local_26c = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x61b);
                  local_268 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x61c);
                  local_230 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x61d);
                  (**(code **)(*local_234 + 0x80))(local_254[local_22c[4]],0);
                  (**(code **)(*local_26c + 0x80))(local_27c[local_22c[4]],0);
                  (**(code **)(*local_268 + 0x80))(local_264[local_22c[4]],0);
                  (**(code **)(*local_230 + 0x80))(local_244[local_22c[4]],0);
                  *(int *)(param_1 + 0x26ef8) = local_22c[local_22c[4]];
                  if (DAT_005b892c == 2) {
                    FUN_004a218c(local_22c[4],1,0);
                    local_2d4[0xc] = (int *)0x42a00000;
                    local_2d4[0xd] = (int *)0x42480000;
                    local_2d4[0xe] = (int *)0x42700000;
                    local_2d4[0xf] = (int *)0x42a00000;
                    local_2d4[4] = (int *)0x42200000;
                    local_2d4[5] = (int *)0x42a00000;
                    local_2d4[6] = (int *)0x42700000;
                    local_2d4[7] = (int *)0x42b40000;
                    local_2d4[0] = (int *)0x428c0000;
                    local_2d4[1] = (int *)0x42480000;
                    local_2d4[2] = (int *)0x42b40000;
                    local_2d4[3] = (int *)0x43020000;
                    local_2d4[8] = (int *)0x42700000;
                    local_2d4[9] = (int *)0x42480000;
                    local_2d4[10] = (int *)0x42480000;
                    local_2d4[0xb] = (int *)0x42700000;
                    local_2d4[0x10] =
                         (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x1622);
                    local_290 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x1623);
                    local_28c = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x1624);
                    local_288 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x1625);
                    local_284 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x1626);
                    local_280 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x1627);
                    if ((-1 < local_22c[4]) && (local_22c[4] < 4)) {
                      *(int **)(local_290 + 0x54) = local_2d4[local_22c[4] + 0xc];
                      *(int **)(local_28c + 0x54) = local_2d4[local_22c[4] + 4];
                      *(int **)(local_288 + 0x54) = local_2d4[local_22c[4]];
                      *(int **)(local_284 + 0x54) = local_2d4[local_22c[4] + 8];
                      piVar6 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x606);
                      (**(code **)(*piVar6 + 0x60))(1);
                    }
                    switch(local_22c[4]) {
                    case 0:
                      (**(code **)(*local_2d4[0x10] + 0x80))(&DAT_00a37df8,0);
                      FUN_004091ab();
                      FUN_0049e2ef(local_280,s_UI_chardesctrans_txt_005b94f0);
                      break;
                    case 1:
                      (**(code **)(*local_2d4[0x10] + 0x80))(&DAT_00a37e78,0);
                      FUN_004091ab();
                      FUN_0049e2ef(local_280,s_UI_chardescfoema_txt_005b9508);
                      break;
                    case 2:
                      (**(code **)(*local_2d4[0x10] + 0x80))(&DAT_00a37ef8,0);
                      FUN_004091ab();
                      FUN_0049e2ef(local_280,s_UI_chardescbeast_txt_005b9520);
                      break;
                    case 3:
                      (**(code **)(*local_2d4[0x10] + 0x80))(&DAT_00a37f78,0);
                      FUN_004091ab();
                      FUN_0049e2ef(local_280,s_UI_chardeschunter_txt_005b9538);
                      break;
                    default:
                      (**(code **)(*local_2d4[0x10] + 0x80))(&DAT_005d04e8,0);
                    }
                    if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x39), iVar3 != 0)) {
                      uVar7 = 0;
                      uVar4 = 0;
                      FUN_00429a6d(0x39);
                      FUN_0042ad2b(uVar4,uVar7);
                    }
                  }
                  else {
                    iVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x606);
                    if (*(float *)(iVar3 + 0x50) == _DAT_005a34a4 * DAT_005b4910) {
                      if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x39), iVar3 != 0)) {
                        uVar7 = 0;
                        uVar4 = 0;
                        FUN_00429a6d(0x39);
                        FUN_0042ad2b(uVar4,uVar7);
                      }
                      uVar4 = (**(code **)(*DAT_0092e654 + 8))();
                      *(undefined4 *)(param_1 + 0x26e7c) = uVar4;
                      FUN_004a24b4(s_UI_CreCamAction2_005b9550);
                    }
                  }
                  switch(local_22c[4]) {
                  case 0:
                    FUN_0051dba4(5,0);
                    break;
                  case 1:
                    FUN_0051dba4(9,0);
                    break;
                  case 2:
                    FUN_0051dba4(8,0);
                    break;
                  case 3:
                    FUN_0051dba4(8,0);
                    break;
                  default:
                    FUN_0051dba4(5,0);
                  }
                  if (((local_22c[4] == 3) && (DAT_005ccf98 != 0)) &&
                     (iVar3 = FUN_00429a6d(0x86), iVar3 != 0)) {
                    uVar7 = 0;
                    uVar4 = 0;
                    FUN_00429a6d(0x86);
                    FUN_0042ad2b(uVar4,uVar7);
                  }
                  return 1;
                }
              }
              *(undefined4 *)(param_1 + 0x7c) = 0;
              if ((DAT_005b892c != 2) &&
                 (iVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x606),
                 *(float *)(iVar3 + 0x50) == _DAT_005a428c * DAT_005b4910)) {
                if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x39), iVar3 != 0)) {
                  uVar7 = 0;
                  uVar4 = 0;
                  FUN_00429a6d(0x39);
                  FUN_0042ad2b(uVar4,uVar7);
                }
                uVar4 = (**(code **)(*DAT_0092e654 + 8))();
                *(undefined4 *)(param_1 + 0x26e80) = uVar4;
                FUN_004a24b4(s_UI_CreCamAction3_005b9564);
              }
            }
            uVar4 = 1;
          }
          else {
            uVar4 = 0;
          }
        }
        else {
          uVar4 = 1;
        }
      }
    }
  }
  return uVar4;
}

