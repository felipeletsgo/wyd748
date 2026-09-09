// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0047fcd7 | Name: FUN_0047fcd7


undefined4 __thiscall FUN_0047fcd7(int *param_1,byte param_2)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  int *piVar4;
  undefined4 uVar5;
  int iVar6;
  char *pcVar7;
  void *pvVar8;
  size_t sVar9;
  undefined4 uVar10;
  size_t _Count;
  undefined4 local_f80;
  int local_f2c;
  char local_f28 [64];
  char local_ee8 [64];
  char local_ea8 [64];
  undefined1 local_e68 [64];
  undefined1 local_e28 [384];
  int local_ca8;
  int local_ca4;
  undefined1 local_ca0 [128];
  undefined1 local_c20 [2560];
  char local_220 [6];
  undefined1 local_21a [250];
  undefined1 local_120 [256];
  char local_20 [8];
  uint local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f985;
  local_10 = ExceptionList;
  local_14 = (int *)param_1[0xa304];
  if (local_14 == (int *)0x0) {
    uVar5 = 0;
  }
  else if (param_1[0xa305] == 0) {
    uVar5 = 0;
  }
  else {
    ExceptionList = &local_10;
    (**(code **)(*local_14 + 0x60))(param_2);
    if (param_2 == 1) {
      if ((param_1[0x9f21] != 0) && (*(int *)(param_1[0x9f21] + 0x28) == 1)) {
        FUN_0044c53f();
      }
      if ((param_1[0x9f1f] != 0) && (*(int *)(param_1[0x9f1f] + 0x28) == 1)) {
        FUN_00447c73();
      }
      if ((param_1[0xa176] != 0) && (*(int *)(param_1[0xa176] + 0x28) == 1)) {
        FUN_004484f3(0);
      }
      if ((param_1[0x9ece] != 0) && (*(int *)(param_1[0x9ece] + 0x28) == 1)) {
        FUN_0044ae38(0,0);
      }
      if ((param_1[0x9ecd] != 0) && (*(int *)(param_1[0x9ecd] + 0x28) == 1)) {
        FUN_00447691();
      }
      if ((param_1[0xa175] != 0) && (*(int *)(param_1[0xa175] + 0x28) == 1)) {
        FUN_004481c5(0);
      }
      FUN_0058f078(local_ca0,&DAT_005b8748,&DAT_00a3fe78);
      _Count = 0;
      (**(code **)(*(int *)param_1[0xa306] + 0x80))(local_ca0);
      (**(code **)(*(int *)param_1[0xa306] + 0x84))(0xffffffff);
      if (param_1[0xa307] != 0) {
        (**(code **)(*(int *)param_1[0xa307] + 0x80))(&DAT_00a3ff78,0);
      }
      if (param_1[0xa308] != 0) {
        (**(code **)(*(int *)param_1[0xa308] + 0x80))(&DAT_00a3fef8,0);
      }
      if (param_1[0xa309] != 0) {
        (**(code **)(*(int *)param_1[0xa309] + 0x80))(&DAT_00a3fff8,0);
      }
      FUN_0049e77b(param_1[0x9f44],s_UI_QuestSubjects_txt_005b874c,400,0);
      FUN_0049e77b(param_1[0x9f45],s_UI_QuestSubjects2_txt_005b8764,400,0);
      FUN_0049e77b(param_1[0x9f46],s_UI_QuestSubjects3_txt_005b877c,400,0);
      FUN_0049e77b(param_1[0x9f47],s_UI_QuestSubjects4_txt_005b8794,400,0);
      _memset(local_c20,0,0xa00);
      if ((param_1[0x9f44] == 0) || (param_1[0x9f45] == 0)) {
        ExceptionList = local_10;
        return 0;
      }
      _memset(local_f28,0,0x280);
      sVar9 = 0x16;
      iVar6 = (**(code **)(**(int **)(param_1[0x9f44] + 0x214) + 0x88))();
      _strncpy(local_f28,(char *)(iVar6 + 8),sVar9);
      sVar9 = 0xf;
      iVar6 = (**(code **)(**(int **)(param_1[0x9f44] + 0x218) + 0x88))();
      _strncpy(local_ee8,(char *)(iVar6 + 8),sVar9);
      iVar6 = (**(code **)(**(int **)(param_1[0x9f44] + 0x22c) + 0x88))(0x12);
      _strncpy(local_ea8,(char *)(iVar6 + 8),_Count);
      FUN_0058f078(local_e68,&DAT_00a3ab78);
      FUN_0058f078(local_e28,&DAT_00a3ae78);
      for (local_f2c = 0; local_f2c < 5; local_f2c = local_f2c + 1) {
        FUN_0058f078(local_c20 + local_f2c * 0x100,s_FFFFFF__04d__04d__s_005b87ac,
                     *(undefined4 *)(&DAT_005c3ae0 + local_f2c * 8),
                     *(undefined4 *)(&DAT_005c3ae4 + local_f2c * 8),local_f28 + local_f2c * 0x40,0);
      }
      *(undefined4 *)(param_1[0xa308] + 0x4c) = 0x43190000;
      *(undefined4 *)(param_1[0xa309] + 0x4c) = 0x43770000;
      *(undefined4 *)(param_1[0xa307] + 0x50) = 0x41e80000;
      *(undefined4 *)(param_1[0xa308] + 0x50) = 0x41e80000;
      *(undefined4 *)(param_1[0xa309] + 0x50) = 0x41e80000;
      local_ca8 = 0;
      FUN_004091ab();
      for (local_ca4 = 0; local_ca4 < 5; local_ca4 = local_ca4 + 1) {
        FUN_0058f078(local_220,&DAT_005b87d0,local_c20 + local_ca4 * 0x100);
        _memset(local_20,0,7);
        _strncpy(local_20,local_220,6);
        FUN_005909d2(local_20,&DAT_005b87d4,&local_18);
        pcVar7 = _strstr(local_220,&DAT_005b87d8);
        if (pcVar7 != (char *)0x0) {
          *pcVar7 = '\0';
        }
        FUN_0058f078(local_120,&DAT_005b87dc,local_21a);
        pvVar8 = operator_new(0xe50);
        local_8 = 0;
        if (pvVar8 == (void *)0x0) {
          local_f80 = 0;
        }
        else {
          local_f80 = FUN_00407203(local_120,local_18 | 0xff000000,0,0,
                                   *(undefined4 *)(param_1[0xa305] + 0x54),0x41800000,0,0x77777777,1
                                   ,0);
        }
        local_8 = 0xffffffff;
        FUN_00408d33(local_f80);
        local_ca8 = local_ca8 + 1;
        if (100 < local_ca8) break;
      }
      if (*(int *)(param_1[0xa305] + 0x20c) != 0) {
        (**(code **)(**(int **)(param_1[0xa305] + 0x20c) + 0x80))(0);
      }
    }
    param_1[0x9b8f] = (uint)param_2;
    if (param_2 == 0) {
      piVar1 = (int *)param_1[0x9f1e];
      piVar2 = (int *)param_1[0x9f1d];
      piVar3 = (int *)param_1[0xa2e4];
      piVar4 = (int *)param_1[0xa304];
      if (DAT_013b736c == 1) {
        FUN_0055ee59(0);
        ExceptionList = local_10;
        return 0;
      }
      if ((param_1[0x9ece] == 0) || (iVar6 = FUN_0040c0f0(), iVar6 != 1)) {
        iVar6 = FUN_0040c0f0();
        if (iVar6 == 1) {
          (**(code **)(*piVar2 + 0x60))(0);
        }
        else {
          iVar6 = FUN_0040c0f0();
          if (iVar6 == 1) {
            (**(code **)(*(int *)param_1[0x9f3e] + 0x60))(0);
            (**(code **)(*(int *)param_1[0x9f25] + 0x8c))(0);
            if ((DAT_005ccf98 != 0) && (iVar6 = FUN_00429a6d(0x33), iVar6 != 0)) {
              uVar10 = 0;
              uVar5 = 0;
              FUN_00429a6d(0x33);
              FUN_0042ad2b(uVar5,uVar10);
            }
          }
          else if ((param_1[0xa269] == 0) || (iVar6 = FUN_0040c0f0(), iVar6 != 1)) {
            if ((param_1[0xa2d1] == 0) || (iVar6 = FUN_0040c0f0(), iVar6 != 1)) {
              iVar6 = FUN_0040c0f0();
              if (iVar6 == 1) {
                (**(code **)(*param_1 + 0x58))(0x170,0);
              }
              else {
                iVar6 = FUN_0040c0f0();
                if (iVar6 == 1) {
                  (**(code **)(*param_1 + 0x58))(0x215,0);
                }
                else {
                  iVar6 = FUN_0040c0f0();
                  if (iVar6 == 1) {
                    (**(code **)(*param_1 + 0x58))(0x779,0);
                  }
                  else {
                    iVar6 = FUN_0040c0f0();
                    if (iVar6 == 1) {
                      FUN_0044b890(0);
                    }
                    else {
                      iVar6 = FUN_0040c0f0();
                      if (iVar6 == 1) {
                        FUN_0044da6f();
                      }
                      else {
                        iVar6 = FUN_0040c0f0();
                        if (iVar6 == 1) {
                          FUN_004481c5(0);
                        }
                        else {
                          iVar6 = FUN_0040c0f0();
                          if (iVar6 == 1) {
                            FUN_004484f3(0);
                          }
                          else {
                            iVar6 = FUN_0040c0f0();
                            if (iVar6 == 1) {
                              FUN_0044a745(0,0);
                            }
                            else {
                              iVar6 = FUN_0040c0f0();
                              if (iVar6 == 1) {
                                (**(code **)(*(int *)param_1[0x9f66] + 0x60))(0);
                                (**(code **)(*(int *)param_1[0x9f67] + 0x60))(0);
                              }
                              else {
                                iVar6 = FUN_0040c0f0();
                                if (iVar6 == 1) {
                                  (**(code **)(*piVar1 + 0x60))(0);
                                }
                                else if ((param_1[0x9f22] == 0) ||
                                        (iVar6 = FUN_0040c0f0(), iVar6 != 1)) {
                                  if ((param_1[0x9f4e] == 0) || (iVar6 = FUN_0040c0f0(), iVar6 != 1)
                                     ) {
                                    if ((param_1[0x9f2a] == 0) ||
                                       (iVar6 = FUN_0040c0f0(), iVar6 != 1)) {
                                      if ((piVar3 == (int *)0x0) ||
                                         (iVar6 = FUN_0040c0f0(), iVar6 != 1)) {
                                        if ((piVar4 == (int *)0x0) ||
                                           (iVar6 = FUN_0040c0f0(), iVar6 != 1)) {
                                          if ((param_1[0x23] != 0) &&
                                             (iVar6 = FUN_0040c0f0(), iVar6 == 1)) {
                                            (**(code **)(*(int *)param_1[0x23] + 0x60))(0);
                                          }
                                        }
                                        else {
                                          (**(code **)(*piVar4 + 0x60))(0);
                                        }
                                      }
                                      else {
                                        (**(code **)(*piVar3 + 0x60))(0);
                                      }
                                    }
                                    else {
                                      (**(code **)(*(int *)param_1[0x9f2a] + 0x60))(0);
                                      (**(code **)(*(int *)param_1[0x9f29] + 0x8c))(0);
                                      if ((DAT_005ccf98 != 0) &&
                                         (iVar6 = FUN_00429a6d(0x33), iVar6 != 0)) {
                                        uVar10 = 0;
                                        uVar5 = 0;
                                        FUN_00429a6d(0x33);
                                        FUN_0042ad2b(uVar5,uVar10);
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
    }
    uVar5 = 1;
  }
  ExceptionList = local_10;
  return uVar5;
}

