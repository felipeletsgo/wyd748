// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ac985 | Name: FUN_004ac985


/* WARNING: Type propagation algorithm not settling */

undefined4 __thiscall FUN_004ac985(int *param_1,int param_2,int param_3)

{
  void *pvVar1;
  char *pcVar2;
  size_t sVar3;
  int iVar4;
  undefined4 uVar5;
  uint uVar6;
  int local_b54;
  int local_b50;
  int local_b30;
  uint local_b2c;
  int local_b28;
  char local_b24 [256];
  char *local_a24;
  int *local_a20;
  undefined4 local_a1c;
  size_t local_a18;
  undefined4 local_a14;
  size_t local_a10;
  int *local_a0c;
  size_t local_a08;
  int local_a04;
  int *local_a00;
  undefined1 local_9fc [4];
  undefined2 local_9f8;
  undefined2 local_9f6;
  undefined1 local_9f0 [16];
  undefined1 local_9e0;
  undefined1 local_9df;
  undefined4 local_9d4;
  undefined4 local_9d0;
  undefined1 local_9cc [4];
  undefined1 local_9c8 [4];
  undefined1 local_9c4 [4];
  undefined1 local_9c0 [56];
  undefined1 local_988 [640];
  int local_708;
  int local_704;
  int local_700;
  int local_6fc;
  int local_6f8;
  int local_6f4;
  int local_6f0;
  int local_6ec;
  undefined1 local_6e8 [256];
  int *local_5e8;
  undefined1 local_5e4 [128];
  undefined4 local_564;
  size_t local_560;
  size_t local_55c;
  size_t local_558;
  size_t local_554;
  int local_550;
  int local_54c;
  int local_548;
  uint local_544;
  int *local_540;
  char local_53c [14];
  undefined1 local_52e;
  uint local_4bc;
  undefined4 local_4b8;
  int local_4b4 [12];
  int local_484;
  undefined1 local_480 [4];
  undefined1 local_47c [4];
  undefined1 local_478 [4];
  undefined1 local_474 [4];
  undefined1 local_470 [4];
  undefined1 local_46c [4];
  undefined1 local_468 [4];
  undefined1 local_464 [4];
  undefined1 local_460 [4];
  undefined1 local_45c [8];
  int local_454;
  undefined1 local_450 [1024];
  int local_50;
  int *local_4c [2];
  undefined1 local_44 [4];
  undefined1 local_40 [4];
  undefined1 local_3c [4];
  undefined1 local_38 [4];
  undefined1 local_34 [4];
  undefined1 local_30 [4];
  undefined1 local_2c [4];
  undefined1 local_28 [4];
  undefined1 local_24 [8];
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a03f7;
  local_10 = ExceptionList;
  local_18 = 0;
  for (local_14 = 0; local_14 < param_1[0x9b9b]; local_14 = local_14 + 1) {
    if (*(int *)(&DAT_013b71b0 + local_14 * 4) != 0) {
      local_18 = local_18 + 1;
    }
  }
  local_1c = *(int *)(&DAT_013b71ac + (local_18 - param_3) * 4) + -1;
  if (param_2 == 0x120a) {
    for (local_50 = 0; local_50 < 0xb; local_50 = local_50 + 1) {
      *(undefined4 *)(local_44 + local_50 * 4 + -4) = 0xffffffff;
    }
    ExceptionList = &local_10;
    FUN_00403df2(&DAT_00a34cf8,0);
    (**(code **)(*(int *)param_1[0x21] + 0x88))(1,0);
    if ((param_1[0x9b9d] == 1) && (local_1c == param_1[0x9b9c])) {
      for (local_454 = param_1[0x9b9c]; local_454 < 10; local_454 = local_454 + 1) {
        (&DAT_009325f8)[local_454 * 0x40 + local_1c * 0x2c0] = 0;
      }
      for (local_454 = 0; local_454 < param_1[0x9b9c]; local_454 = local_454 + 1) {
        _memset(local_480,-1,0x2c);
        FUN_00557a41(&DAT_009325f8 + local_454 * 0x2c0,local_450,0x400);
        FUN_005909d2(local_450,s__d__d__d__d__d__d__d__d__d__d_005b97e8,local_480,local_47c,
                     local_478,local_474,local_470,local_46c,local_468,local_464,local_460,local_45c
                    );
        local_4c[param_1[0x9b9c] - local_454] =
             *(int **)(local_480 + param_1[local_454 + 0x9b9e] * 4 + -4);
        FUN_0058f078(&DAT_009325f8 + (local_454 + 1) * 0x40 + local_1c * 0x2c0,&DAT_005b9808,
                     &DAT_009325f8 +
                     param_1[(param_1[0x9b9c] - local_454) + 0x9b9d] * 0x40 +
                     ((param_1[0x9b9c] - local_454) + -1) * 0x2c0);
      }
    }
    else {
      FUN_00557a41(&DAT_009325f8 + local_1c * 0x2c0,local_450,0x400);
      FUN_005909d2(local_450,s__d__d__d__d__d__d__d__d__d__d_005b980c,local_4c + 1,local_44,local_40
                   ,local_3c,local_38,local_34,local_30,local_2c,local_28,local_24);
    }
    (**(code **)(*(int *)param_1[0x21] + 0x88))(0,1);
    local_4c[0] = (int *)param_1[0x9b89];
    if (local_4c[0] != (int *)0x0) {
      FUN_004091ab();
      for (local_484 = 1; local_484 < 0xb; local_484 = local_484 + 1) {
        if ((&DAT_009325f8)[local_484 * 0x40 + local_1c * 0x2c0] == '\0') {
          if (((param_1[0x9b9d] == 1) && (local_1c == param_1[0x9b9c])) &&
             (local_484 < param_1[0x9b9b])) {
            FUN_0058f078(local_5e4,&DAT_00a36478);
            local_5e8 = local_4c[local_484];
            if ((int)local_5e8 < 0) {
              local_5e8 = (int *)0x0;
            }
            local_564 = 6;
            pvVar1 = operator_new(0xe68);
            local_8 = 1;
            if (pvVar1 == (void *)0x0) {
              local_b54 = 0;
            }
            else {
              local_b54 = FUN_004072f2(local_564,local_5e4,0xffffffff,0,0,(float)DAT_005d04f4,
                                       0x41800000,local_5e8,0,0,local_484);
            }
            local_8 = 0xffffffff;
            local_4b4[local_484] = local_b54;
            if ((int)local_4c[local_484] < 0) {
              *(undefined1 *)(local_4b4[local_484] + 0xe64) = 0;
            }
            FUN_00408d33(local_4b4[local_484]);
          }
        }
        else {
          local_4bc = FUN_004a8a90(local_484 + -1);
          local_544 = 0;
          local_4b4[0] = local_484;
          if ((param_1[0x9b9d] == 1) && (local_1c == param_1[0x9b9c])) {
            local_548 = param_1[0x9b9c] - local_484;
            if (-1 < local_548) {
              local_550 = 0;
              for (local_54c = 0; local_54c < param_1[0x9b9c]; local_54c = local_54c + 1) {
                if (*(int *)(&DAT_013b71b0 + local_54c * 4) == local_548 + 1) {
                  local_550 = 1;
                }
              }
              if (local_550 != 0) {
                local_4bc = FUN_004a8a90(param_1[local_548 + 0x9b9e] + -1);
                local_544 = FUN_004a8aff(param_1[local_548 + 0x9b9e] + -1);
                if ((&DAT_013b6dc8)[(param_1[local_548 + 0x9b9e] + -1) * 9 + local_548 * 0x5a] ==
                    '\0') {
                  FUN_0058f078(local_53c,s__s__d_005b982c,&DAT_013b714c + local_548 * 9,
                               param_1[local_548 + 0x9b9e]);
                  local_4b4[0] = param_1[local_548 + 0x9b9e];
                  if (700 < (int)local_4c[local_484]) {
                    local_554 = _strlen(local_53c);
                    sVar3 = local_554;
                    if ((int)local_554 < 0xe) {
                      while (local_558 = sVar3, (int)local_558 < 0xe) {
                        local_53c[local_558] = ' ';
                        sVar3 = local_558 + 1;
                      }
                    }
                    local_52e = 0;
                    FUN_0058ee30(local_53c,&DAT_005b9834);
                  }
                }
                else {
                  FUN_0058f078(local_53c,s__s__s_005b983c,&DAT_013b714c + local_548 * 9,
                               &DAT_013b6dc8 +
                               (param_1[local_548 + 0x9b9e] + -1) * 9 + local_548 * 0x5a);
                }
                goto LAB_004ad0fc;
              }
            }
          }
          else {
            if ((&DAT_013b714c)[local_1c * 9] == '\0') {
              FUN_0058f078(local_53c,&DAT_00a36378,local_1c + 1,local_484);
            }
            else if ((&DAT_013b6dc8)[(local_484 + -1) * 9 + local_1c * 0x5a] == '\0') {
              FUN_0058f078(local_53c,s__s__d_005b9844,&DAT_013b714c + local_1c * 9,local_484);
              if (700 < (int)local_4c[local_484]) {
                local_55c = _strlen(local_53c);
                sVar3 = local_55c;
                if ((int)local_55c < 0xe) {
                  while (local_560 = sVar3, (int)local_560 < 0xe) {
                    local_53c[local_560] = ' ';
                    sVar3 = local_560 + 1;
                  }
                }
                local_52e = 0;
                FUN_0058ee30(local_53c,&DAT_005b984c);
              }
            }
            else {
              FUN_0058f078(local_53c,s__s__s_005b9854,&DAT_013b714c + local_1c * 9,
                           &DAT_013b6dc8 + (local_484 + -1) * 9 + local_1c * 0x5a);
            }
LAB_004ad0fc:
            if ((int)local_4c[local_484] < 0) {
              FUN_0058f078(local_53c,&DAT_00a36478);
            }
            local_540 = local_4c[local_484];
            if ((int)local_540 < 0) {
              local_540 = (int *)0x0;
            }
            local_4b8 = 6;
            if ((param_1[0x9b9d] == 1) && (local_484 == param_1[local_1c + 0x9b9e])) {
              local_4b8 = 0xfffffffe;
            }
            if ((param_1[0x9b9d] == 1) && (local_1c == param_1[0x9b9c])) {
              local_4b8 = 0xfffffffe;
            }
            pvVar1 = operator_new(0xe68);
            local_8 = 0;
            if (pvVar1 == (void *)0x0) {
              local_b50 = 0;
            }
            else {
              local_b50 = FUN_004072f2(local_4b8,local_53c,0xffffffff,0,0,(float)DAT_005d04f4,
                                       0x41800000,local_540,local_4bc & 0xff,local_544 & 0xff,
                                       local_4b4[0]);
            }
            local_8 = 0xffffffff;
            local_4b4[local_484] = local_b50;
            if ((int)local_4c[local_484] < 0) {
              *(undefined1 *)(local_4b4[local_484] + 0xe64) = 0;
            }
            FUN_00408d33(local_4b4[local_484]);
          }
        }
      }
      (**(code **)(*local_4c[0] + 0x60))(1);
    }
    FUN_004a89a0();
  }
  else if (param_2 == 0x120c) {
    local_700 = param_1[0x9b88];
    local_6f8 = param_1[0x9b89];
    ExceptionList = &local_10;
    iVar4 = FUN_00493cb0();
    local_6fc = *(int *)(&DAT_013b71ac + (local_18 - iVar4) * 4) + -1;
    iVar4 = FUN_00493cb0();
    local_6ec = iVar4 + 1;
    local_6f4 = FUN_00409046(iVar4);
    if (((local_6f4 == 0) || (local_6fc < 0)) || (local_6ec < 1)) {
      FUN_00403df2(&DAT_00a34d78,4000);
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
      ExceptionList = local_10;
      return 1;
    }
    if ((param_1[0x9b9d] == 1) && (local_6fc == param_1[0x9b9c])) {
      local_704 = FUN_00493cb0();
      local_6fc = *(int *)(&DAT_013b71ac + ((local_18 + -1) - local_704) * 4) + -1;
      local_6ec = param_1[*(int *)(&DAT_013b71ac + ((local_18 + -1) - local_704) * 4) + 0x9b9d];
    }
    if (699 < *(int *)(local_6f4 + 0xe60)) {
      FUN_00403df2(&DAT_00a34df8,4000);
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
      ExceptionList = local_10;
      return 1;
    }
    if (*(char *)(local_6f4 + 0xe64) == '\0') {
      FUN_00403df2(&DAT_00a34e78,4000);
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
      ExceptionList = local_10;
      return 1;
    }
    *(int *)(DAT_013b71e8 + 0x1af4c) = local_6fc;
    *(int *)(DAT_013b71e8 + 0x1af50) = local_6ec;
    FUN_0058f078(DAT_013b71e0 + 0x60,&DAT_005b985c,
                 &DAT_009325f8 + local_6ec * 0x40 + local_6fc * 0x2c0);
    for (local_6f0 = 2; local_6f0 < 6; local_6f0 = local_6f0 + 1) {
      if (param_1[local_6f0 + 0x9b8a] != 0) {
        (**(code **)(*(int *)param_1[local_6f0 + 0x9b8a] + 0x60))(0);
      }
    }
    (**(code **)(*(int *)param_1[0x9b90] + 0x60))(0);
    (**(code **)(*(int *)param_1[0x9b88] + 0x60))(0);
    (**(code **)(*(int *)param_1[0x9b89] + 0x60))(0);
    for (local_6f0 = 0; local_6f0 < 3; local_6f0 = local_6f0 + 1) {
      (**(code **)(*(int *)param_1[local_6f0 + 0x9b98] + 0x60))(1);
    }
    (**(code **)(*(int *)param_1[0x9b91] + 0x60))(1);
    (**(code **)(*(int *)param_1[10] + 0x40))(param_1[0x9f47]);
    *(undefined1 *)(param_1 + 0x9f49) = 1;
    iVar4 = (**(code **)(*DAT_0092e654 + 8))();
    param_1[0x9f4a] = iVar4;
    FUN_00494d83(local_6ec);
    if (DAT_013b723c != '\0') {
      iVar4 = FUN_0055ad65(&DAT_013b723c,local_6e8);
      if (iVar4 == 0) {
        ExceptionList = local_10;
        return 1;
      }
      FUN_005909d2(local_6e8,s__s__s_005b9860,&DAT_013b7354,&DAT_013b733c);
      (**(code **)(*(int *)param_1[0x9f47] + 0x90))(&DAT_013b7354);
      (**(code **)(*(int *)param_1[0x9f48] + 0x90))(&DAT_013b733c);
      _memset(&DAT_013b7354,0,0x10);
      _memset(&DAT_013b733c,0,0xc);
      for (local_6f0 = 0; local_6f0 < 3; local_6f0 = local_6f0 + 1) {
        (**(code **)(*(int *)param_1[local_6f0 + 0x9b98] + 0x60))(0);
      }
      (**(code **)(*(int *)param_1[0x9b91] + 0x60))(0);
      (**(code **)(*param_1 + 0x58))(0x1201,0);
      ExceptionList = local_10;
      return 1;
    }
  }
  else {
    ExceptionList = &local_10;
    if (param_2 == 0x1209) {
      if (param_3 == 0) {
        if (*(int *)(param_1[0x23] + 0x1ec) == 0x1203) {
          if ((char)param_1[0x9f49] == '\x01') {
            ExceptionList = &local_10;
            for (local_708 = 2; local_708 < 6; local_708 = local_708 + 1) {
              if (param_1[local_708 + 0x9b8a] != 0) {
                (**(code **)(*(int *)param_1[local_708 + 0x9b8a] + 0x60))(1);
              }
            }
            (**(code **)(*(int *)param_1[0x9b90] + 0x60))(1);
            (**(code **)(*(int *)param_1[0x9b88] + 0x60))(1);
            (**(code **)(*(int *)param_1[0x9b89] + 0x60))(1);
            for (local_708 = 0; local_708 < 3; local_708 = local_708 + 1) {
              (**(code **)(*(int *)param_1[local_708 + 0x9b98] + 0x60))(0);
            }
            (**(code **)(*(int *)param_1[0x9b91] + 0x60))(0);
            (**(code **)(*(int *)param_1[10] + 0x40))(0);
            *(undefined1 *)(param_1 + 0x9f49) = 2;
            iVar4 = (**(code **)(*DAT_0092e654 + 8))();
            param_1[0x9f4a] = iVar4;
          }
          else {
            ExceptionList = &local_10;
            PostMessageA(*(HWND *)(DAT_013b71e0 + 4),0x10,0,0);
          }
        }
        else {
          ExceptionList = &local_10;
          if (*(int *)(param_1[0x23] + 0x1ec) == 0x1202) {
            ExceptionList = &local_10;
            ShellExecuteA((HWND)0x0,(LPCSTR)0x0,&DAT_00a3c4f8,(LPCSTR)0x0,(LPCSTR)0x0,3);
          }
        }
      }
      else {
        ExceptionList = &local_10;
        (**(code **)(*(int *)param_1[0x23] + 0x60))(0);
      }
    }
  }
  if (param_2 == 0x1201) {
    local_a20 = (int *)param_1[0x9b98];
    local_a00 = (int *)param_1[0x9f47];
    local_a0c = (int *)param_1[0x9f48];
    pcVar2 = (char *)(**(code **)(*local_a00 + 0x88))();
    sVar3 = _strlen(pcVar2);
    if (sVar3 < 4) {
      FUN_00403df2(&DAT_00a342f8,4000);
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
      ExceptionList = local_10;
      return 1;
    }
    pcVar2 = (char *)(**(code **)(*local_a00 + 0x88))();
    sVar3 = _strlen(pcVar2);
    if (0xc < sVar3) {
      FUN_00403df2(&DAT_00a34378,4000);
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
      ExceptionList = local_10;
      return 1;
    }
    pcVar2 = (char *)(**(code **)(*local_a0c + 0x88))();
    sVar3 = _strlen(pcVar2);
    if (sVar3 < 4) {
      FUN_00403df2(&DAT_00a343f8,4000);
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
      ExceptionList = local_10;
      return 1;
    }
    pcVar2 = (char *)(**(code **)(*local_a0c + 0x88))();
    sVar3 = _strlen(pcVar2);
    if (10 < sVar3) {
      FUN_00403df2(&DAT_00a34478,4000);
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
      ExceptionList = local_10;
      return 1;
    }
    (**(code **)(*local_a20 + 100))(0);
    (**(code **)(*local_a0c + 100))(0);
    iVar4 = (**(code **)(*DAT_0092e654 + 8))();
    param_1[0x9f4b] = iVar4;
    FUN_00403df2(&DAT_00a344f8,0);
    (**(code **)(*(int *)param_1[0x21] + 0x88))(1,0);
    iVar4 = FUN_00423f37(DAT_013b71e0 + 0x60,0x2059,0,0x464);
    if (iVar4 == 0) {
      (**(code **)(*local_a20 + 100))(1);
      FUN_00403df2(&DAT_00a34578,4000);
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
      ExceptionList = local_10;
      return 1;
    }
    DAT_005d04e0 = 0;
    _memset(local_9fc,0,0x74);
    local_9f6 = 0;
    local_9f8 = 0x20d;
    local_9d4 = 0x2ec;
    local_9d0 = 1;
    local_a08 = 0;
    _memset(local_988,0,0x280);
    GetAdaptersInfo(local_988,&local_a08);
    if (local_a08 != 0) {
      pvVar1 = _malloc(local_a08);
      GetAdaptersInfo(pvVar1,&local_a08);
      local_a24 = (char *)((int)pvVar1 + 8);
      local_b30 = 0;
      local_b2c = 0;
      sVar3 = _strlen(local_a24);
      for (local_b28 = 0; local_b28 < (int)sVar3; local_b28 = local_b28 + 1) {
        iVar4 = local_b30;
        if (((local_a24[local_b28] != '{') && (local_a24[local_b28] != '}')) &&
           (local_a24[local_b28] != '-')) {
          local_b24[local_b30] = local_a24[local_b28];
          local_b2c = local_b2c + 1;
          uVar6 = local_b2c & 0x80000007;
          if ((int)uVar6 < 0) {
            uVar6 = (uVar6 - 1 | 0xfffffff8) + 1;
          }
          iVar4 = local_b30 + 1;
          if (uVar6 == 0) {
            local_b24[local_b30 + 1] = ' ';
            iVar4 = local_b30 + 2;
          }
        }
        local_b30 = iVar4;
      }
      local_b24[local_b30] = '\0';
      FUN_005909d2(local_b24,s__x__x__x__x_005b9868,local_9cc,local_9c8,local_9c4,local_9c0);
      FUN_0058ffd8(pvVar1);
    }
    uVar5 = (**(code **)(*local_a00 + 0x88))();
    FUN_0058f078(local_9f0,&DAT_005b9874,uVar5);
    uVar5 = (**(code **)(*local_a0c + 0x88))();
    FUN_0058f078(&local_9e0,&DAT_005b9878,uVar5);
    FUN_0058f078(DAT_013b71e8 + 0x1af54,&DAT_005b987c,local_9f0);
    *(undefined1 *)(DAT_013b71e8 + 0x1af64) = local_9e0;
    *(undefined1 *)(DAT_013b71e8 + 0x1af65) = local_9df;
    for (local_a04 = 2; local_a04 < 0xc; local_a04 = local_a04 + 1) {
      iVar4 = _rand();
      *(char *)(DAT_013b71e8 + local_a04 + 0x1af64) = (char)(iVar4 % 10) + '0';
    }
    *(undefined1 *)(DAT_013b71e8 + 0x1af6f) = 0;
    local_a14 = FUN_0059e5fe(DAT_013b71e8 + 0x1af54);
    local_a1c = FUN_0059e5fe(DAT_013b71e8 + 0x1af64);
    FUN_0058f078(DAT_013b71e8 + 0x1af54,&DAT_005b9880,local_a14);
    FUN_0058f078(DAT_013b71e8 + 0x1af64,&DAT_005b9884,local_a1c);
    local_a10 = _strlen((char *)(DAT_013b71e8 + 0x1af54));
    local_a18 = _strlen((char *)(DAT_013b71e8 + 0x1af64));
    for (local_a04 = 0; local_a04 < (int)local_a10; local_a04 = local_a04 + 1) {
      *(char *)(DAT_013b71e8 + local_a04 + 0x1af54) =
           *(char *)(DAT_013b71e8 + local_a04 + 0x1af54) + (char)local_a04;
    }
    for (local_a04 = 0; local_a04 < (int)local_a18; local_a04 = local_a04 + 1) {
      *(char *)(DAT_013b71e8 + local_a04 + 0x1af64) =
           *(char *)(DAT_013b71e8 + local_a04 + 0x1af64) + (char)local_a04;
    }
    FUN_0042550e(local_9fc,0x74);
  }
  else if (param_2 == 0x1202) {
    (**(code **)(*(int *)param_1[0x23] + 0x8c))(&DAT_00a345f8,0x1202,&DAT_00a34678);
    (**(code **)(*(int *)param_1[0x23] + 0x60))(1);
  }
  else if (param_2 == 0x1203) {
    (**(code **)(*(int *)param_1[0x23] + 0x8c))(&DAT_00a346f8,0x1203,0);
    (**(code **)(*(int *)param_1[0x23] + 0x60))(1);
  }
  if ((DAT_005b892c == 2) && (param_2 == 0x1605)) {
    (**(code **)(*(int *)param_1[0x23] + 0x8c))(&DAT_00a34c78,0x1203,0);
    (**(code **)(*(int *)param_1[0x23] + 0x60))(1);
  }
  ExceptionList = local_10;
  return 1;
}

