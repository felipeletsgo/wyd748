// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00453c59 | Name: FUN_00453c59


undefined4 __thiscall FUN_00453c59(int param_1,char param_2,undefined4 param_3)

{
  undefined4 uVar1;
  uint uVar2;
  int iVar3;
  undefined8 uVar4;
  
  if (*(int *)(param_1 + 0x26e1c) == 1) {
    uVar1 = 0;
  }
  else {
    uVar2 = (**(code **)(*DAT_0092e654 + 8))();
    if (uVar2 < DAT_013b7220 + 6000U) {
      uVar1 = 1;
    }
    else if (uVar2 < *(int *)(param_1 + 0x27138) + 6000U) {
      uVar1 = 1;
    }
    else if (uVar2 < *(int *)(param_1 + 0x2713c) + 6000U) {
      uVar1 = 1;
    }
    else if (uVar2 < *(int *)(param_1 + 0x27140) + 6000U) {
      uVar1 = 1;
    }
    else if (uVar2 < *(int *)(param_1 + 0x27148) + 6000U) {
      uVar1 = 1;
    }
    else if (uVar2 < *(int *)(param_1 + 0x27144) + 6000U) {
      uVar1 = 1;
    }
    else if (uVar2 < *(int *)(param_1 + 0x2714c) + 6000U) {
      uVar1 = 1;
    }
    else if (uVar2 < *(int *)(param_1 + 0x27150) + 6000U) {
      uVar1 = 1;
    }
    else {
      uVar4 = FUN_0040c0f0();
      iVar3 = (int)((ulonglong)uVar4 >> 0x20);
      if (((int)uVar4 == 1) &&
         (((iVar3 = *(int *)(param_1 + 0x8c), *(int *)(iVar3 + 0x1ec) == 0x259 ||
           (*(int *)(*(int *)(param_1 + 0x8c) + 0x1ec) == 0x39f)) &&
          (iVar3 = (int)param_2, iVar3 == 0xd)))) {
        uVar1 = 1;
      }
      else {
        if (DAT_013b722c == 1) {
          if (param_2 == '\x1b') {
            FUN_0044df53();
            return 1;
          }
          uVar4 = FUN_0049ac84(CONCAT31((int3)((uint)iVar3 >> 8),param_2),param_3);
          iVar3 = (int)((ulonglong)uVar4 >> 0x20);
          if ((int)uVar4 == 1) {
            return 1;
          }
        }
        else {
          iVar3 = FUN_0049ac84(param_2,param_3);
          if (iVar3 == 1) {
            return 1;
          }
          iVar3 = (int)param_2;
          if (iVar3 == 0x1b) {
            FUN_0044df53();
            return 1;
          }
        }
        if (*(int *)(param_1 + 0x54) == 1) {
          uVar1 = 1;
        }
        else {
          iVar3 = FUN_00455b5b(CONCAT31((int3)((uint)iVar3 >> 8),param_2),param_3);
          if (iVar3 == 0) {
            iVar3 = FUN_00453802(param_2,param_3);
            if (iVar3 == 0) {
              if (DAT_013b722c == 0) {
                iVar3 = FUN_00455c38(param_2,param_3);
                if (iVar3 != 0) {
                  return 1;
                }
                iVar3 = FUN_004509eb(param_2,param_3);
                if (iVar3 != 0) {
                  return 1;
                }
                iVar3 = FUN_004509fa(param_2,param_3);
                if (iVar3 != 0) {
                  return 1;
                }
                iVar3 = FUN_00452210(param_2,param_3);
                if (iVar3 != 0) {
                  return 1;
                }
                iVar3 = FUN_00452271(param_2,param_3);
                if (iVar3 != 0) {
                  return 1;
                }
                iVar3 = FUN_004523fa(param_2,param_3);
                if (iVar3 != 0) {
                  return 1;
                }
                iVar3 = FUN_0045243c(param_2,param_3);
                if (iVar3 != 0) {
                  return 1;
                }
                iVar3 = FUN_0045246c(param_2,param_3);
                if (iVar3 != 0) {
                  return 1;
                }
                iVar3 = FUN_004524ab(param_2,param_3);
                if (iVar3 != 0) {
                  return 1;
                }
                iVar3 = FUN_00452571(param_2,param_3);
                if (iVar3 != 0) {
                  return 1;
                }
                iVar3 = FUN_004525a1(param_2,param_3);
                if (iVar3 != 0) {
                  return 1;
                }
                iVar3 = FUN_004525d1(param_2,param_3);
                if (iVar3 != 0) {
                  return 1;
                }
                iVar3 = FUN_004539bc(param_2,param_3);
                if (iVar3 != 0) {
                  return 1;
                }
                if (0 < *(short *)(DAT_013b71e8 + 0x738)) {
                  iVar3 = FUN_00452601(param_2,param_3);
                  if (iVar3 != 0) {
                    return 1;
                  }
                  iVar3 = FUN_00452631(param_2,param_3);
                  if (iVar3 != 0) {
                    return 1;
                  }
                  iVar3 = FUN_00452661(param_2,param_3);
                  if (iVar3 != 0) {
                    return 1;
                  }
                  iVar3 = FUN_00452691(param_2,param_3);
                  if (iVar3 != 0) {
                    return 1;
                  }
                  iVar3 = FUN_004526ee(param_2,param_3);
                  if (iVar3 != 0) {
                    return 1;
                  }
                  iVar3 = FUN_00452728(param_2,param_3);
                  if (iVar3 != 0) {
                    return 1;
                  }
                  iVar3 = FUN_00452737(param_2,param_3);
                  if (iVar3 != 0) {
                    return 1;
                  }
                  iVar3 = FUN_0045288b(param_2,param_3);
                  if (iVar3 != 0) {
                    return 1;
                  }
                  iVar3 = FUN_004528c5(param_2,param_3);
                  if (iVar3 != 0) {
                    return 1;
                  }
                }
                iVar3 = FUN_0045379f(param_2,param_3);
                if (iVar3 != 0) {
                  return 1;
                }
                iVar3 = FUN_00453829(param_2,param_3);
                if (iVar3 != 0) {
                  return 1;
                }
                iVar3 = FUN_004538c6(param_2,param_3);
                if (iVar3 != 0) {
                  return 1;
                }
                iVar3 = FUN_0045398c(param_2,param_3);
                if (iVar3 != 0) {
                  return 1;
                }
                iVar3 = FUN_00453b35(param_2,param_3);
                if (iVar3 != 0) {
                  return 1;
                }
                iVar3 = FUN_00453b65(param_2,param_3);
                if (iVar3 != 0) {
                  return 1;
                }
              }
              uVar1 = 0;
            }
            else {
              uVar1 = 1;
            }
          }
          else {
            uVar1 = 1;
          }
        }
      }
    }
  }
  return uVar1;
}

