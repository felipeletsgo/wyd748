// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005673a1 | Name: FUN_005673a1


undefined4 FUN_005673a1(void)

{
  int *piVar1;
  float fVar2;
  int iVar3;
  int *piVar4;
  void *pvVar5;
  uint uVar6;
  int *extraout_ECX;
  int *piVar7;
  int *piVar8;
  float *pfVar9;
  int unaff_EBP;
  undefined4 uVar10;
  undefined4 *puVar11;
  float *pfVar12;
  
  FUN_00592900();
  *(undefined4 *)(unaff_EBP + -0x2c) = 0;
  *(undefined4 *)(unaff_EBP + -0x28) = 0;
  *(undefined4 *)(unaff_EBP + -0x58) = 0;
  *(undefined4 *)(unaff_EBP + -0x20) = 0;
  *(undefined4 *)(unaff_EBP + -0x5c) = 0;
  uVar6 = extraout_ECX[2];
  *(uint *)(unaff_EBP + -0x54) = ~(uVar6 >> 0x12) & 1;
  iVar3 = FUN_00567098(*(undefined4 *)(*extraout_ECX + 0x1058),
                       *(undefined4 *)(extraout_ECX[1] + 0x1058),~(uVar6 >> 0x10) & 1);
  *(int *)(unaff_EBP + -0x44) = iVar3;
  if (iVar3 != 0) {
    iVar3 = FUN_00567098(*(undefined4 *)(*extraout_ECX + 0x105c),
                         *(undefined4 *)(extraout_ECX[1] + 0x105c),~(uVar6 >> 0x11) & 1);
    *(int *)(unaff_EBP + -0x2c) = iVar3;
    if (iVar3 != 0) {
      iVar3 = FUN_00567098(*(undefined4 *)(*extraout_ECX + 0x1060),
                           *(undefined4 *)(extraout_ECX[1] + 0x1060),
                           *(undefined4 *)(unaff_EBP + -0x54));
      *(int *)(unaff_EBP + -0x28) = iVar3;
      if (iVar3 != 0) {
        *(int *)(unaff_EBP + -0x4c) =
             **(int **)(unaff_EBP + -0x44) + (int)*(int **)(unaff_EBP + -0x44);
        *(int *)(unaff_EBP + -0x50) =
             **(int **)(unaff_EBP + -0x2c) + (int)*(int **)(unaff_EBP + -0x2c);
        *(int *)(unaff_EBP + -0x24) =
             **(int **)(unaff_EBP + -0x28) + (int)*(int **)(unaff_EBP + -0x28);
        iVar3 = *(int *)(extraout_ECX[1] + 0x1060);
        piVar4 = operator_new(iVar3 * 0xc + 4);
        *(int **)(unaff_EBP + -0x54) = piVar4;
        *(undefined4 *)(unaff_EBP + -4) = 0;
        if (piVar4 == (int *)0x0) {
          *(undefined4 *)(unaff_EBP + -0x14) = 0;
          iVar3 = *(int *)(unaff_EBP + -0x14);
        }
        else {
          *piVar4 = iVar3;
          *(int **)(unaff_EBP + -0x58) = piVar4 + 1;
          FUN_00590340(piVar4 + 1,0xc,iVar3,&LAB_00567082,&LAB_0056708f);
          iVar3 = *(int *)(unaff_EBP + -0x58);
          *(int *)(unaff_EBP + -0x14) = iVar3;
        }
        *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
        *(int *)(unaff_EBP + -0x58) = iVar3;
        if (iVar3 != 0) {
          iVar3 = *(int *)(*extraout_ECX + 0x1058);
          pvVar5 = operator_new(iVar3 << 4);
          *(void **)(unaff_EBP + -0x5c) = pvVar5;
          *(undefined4 *)(unaff_EBP + -4) = 1;
          if (pvVar5 == (void *)0x0) {
            *(undefined4 *)(unaff_EBP + -0x1c) = 0;
            iVar3 = *(int *)(unaff_EBP + -0x1c);
          }
          else {
            _vector_constructor_iterator_(pvVar5,0x10,iVar3,FUN_00430f10);
            iVar3 = *(int *)(unaff_EBP + -0x5c);
            *(int *)(unaff_EBP + -0x1c) = iVar3;
          }
          *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
          *(int *)(unaff_EBP + -0x5c) = iVar3;
          if (iVar3 != 0) {
            piVar4 = (int *)(*(int *)(unaff_EBP + -0x28) + 4);
            piVar8 = piVar4;
            if (piVar4 < *(int **)(unaff_EBP + -0x24)) {
              do {
                piVar7 = (int *)(*piVar8 + (int)piVar8);
                for (piVar8 = piVar8 + 1; piVar8 < piVar7; piVar8 = piVar8 + 2) {
                  piVar1 = (int *)(*(int *)(unaff_EBP + -0x14) + 8 + *piVar8 * 0xc);
                  *piVar1 = *piVar1 + 1;
                }
                piVar8 = piVar7;
              } while (piVar7 < *(int **)(unaff_EBP + -0x24));
            }
            *(undefined4 *)(unaff_EBP + -0x40) = 0;
            if (piVar4 < *(int **)(unaff_EBP + -0x24)) {
              do {
                piVar8 = (int *)(*piVar4 + (int)piVar4);
                piVar7 = piVar4 + 1;
                *(int **)(unaff_EBP + -0x18) = piVar8;
                *(int **)(unaff_EBP + -0x54) = piVar7;
                *(int **)(unaff_EBP + -0x10) = piVar7;
                piVar4 = piVar8;
                if (piVar7 < piVar8) {
                  do {
                    piVar4 = (int *)(*(int *)(unaff_EBP + -0x14) +
                                    **(int **)(unaff_EBP + -0x10) * 0xc);
                    if (*piVar4 == 0) {
                      piVar7 = *(int **)(unaff_EBP + -0x20);
                      if (piVar7 == (int *)0x0) {
                        iVar3 = *(int *)(extraout_ECX[1] + 0x105c) *
                                *(int *)(extraout_ECX[1] + 0x1058);
                        pvVar5 = operator_new(iVar3 * 0x10);
                        *(void **)(unaff_EBP + -0x48) = pvVar5;
                        *(undefined4 *)(unaff_EBP + -4) = 2;
                        if (pvVar5 == (void *)0x0) {
                          iVar3 = 0;
                        }
                        else {
                          _vector_constructor_iterator_(pvVar5,0x10,iVar3,FUN_00430f10);
                          iVar3 = *(int *)(unaff_EBP + -0x48);
                        }
                        *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
                        *piVar4 = iVar3;
                        if (iVar3 == 0) goto LAB_005677c0;
                        piVar8 = *(int **)(unaff_EBP + -0x18);
                      }
                      else {
                        *piVar4 = *piVar7;
                        *piVar7 = 0;
                        *(int *)(unaff_EBP + -0x20) = piVar7[1];
                      }
                      puVar11 = (undefined4 *)*piVar4;
                      for (iVar3 = (*(int *)(extraout_ECX[1] + 0x105c) *
                                    *(int *)(extraout_ECX[1] + 0x1058) & 0xfffffffU) << 2;
                          iVar3 != 0; iVar3 = iVar3 + -1) {
                        *puVar11 = 0;
                        puVar11 = puVar11 + 1;
                      }
                      for (iVar3 = 0; iVar3 != 0; iVar3 = iVar3 + -1) {
                        *(undefined1 *)puVar11 = 0;
                        puVar11 = (undefined4 *)((int)puVar11 + 1);
                      }
                    }
                    *(int *)(unaff_EBP + -0x10) = *(int *)(unaff_EBP + -0x10) + 8;
                    piVar4 = piVar8;
                  } while (*(int **)(unaff_EBP + -0x10) < piVar8);
                }
                *(undefined4 *)(unaff_EBP + -0x38) = 0;
                uVar6 = *(int *)(unaff_EBP + -0x2c) + 4;
                *(uint *)(unaff_EBP + -0x3c) = uVar6;
                if (uVar6 < *(uint *)(unaff_EBP + -0x50)) {
                  do {
                    piVar4 = (int *)(**(int **)(unaff_EBP + -0x3c) +
                                    (int)*(int **)(unaff_EBP + -0x3c));
                    iVar3 = *(int *)*extraout_ECX;
                    *(int **)(unaff_EBP + -0x48) = piVar4;
                    (**(code **)(iVar3 + 4))
                              (*(undefined4 *)(unaff_EBP + -0x38),*(undefined4 *)(unaff_EBP + -0x40)
                               ,*(undefined4 *)(unaff_EBP + -0x1c));
                    uVar6 = *(int *)(unaff_EBP + -0x44) + 4;
                    *(uint *)(unaff_EBP + -0x34) = uVar6;
                    if (uVar6 < *(uint *)(unaff_EBP + -0x4c)) {
                      pfVar9 = (float *)(*(int *)(unaff_EBP + -0x1c) + 8);
                      do {
                        uVar6 = **(int **)(unaff_EBP + -0x34) + (int)*(int **)(unaff_EBP + -0x34);
                        piVar8 = *(int **)(unaff_EBP + -0x54);
                        *(uint *)(unaff_EBP + -0x30) = uVar6;
                        for (; *(int **)(unaff_EBP + -0x10) = piVar8,
                            piVar8 < *(int **)(unaff_EBP + -0x18); piVar8 = piVar8 + 2) {
                          piVar7 = (int *)(*(int *)(unaff_EBP + -0x3c) + 4);
                          if (piVar7 < piVar4) {
                            do {
                              iVar3 = *(int *)(extraout_ECX[1] + 0x1058) * *piVar7 * 0x10 +
                                      *(int *)(*(int *)(unaff_EBP + -0x14) + *piVar8 * 0xc);
                              for (piVar4 = (int *)(*(int *)(unaff_EBP + -0x34) + 4);
                                  piVar4 < *(int **)(unaff_EBP + -0x30); piVar4 = piVar4 + 2) {
                                fVar2 = (float)piVar4[1] * (float)piVar7[1] *
                                        *(float *)(*(int *)(unaff_EBP + -0x10) + 4);
                                pfVar12 = (float *)(*piVar4 * 0x10 + iVar3);
                                *pfVar12 = fVar2 * pfVar9[-2] + *pfVar12;
                                pfVar12 = (float *)(*piVar4 * 0x10 + 4 + iVar3);
                                *pfVar12 = fVar2 * pfVar9[-1] + *pfVar12;
                                pfVar12 = (float *)(*piVar4 * 0x10 + 8 + iVar3);
                                *pfVar12 = fVar2 * *pfVar9 + *pfVar12;
                                pfVar12 = (float *)(*piVar4 * 0x10 + 0xc + iVar3);
                                *pfVar12 = fVar2 * pfVar9[1] + *pfVar12;
                              }
                              piVar8 = *(int **)(unaff_EBP + -0x10);
                              piVar7 = piVar7 + 2;
                            } while (piVar7 < *(int **)(unaff_EBP + -0x48));
                            piVar4 = *(int **)(unaff_EBP + -0x48);
                            uVar6 = *(uint *)(unaff_EBP + -0x30);
                          }
                        }
                        pfVar9 = pfVar9 + 4;
                        *(uint *)(unaff_EBP + -0x34) = uVar6;
                      } while (uVar6 < *(uint *)(unaff_EBP + -0x4c));
                    }
                    *(int *)(unaff_EBP + -0x38) = *(int *)(unaff_EBP + -0x38) + 1;
                    *(int **)(unaff_EBP + -0x3c) = piVar4;
                  } while (piVar4 < *(int **)(unaff_EBP + -0x50));
                  piVar4 = *(int **)(unaff_EBP + -0x18);
                }
                for (piVar8 = *(int **)(unaff_EBP + -0x54); piVar8 < piVar4; piVar8 = piVar8 + 2) {
                  piVar1 = (int *)(*(int *)(unaff_EBP + -0x14) + *piVar8 * 0xc);
                  piVar7 = piVar1 + 2;
                  *piVar7 = *piVar7 + -1;
                  if (*piVar7 == 0) {
                    piVar7 = (int *)extraout_ECX[1];
                    *(undefined4 *)(unaff_EBP + -0x10) = 0;
                    if (piVar7[0x417] != 0) {
                      do {
                        (**(code **)(*piVar7 + 8))
                                  (*(undefined4 *)(unaff_EBP + -0x10),*piVar8,
                                   piVar7[0x416] * *(int *)(unaff_EBP + -0x10) * 0x10 + *piVar1);
                        *(int *)(unaff_EBP + -0x10) = *(int *)(unaff_EBP + -0x10) + 1;
                        piVar7 = (int *)extraout_ECX[1];
                      } while (*(uint *)(unaff_EBP + -0x10) < (uint)piVar7[0x417]);
                      piVar4 = *(int **)(unaff_EBP + -0x18);
                    }
                    piVar1[1] = *(int *)(unaff_EBP + -0x20);
                    *(int **)(unaff_EBP + -0x20) = piVar1;
                  }
                }
                *(int *)(unaff_EBP + -0x40) = *(int *)(unaff_EBP + -0x40) + 1;
              } while (piVar4 < *(int **)(unaff_EBP + -0x24));
            }
            uVar10 = 0;
            goto LAB_005677cc;
          }
        }
LAB_005677c0:
        uVar10 = 0x8007000e;
        goto LAB_005677cc;
      }
    }
  }
  uVar10 = 0x80004005;
LAB_005677cc:
  if (*(int *)(unaff_EBP + -0x58) != 0) {
    FUN_00567355(3);
  }
  FUN_0058ea3a(*(undefined4 *)(unaff_EBP + -0x28));
  FUN_0058ea3a(*(undefined4 *)(unaff_EBP + -0x2c));
  FUN_0058ea3a(*(undefined4 *)(unaff_EBP + -0x44));
  FUN_0058ea3a(*(undefined4 *)(unaff_EBP + -0x5c));
  FUN_0058ea3a(0);
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return uVar10;
}

