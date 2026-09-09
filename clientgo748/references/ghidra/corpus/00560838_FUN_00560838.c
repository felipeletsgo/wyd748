// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00560838 | Name: FUN_00560838


int FUN_00560838(void)

{
  byte bVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  uint uVar4;
  int *piVar5;
  uint uVar6;
  undefined4 *puVar7;
  int iVar8;
  uint uVar9;
  int *piVar10;
  int unaff_EBP;
  int iVar11;
  undefined4 *puVar12;
  
  FUN_00592900();
  iVar8 = *(int *)(unaff_EBP + 0x3c);
  FUN_00564a94();
  *(undefined4 *)(unaff_EBP + -4) = 0;
  *(undefined4 *)(unaff_EBP + -0x18) = 0;
  *(undefined4 *)(unaff_EBP + -0x14) = 0;
  *(undefined4 *)(unaff_EBP + -0x1c) = 0;
  *(undefined4 *)(unaff_EBP + -0x20) = 0;
  if ((((*(int *)(unaff_EBP + 8) == 0) || (*(int *)(unaff_EBP + 0xc) == 0)) ||
      (*(int *)(unaff_EBP + 0x10) == 0)) || (*(int *)(unaff_EBP + 0x48) == 0)) {
    iVar11 = -0x7789f794;
    goto LAB_00560e5d;
  }
  if ((iVar8 == 0) && (*(int *)(unaff_EBP + 0x44) == -1)) {
    iVar8 = unaff_EBP + -0xcc;
  }
  iVar11 = FUN_00566e85(*(undefined4 *)(unaff_EBP + 0xc),*(undefined4 *)(unaff_EBP + 0x10),iVar8,1);
  if (iVar11 < 0) goto LAB_00560e5d;
  if (*(int *)(unaff_EBP + 0x44) == -1) {
    *(undefined4 *)(unaff_EBP + 0x44) = *(undefined4 *)(iVar8 + 0x14);
  }
  iVar8 = *(int *)(unaff_EBP + -0x40);
  *(undefined4 *)(unaff_EBP + -0x24) = 1;
  for (; iVar8 != 0; iVar8 = *(int *)(iVar8 + 0x4c)) {
    *(int *)(unaff_EBP + -0x24) = *(int *)(unaff_EBP + -0x24) + 1;
  }
  *(undefined4 *)(unaff_EBP + -0x28) = 1;
  if (*(int *)(unaff_EBP + 0x44) == 5) {
    iVar8 = *(int *)(unaff_EBP + -0x3c);
    if (iVar8 != 0) {
      do {
        iVar8 = *(int *)(iVar8 + 0x50);
        *(int *)(unaff_EBP + -0x28) = *(int *)(unaff_EBP + -0x28) + 1;
      } while (iVar8 != 0);
      if (*(int *)(unaff_EBP + -0x28) == 6) goto LAB_00560905;
    }
    iVar11 = -0x7fffbffb;
    goto LAB_00560e5d;
  }
LAB_00560905:
  if ((*(int *)(unaff_EBP + 0x14) == 0) || (*(int *)(unaff_EBP + 0x14) == -1)) {
    *(undefined4 *)(unaff_EBP + 0x14) = *(undefined4 *)(unaff_EBP + -0x80);
  }
  if ((*(int *)(unaff_EBP + 0x18) == 0) || (*(int *)(unaff_EBP + 0x18) == -1)) {
    *(undefined4 *)(unaff_EBP + 0x18) = *(undefined4 *)(unaff_EBP + -0x7c);
  }
  if ((*(int *)(unaff_EBP + 0x1c) == 0) || (*(int *)(unaff_EBP + 0x1c) == -1)) {
    *(undefined4 *)(unaff_EBP + 0x1c) = *(undefined4 *)(unaff_EBP + -0x78);
  }
  if (*(int *)(unaff_EBP + 0x30) == -1) {
    *(undefined4 *)(unaff_EBP + 0x30) = 0x80004;
  }
  if (*(int *)(unaff_EBP + 0x34) == -1) {
    *(undefined4 *)(unaff_EBP + 0x34) = 5;
  }
  if (*(int *)(unaff_EBP + 0x44) == 5) {
    *(uint *)(unaff_EBP + 0x30) = *(uint *)(unaff_EBP + 0x30) | 0x70000;
    *(uint *)(unaff_EBP + 0x34) = *(uint *)(unaff_EBP + 0x34) | 0x70000;
  }
  if (((*(char *)(unaff_EBP + 0x30) == '\x01') ||
      (uVar6 = *(uint *)(unaff_EBP + 0x34) & 0xff, uVar6 == 2)) ||
     (*(undefined4 *)(unaff_EBP + -0x30) = 0, uVar6 == 5)) {
    *(undefined4 *)(unaff_EBP + -0x30) = 1;
  }
  if (*(int *)(unaff_EBP + 0x28) == 0) {
    iVar8 = *(int *)(unaff_EBP + -0x8c);
    if (*(int *)(unaff_EBP + 0x38) != 0) {
      puVar7 = (undefined4 *)FUN_0055fab2(iVar8);
      iVar11 = puVar7[1];
      if ((((iVar11 == 0) || (iVar11 == 1)) || (iVar11 == 2)) && (puVar7[4] == 0)) {
        uVar2 = *(undefined4 *)(unaff_EBP + 0x44);
        uVar3 = *(undefined4 *)(unaff_EBP + 0x24);
        puVar12 = (undefined4 *)(unaff_EBP + -0xb0);
        for (iVar8 = 9; iVar8 != 0; iVar8 = iVar8 + -1) {
          *puVar12 = *puVar7;
          puVar7 = puVar7 + 1;
          puVar12 = puVar12 + 1;
        }
        *(undefined4 *)(unaff_EBP + -0xb0) = 0;
        *(undefined4 *)(unaff_EBP + -0xa0) = 1;
        iVar8 = FUN_0055fbb9(0,uVar3,uVar2,unaff_EBP + -0xb0);
        if (iVar8 == 0) {
          iVar8 = *(int *)(unaff_EBP + -0x8c);
        }
      }
    }
    iVar8 = FUN_0055fc9b(iVar8);
    *(int *)(unaff_EBP + 0x28) = iVar8;
    if ((*(int *)(unaff_EBP + 0x2c) != 3) && (iVar8 == 0x14)) {
      *(undefined4 *)(unaff_EBP + 0x28) = 0x16;
    }
  }
  iVar8 = *(int *)(unaff_EBP + -0x84);
  if (iVar8 == 0) {
    puVar7 = (undefined4 *)(unaff_EBP + -0x4cc);
    for (iVar8 = 0x100; iVar8 != 0; iVar8 = iVar8 + -1) {
      *puVar7 = 0xffffffff;
      puVar7 = puVar7 + 1;
    }
  }
  else {
    uVar6 = *(uint *)(unaff_EBP + 0x38);
    bVar1 = *(byte *)(unaff_EBP + 0x3a);
    uVar9 = 0;
    do {
      uVar4 = *(uint *)(iVar8 + uVar9 * 4);
      *(uint *)(unaff_EBP + -0x4cc + uVar9 * 4) =
           -(uint)(uVar4 != ((uint)bVar1 | (uVar6 & 0xff) << 0x10 | uVar6 & 0xff00ff00)) & uVar4;
      uVar9 = uVar9 + 1;
    } while (uVar9 < 0x100);
    *(undefined4 *)(unaff_EBP + 0x38) = 0;
  }
  if (*(undefined4 **)(unaff_EBP + 0x40) == (undefined4 *)0x0) {
    if (*(int *)(unaff_EBP + 0x28) == 0x28) {
LAB_00560aab:
      *(undefined4 *)(unaff_EBP + 0x28) = 0x15;
    }
    else if (*(int *)(unaff_EBP + 0x28) == 0x29) {
      *(undefined4 *)(unaff_EBP + 0x28) = 0x16;
      uVar6 = 0;
      do {
        if (*(char *)(unaff_EBP + -0x4c9 + uVar6 * 4) != -1) goto LAB_00560aab;
        uVar6 = uVar6 + 1;
      } while (uVar6 < 0x100);
    }
  }
  else {
    puVar7 = (undefined4 *)(unaff_EBP + -0x4cc);
    puVar12 = *(undefined4 **)(unaff_EBP + 0x40);
    for (iVar8 = 0x100; iVar8 != 0; iVar8 = iVar8 + -1) {
      *puVar12 = *puVar7;
      puVar7 = puVar7 + 1;
      puVar12 = puVar12 + 1;
    }
  }
  if ((*(int *)(unaff_EBP + 0x2c) != 0) ||
     (*(undefined4 *)(unaff_EBP + -0x34) = 1, (*(byte *)(unaff_EBP + 0x25) & 2) != 0)) {
    *(undefined4 *)(unaff_EBP + -0x34) = 0;
  }
  iVar11 = FUN_0055ff4e(*(undefined4 *)(unaff_EBP + 8),unaff_EBP + 0x14,unaff_EBP + 0x18,
                        unaff_EBP + 0x1c,unaff_EBP + 0x20,unaff_EBP + 0x28,
                        *(undefined4 *)(unaff_EBP + 0x2c),*(undefined4 *)(unaff_EBP + 0x44));
  if (-1 < iVar11) {
    iVar8 = *(int *)(unaff_EBP + 0x44);
    uVar2 = *(undefined4 *)(unaff_EBP + 0x28);
    uVar3 = *(undefined4 *)(unaff_EBP + 0x14);
    if (iVar8 == 3) {
      iVar11 = (**(code **)(**(int **)(unaff_EBP + 8) + 0x50))
                         (*(int **)(unaff_EBP + 8),uVar3,*(undefined4 *)(unaff_EBP + 0x18),
                          *(undefined4 *)(unaff_EBP + 0x20),*(undefined4 *)(unaff_EBP + 0x24),uVar2,
                          *(undefined4 *)(unaff_EBP + 0x2c),unaff_EBP + -0x1c);
    }
    else if (iVar8 == 4) {
      iVar11 = (**(code **)(**(int **)(unaff_EBP + 8) + 0x54))
                         (*(int **)(unaff_EBP + 8),uVar3,*(undefined4 *)(unaff_EBP + 0x18),
                          *(undefined4 *)(unaff_EBP + 0x1c),*(undefined4 *)(unaff_EBP + 0x20),
                          *(undefined4 *)(unaff_EBP + 0x24),uVar2,*(undefined4 *)(unaff_EBP + 0x2c),
                          unaff_EBP + -0x1c);
    }
    else if (iVar8 == 5) {
      iVar11 = (**(code **)(**(int **)(unaff_EBP + 8) + 0x58))
                         (*(int **)(unaff_EBP + 8),uVar3,*(undefined4 *)(unaff_EBP + 0x20),
                          *(undefined4 *)(unaff_EBP + 0x24),uVar2,*(undefined4 *)(unaff_EBP + 0x2c),
                          unaff_EBP + -0x1c);
    }
    if (-1 < iVar11) {
      if (*(int *)(unaff_EBP + -0x34) == 0) {
        piVar10 = *(int **)(unaff_EBP + -0x1c);
      }
      else {
        iVar8 = *(int *)(unaff_EBP + 0x44);
        if (iVar8 == 3) {
          iVar11 = (**(code **)(**(int **)(unaff_EBP + 8) + 0x50))
                             (*(int **)(unaff_EBP + 8),uVar3,*(undefined4 *)(unaff_EBP + 0x18),
                              *(undefined4 *)(unaff_EBP + 0x20),0,uVar2,2,unaff_EBP + -0x20);
        }
        else if (iVar8 == 4) {
          iVar11 = (**(code **)(**(int **)(unaff_EBP + 8) + 0x54))
                             (*(int **)(unaff_EBP + 8),uVar3,*(undefined4 *)(unaff_EBP + 0x18),
                              *(undefined4 *)(unaff_EBP + 0x1c),*(undefined4 *)(unaff_EBP + 0x20),0,
                              uVar2,2,unaff_EBP + -0x20);
        }
        else if (iVar8 == 5) {
          iVar11 = (**(code **)(**(int **)(unaff_EBP + 8) + 0x58))
                             (*(int **)(unaff_EBP + 8),uVar3,*(undefined4 *)(unaff_EBP + 0x20),0,
                              uVar2,2,unaff_EBP + -0x20);
        }
        if (iVar11 < 0) goto LAB_00560e1b;
        piVar10 = *(int **)(unaff_EBP + -0x20);
      }
      *(undefined4 *)(unaff_EBP + -0x2c) = 0;
      puVar7 = (undefined4 *)(unaff_EBP + -0x8c);
      *(undefined4 **)(unaff_EBP + -0x38) = puVar7;
      if (*(int *)(unaff_EBP + -0x28) != 0) {
        do {
          *(undefined4 *)(unaff_EBP + -0x10) = 0;
          if (*(int *)(unaff_EBP + -0x24) != 0) {
            while (*(uint *)(unaff_EBP + -0x10) < *(uint *)(unaff_EBP + 0x20)) {
              iVar8 = *(int *)(unaff_EBP + 0x44);
              if (iVar8 == 3) {
                iVar8 = unaff_EBP + -0x14;
LAB_00560c3a:
                iVar11 = (**(code **)(*piVar10 + 0x3c))
                                   (piVar10,*(undefined4 *)(unaff_EBP + -0x10),iVar8);
              }
              else {
                if (iVar8 == 4) {
                  iVar8 = unaff_EBP + -0x18;
                  goto LAB_00560c3a;
                }
                if (iVar8 == 5) {
                  iVar11 = (**(code **)(*piVar10 + 0x3c))
                                     (piVar10,*(undefined4 *)(unaff_EBP + -0x2c),
                                      *(undefined4 *)(unaff_EBP + -0x10),unaff_EBP + -0x14);
                }
              }
              if (iVar11 < 0) goto LAB_00560e1b;
              iVar8 = *(int *)(unaff_EBP + 0x44);
              if (iVar8 == 3) {
LAB_00560c5c:
                iVar11 = FUN_0055fccc(*(undefined4 *)(unaff_EBP + -0x14),
                                      *(undefined4 *)(unaff_EBP + 0x40),0,puVar7[1],*puVar7,
                                      puVar7[0xc],unaff_EBP + -0x4cc,puVar7 + 6,
                                      *(undefined4 *)(unaff_EBP + 0x30),
                                      *(undefined4 *)(unaff_EBP + 0x38));
              }
              else if (iVar8 == 4) {
                iVar11 = FUN_0055fe7b(*(undefined4 *)(unaff_EBP + -0x18),
                                      *(undefined4 *)(unaff_EBP + 0x40),0,puVar7[1],*puVar7,
                                      puVar7[0xc],puVar7[0xd],unaff_EBP + -0x4cc,puVar7 + 6,
                                      *(undefined4 *)(unaff_EBP + 0x30),
                                      *(undefined4 *)(unaff_EBP + 0x38));
              }
              else if (iVar8 == 5) goto LAB_00560c5c;
              if (iVar11 < 0) goto LAB_00560e1b;
              piVar5 = *(int **)(unaff_EBP + -0x18);
              if (piVar5 != (int *)0x0) {
                (**(code **)(*piVar5 + 8))(piVar5);
                *(undefined4 *)(unaff_EBP + -0x18) = 0;
              }
              piVar5 = *(int **)(unaff_EBP + -0x14);
              if (piVar5 != (int *)0x0) {
                (**(code **)(*piVar5 + 8))(piVar5);
                *(undefined4 *)(unaff_EBP + -0x14) = 0;
              }
              *(int *)(unaff_EBP + -0x10) = *(int *)(unaff_EBP + -0x10) + 1;
              if (*(uint *)(unaff_EBP + -0x24) <= *(uint *)(unaff_EBP + -0x10)) break;
              puVar7 = (undefined4 *)puVar7[0x13];
            }
          }
          if ((*(int *)(unaff_EBP + -0x30) == 0) &&
             (*(uint *)(unaff_EBP + -0x10) < *(uint *)(unaff_EBP + 0x20))) {
            do {
              iVar8 = *(int *)(unaff_EBP + 0x44);
              if (iVar8 == 3) {
                iVar8 = unaff_EBP + -0x14;
LAB_00560d2a:
                iVar11 = (**(code **)(*piVar10 + 0x3c))
                                   (piVar10,*(undefined4 *)(unaff_EBP + -0x10),iVar8);
              }
              else {
                if (iVar8 == 4) {
                  iVar8 = unaff_EBP + -0x18;
                  goto LAB_00560d2a;
                }
                if (iVar8 == 5) {
                  iVar11 = (**(code **)(*piVar10 + 0x3c))
                                     (piVar10,*(undefined4 *)(unaff_EBP + -0x2c),
                                      *(undefined4 *)(unaff_EBP + -0x10),unaff_EBP + -0x14);
                }
              }
              if (iVar11 < 0) goto LAB_00560e1b;
              iVar8 = *(int *)(unaff_EBP + 0x44);
              if (iVar8 == 3) {
LAB_00560d50:
                iVar11 = FUN_0055fccc(*(undefined4 *)(unaff_EBP + -0x14),
                                      *(undefined4 *)(unaff_EBP + 0x40),0,puVar7[1],*puVar7,
                                      puVar7[0xc],unaff_EBP + -0x4cc,puVar7 + 6,
                                      *(undefined4 *)(unaff_EBP + 0x30),
                                      *(undefined4 *)(unaff_EBP + 0x38));
              }
              else if (iVar8 == 4) {
                iVar11 = FUN_0055fe7b(*(undefined4 *)(unaff_EBP + -0x18),
                                      *(undefined4 *)(unaff_EBP + 0x40),0,puVar7[1],*puVar7,
                                      puVar7[0xc],puVar7[0xd],unaff_EBP + -0x4cc,puVar7 + 6,
                                      *(undefined4 *)(unaff_EBP + 0x30),
                                      *(undefined4 *)(unaff_EBP + 0x38));
              }
              else if (iVar8 == 5) goto LAB_00560d50;
              if (iVar11 < 0) goto LAB_00560e1b;
              piVar5 = *(int **)(unaff_EBP + -0x18);
              if (piVar5 != (int *)0x0) {
                (**(code **)(*piVar5 + 8))(piVar5);
                *(undefined4 *)(unaff_EBP + -0x18) = 0;
              }
              piVar5 = *(int **)(unaff_EBP + -0x14);
              if (piVar5 != (int *)0x0) {
                (**(code **)(*piVar5 + 8))(piVar5);
                *(undefined4 *)(unaff_EBP + -0x14) = 0;
              }
              *(int *)(unaff_EBP + -0x10) = *(int *)(unaff_EBP + -0x10) + 1;
            } while (*(uint *)(unaff_EBP + -0x10) < *(uint *)(unaff_EBP + 0x20));
          }
          *(int *)(unaff_EBP + -0x2c) = *(int *)(unaff_EBP + -0x2c) + 1;
          puVar7 = *(undefined4 **)(*(int *)(unaff_EBP + -0x38) + 0x50);
          *(undefined4 **)(unaff_EBP + -0x38) = puVar7;
        } while (*(uint *)(unaff_EBP + -0x2c) < *(uint *)(unaff_EBP + -0x28));
      }
      if ((((*(int *)(unaff_EBP + -0x30) == 0) ||
           (*(uint *)(unaff_EBP + 0x20) <= *(uint *)(unaff_EBP + -0x24))) ||
          (iVar11 = FUN_00560554(piVar10,unaff_EBP + -0x4cc,*(int *)(unaff_EBP + -0x24) + -1,
                                 *(undefined4 *)(unaff_EBP + 0x34)), -1 < iVar11)) &&
         ((*(int *)(unaff_EBP + -0x34) == 0 ||
          (iVar11 = (**(code **)(**(int **)(unaff_EBP + 8) + 0x74))
                              (*(int **)(unaff_EBP + 8),*(undefined4 *)(unaff_EBP + -0x20),
                               *(undefined4 *)(unaff_EBP + -0x1c)), -1 < iVar11)))) {
        uVar2 = *(undefined4 *)(unaff_EBP + -0x1c);
        *(undefined4 *)(unaff_EBP + -0x1c) = 0;
        **(undefined4 **)(unaff_EBP + 0x48) = uVar2;
        iVar11 = 0;
      }
    }
  }
LAB_00560e1b:
  piVar10 = *(int **)(unaff_EBP + -0x18);
  if (piVar10 != (int *)0x0) {
    (**(code **)(*piVar10 + 8))(piVar10);
    *(undefined4 *)(unaff_EBP + -0x18) = 0;
  }
  piVar10 = *(int **)(unaff_EBP + -0x14);
  if (piVar10 != (int *)0x0) {
    (**(code **)(*piVar10 + 8))(piVar10);
    *(undefined4 *)(unaff_EBP + -0x14) = 0;
  }
  piVar10 = *(int **)(unaff_EBP + -0x1c);
  if (piVar10 != (int *)0x0) {
    (**(code **)(*piVar10 + 8))(piVar10);
    *(undefined4 *)(unaff_EBP + -0x1c) = 0;
  }
  piVar10 = *(int **)(unaff_EBP + -0x20);
  if (piVar10 != (int *)0x0) {
    (**(code **)(*piVar10 + 8))(piVar10);
    *(undefined4 *)(unaff_EBP + -0x20) = 0;
  }
LAB_00560e5d:
  *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
  FUN_00564aad();
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return iVar11;
}

