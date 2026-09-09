// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054b359 | Name: FUN_0054b359


undefined4 FUN_0054b359(short *param_1,int param_2)

{
  short sVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  
  iVar6 = (int)*param_1;
  if ((iVar6 < 1) || (0x1963 < iVar6)) {
    uVar2 = 0;
  }
  else if (param_2 < 500000) {
    iVar3 = FUN_0054e06c(param_1);
    if (iVar3 < 3) {
      sVar1 = *(short *)(&DAT_00d449d6 + iVar6 * 0x8c);
      iVar3 = FUN_0054daac(param_1,0x3c);
      iVar4 = FUN_0054daac(param_1,2);
      iVar5 = FUN_00552d38(param_1);
      if ((sVar1 == 0x40) || (sVar1 == 0xc0)) {
        if ((0xb < iVar3) || (0x1a < iVar4)) {
          return 1;
        }
      }
      else if ((7 < iVar3) || (0xb < iVar4)) {
        return 1;
      }
      if ((((((iVar5 < 1) && (iVar6 != 0x1a3)) && (iVar6 != 0x1a4)) && (iVar6 != 0x291)) &&
          ((iVar6 < 0x1d6 || (0x1ee < iVar6)))) && ((iVar6 < 0xcee || (3999 < iVar6)))) {
        if ((iVar6 < 0x8fc) || (0x952 < iVar6)) {
          if (((iVar6 < 0x227) ||
              (((0x232 < iVar6 || ((char)param_1[1] == '\0')) || ((char)param_1[1] == ';')))) ||
             (((char)param_1[2] == '\0' || ((char)param_1[2] == ';')))) {
            if ((iVar6 < 0x29b) || (0x29f < iVar6)) {
              if ((iVar6 < 0x957) || (0x970 < iVar6)) {
                if (((iVar6 < 0x13b) || (0x155 < iVar6)) &&
                   ((((iVar6 < 0x91a || (0x952 < iVar6)) && (iVar6 != 0xcff)) && (iVar6 != 0xc44))))
                {
                  if (((iVar6 < 0x309) || (0x311 < iVar6)) && ((iVar6 < 0xc65 || (0xc76 < iVar6))))
                  {
                    if ((iVar6 == 0xfad) || (iVar6 == 0xfae)) {
                      uVar2 = 1;
                    }
                    else if ((iVar6 < 0xfb0) || (0xfb9 < iVar6)) {
                      if ((iVar6 < 0x1012) || (0x1014 < iVar6)) {
                        if ((iVar6 < 0x1015) || (0x101e < iVar6)) {
                          if (iVar6 == 0x101f) {
                            uVar2 = 1;
                          }
                          else if ((iVar6 < 0x1020) || (0x1022 < iVar6)) {
                            if (iVar6 == 0x100e) {
                              uVar2 = 1;
                            }
                            else if (iVar6 == 0xc62) {
                              uVar2 = 1;
                            }
                            else if ((iVar6 < 0xfd3) || (0xfd7 < iVar6)) {
                              if (iVar6 == 0x14da) {
                                uVar2 = 1;
                              }
                              else if ((iVar6 == 0x19c) || (iVar6 == 0x19d)) {
                                uVar2 = 1;
                              }
                              else if (*(int *)(&DAT_00d449d0 + iVar6 * 0x8c) < 500000) {
                                uVar2 = 0;
                              }
                              else {
                                uVar2 = 1;
                              }
                            }
                            else {
                              uVar2 = 1;
                            }
                          }
                          else {
                            uVar2 = 1;
                          }
                        }
                        else {
                          uVar2 = 1;
                        }
                      }
                      else {
                        uVar2 = 1;
                      }
                    }
                    else {
                      uVar2 = 1;
                    }
                  }
                  else {
                    uVar2 = 1;
                  }
                }
                else {
                  uVar2 = 1;
                }
              }
              else {
                uVar2 = 1;
              }
            }
            else {
              uVar2 = 1;
            }
          }
          else {
            uVar2 = 1;
          }
        }
        else {
          uVar2 = 1;
        }
      }
      else {
        uVar2 = 1;
      }
    }
    else {
      uVar2 = 1;
    }
  }
  else {
    uVar2 = 1;
  }
  return uVar2;
}

