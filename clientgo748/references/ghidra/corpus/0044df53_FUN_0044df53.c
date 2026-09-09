// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044df53 | Name: FUN_0044df53


void __fastcall FUN_0044df53(int *param_1)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int *piVar9;
  int iVar10;
  int *piVar11;
  int *piVar12;
  int iVar13;
  int iVar14;
  int iVar15;
  int iVar16;
  int *piVar17;
  int iVar18;
  
  iVar3 = param_1[0x9f1f];
  iVar7 = param_1[0x9ecd];
  iVar18 = param_1[0x9f21];
  iVar15 = param_1[0x9f63];
  iVar8 = param_1[0xa17c];
  iVar16 = param_1[0x9ece];
  iVar13 = param_1[0xa175];
  iVar14 = param_1[0xa176];
  iVar10 = param_1[0x9f66];
  piVar9 = (int *)param_1[0x9f1e];
  piVar11 = (int *)param_1[0x9f1d];
  piVar1 = (int *)param_1[0x9f1c];
  iVar6 = param_1[0x9ef9];
  piVar12 = (int *)param_1[0xa2e4];
  piVar17 = (int *)param_1[0xa304];
  if (DAT_013b736c == 1) {
    FUN_0055ee59(0);
  }
  else if ((iVar16 == 0) ||
          (iVar2 = FUN_0040c0f0(param_1,iVar6,iVar3,iVar7,iVar8,piVar9,iVar10,piVar11,piVar12,iVar13
                                ,iVar14,iVar15,iVar16,piVar17,iVar18), iVar2 != 1)) {
    iVar3 = FUN_0040c0f0(param_1,iVar6,iVar3,iVar7,iVar8,piVar9,iVar10,piVar11,piVar12,iVar13,iVar14
                         ,iVar15,iVar16,piVar17,iVar18);
    if (iVar3 == 1) {
      (**(code **)(*piVar11 + 0x60))(0);
    }
    else {
      iVar3 = FUN_0040c0f0();
      if (iVar3 == 1) {
        (**(code **)(*(int *)param_1[0x9f3e] + 0x60))(0);
        (**(code **)(*(int *)param_1[0x9f25] + 0x8c))(0);
        if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x33), iVar3 != 0)) {
          uVar5 = 0;
          uVar4 = 0;
          FUN_00429a6d(0x33);
          FUN_0042ad2b(uVar4,uVar5);
        }
      }
      else if ((param_1[0xa269] == 0) || (iVar3 = FUN_0040c0f0(), iVar3 != 1)) {
        if ((param_1[0xa2d1] == 0) || (iVar3 = FUN_0040c0f0(), iVar3 != 1)) {
          iVar3 = FUN_0040c0f0();
          if (iVar3 == 1) {
            (**(code **)(*param_1 + 0x58))(0x170,0);
          }
          else {
            iVar3 = FUN_0040c0f0();
            if (iVar3 == 1) {
              (**(code **)(*param_1 + 0x58))(0x215,0);
            }
            else {
              iVar3 = FUN_0040c0f0();
              if (iVar3 == 1) {
                (**(code **)(*param_1 + 0x58))(0x779,0);
              }
              else {
                iVar3 = FUN_0040c0f0();
                if (iVar3 == 1) {
                  FUN_0044b890(0);
                }
                else {
                  iVar3 = FUN_0040c0f0();
                  if (iVar3 == 1) {
                    FUN_0044da6f();
                  }
                  else {
                    iVar3 = FUN_0040c0f0();
                    if (iVar3 == 1) {
                      FUN_004481c5(0);
                    }
                    else {
                      iVar3 = FUN_0040c0f0();
                      if (iVar3 == 1) {
                        FUN_004484f3(0);
                      }
                      else {
                        iVar3 = FUN_0040c0f0();
                        if (iVar3 == 1) {
                          FUN_0044a745(0,0);
                        }
                        else {
                          iVar3 = FUN_0040c0f0();
                          if (iVar3 == 1) {
                            (**(code **)(*(int *)param_1[0x9f66] + 0x60))(0);
                            (**(code **)(*(int *)param_1[0x9f67] + 0x60))(0);
                          }
                          else {
                            iVar3 = FUN_0040c0f0();
                            if (iVar3 == 1) {
                              (**(code **)(*piVar9 + 0x60))(0);
                            }
                            else if ((param_1[0x9f22] == 0) || (iVar3 = FUN_0040c0f0(), iVar3 != 1))
                            {
                              if ((param_1[0x9f4e] == 0) || (iVar3 = FUN_0040c0f0(), iVar3 != 1)) {
                                if ((param_1[0x9f2a] == 0) || (iVar3 = FUN_0040c0f0(), iVar3 != 1))
                                {
                                  if ((piVar12 == (int *)0x0) ||
                                     (iVar3 = FUN_0040c0f0(), iVar3 != 1)) {
                                    if ((piVar17 == (int *)0x0) ||
                                       (iVar3 = FUN_0040c0f0(), iVar3 != 1)) {
                                      if ((param_1[0x23] == 0) ||
                                         (iVar3 = FUN_0040c0f0(), iVar3 != 1)) {
                                        iVar3 = FUN_0040c0f0();
                                        if (iVar3 == 0) {
                                          (**(code **)(*piVar1 + 0x60))(1);
                                        }
                                        else {
                                          iVar3 = FUN_0040c0f0();
                                          if (iVar3 == 1) {
                                            (**(code **)(*piVar1 + 0x60))(0);
                                          }
                                        }
                                      }
                                      else {
                                        (**(code **)(*(int *)param_1[0x23] + 0x60))(0);
                                      }
                                    }
                                    else {
                                      (**(code **)(*piVar17 + 0x60))(0);
                                    }
                                  }
                                  else {
                                    (**(code **)(*piVar12 + 0x60))(0);
                                  }
                                }
                                else {
                                  (**(code **)(*(int *)param_1[0x9f2a] + 0x60))(0);
                                  (**(code **)(*(int *)param_1[0x9f29] + 0x8c))(0);
                                  if ((DAT_005ccf98 != 0) &&
                                     (iVar3 = FUN_00429a6d(0x33), iVar3 != 0)) {
                                    uVar5 = 0;
                                    uVar4 = 0;
                                    FUN_00429a6d(0x33);
                                    FUN_0042ad2b(uVar4,uVar5);
                                  }
                                }
                              }
                              else {
                                (**(code **)(*(int *)param_1[0x9f4e] + 0x60))(0);
                              }
                            }
                            else {
                              (**(code **)(*(int *)param_1[0x9f22] + 0x60))(0);
                              (**(code **)(*(int *)param_1[10] + 0x40))(0);
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
        else {
          (**(code **)(*(int *)param_1[0xa2d1] + 0x60))(0);
        }
      }
      else {
        (**(code **)(*(int *)param_1[0xa269] + 0x60))(0);
      }
    }
  }
  else {
    FUN_0044ae38(0,0);
  }
  return;
}

