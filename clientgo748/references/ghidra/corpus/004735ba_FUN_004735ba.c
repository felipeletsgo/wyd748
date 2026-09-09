// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004735ba | Name: FUN_004735ba


void FUN_004735ba(int param_1,int param_2)

{
  char *_Str1;
  int iVar1;
  void *pvVar2;
  int extraout_ECX;
  char *_Str2;
  undefined4 local_1794;
  size_t local_1774;
  undefined1 local_176c [12];
  undefined1 local_1760;
  undefined4 local_166c;
  char local_1668 [256];
  undefined1 local_1568 [10];
  undefined1 local_155e;
  char local_1468 [54];
  undefined1 local_1432;
  int local_1368;
  int local_1364;
  int local_1360;
  undefined4 local_135c;
  int local_1358;
  char *local_1354;
  size_t local_1350;
  size_t local_134c;
  char *local_1348;
  char *local_1344;
  size_t local_1340;
  int local_133c;
  int local_1338;
  int local_1334;
  int local_1330;
  char local_132c [256];
  char local_122c [256];
  char local_112c [256];
  char local_102c [16];
  char local_101c [12];
  char local_1010 [4028];
  undefined4 uStackY_54;
  char *pcStackY_50;
  undefined4 uStackY_4c;
  char *pcStackY_48;
  char *pcStackY_44;
  char *pcStackY_40;
  undefined *puStackY_3c;
  char *pcStackY_38;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f771;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0058f730();
  if (*(int *)(extraout_ECX + 0x27d50) == 0) {
    ExceptionList = local_10;
    return;
  }
  if (*(int *)(extraout_ECX + 0x27d64) == 0) {
    ExceptionList = local_10;
    return;
  }
  if (*(int *)(extraout_ECX + 0x27d54) == 0) {
    ExceptionList = local_10;
    return;
  }
  if (*(int *)(extraout_ECX + 0x27d4c) == 0) {
    ExceptionList = local_10;
    return;
  }
  FUN_004091ab();
  FUN_004091ab();
  local_133c = 0;
  if (param_2 == 0) {
    local_1360 = *(int *)(*(int *)(extraout_ECX + 0x27d4c) + 0x1f8) + param_1;
    if (local_1360 < 0) {
      ExceptionList = local_10;
      return;
    }
    if (*(int *)(*(int *)(extraout_ECX + 0x27d4c) + 0x210) < local_1360) {
      ExceptionList = local_10;
      return;
    }
    local_133c = FUN_00409046();
    if (local_133c == 0) {
      ExceptionList = local_10;
      return;
    }
    param_1 = FUN_0059115a();
  }
  else {
    local_1364 = 0;
    while ((local_1364 < *(int *)(*(int *)(extraout_ECX + 0x27d4c) + 0x210) &&
           ((local_133c = FUN_00409046(), local_133c == 0 ||
            (local_1368 = FUN_0059115a(), local_1368 != param_1))))) {
      local_1364 = local_1364 + 1;
    }
  }
  if (*(short *)(DAT_013b71e8 + 0x7a8) != 0x1fd) {
    _Str2 = (char *)(*(int *)(extraout_ECX + 0x4c) + 0x168);
    _Str1 = (char *)(**(code **)(**(int **)(local_133c + 0xe54) + 0x88))();
    iVar1 = _strcmp(_Str1,_Str2);
    if (iVar1 != 0) {
      (**(code **)(**(int **)(extraout_ECX + 0x27d78) + 0x60))();
      (**(code **)(**(int **)(extraout_ECX + 0x27d7c) + 0x60))();
      goto LAB_00473848;
    }
  }
  (**(code **)(**(int **)(extraout_ECX + 0x27d78) + 0x60))();
  (**(code **)(**(int **)(extraout_ECX + 0x27d7c) + 0x60))();
LAB_00473848:
  local_135c = 1;
  if ((*(int *)(*(int *)(local_133c + 0xe50) + 0x754) != -1) &&
     (*(short *)(DAT_013b71e8 + 0x7a8) != 0x1fd)) {
    local_135c = 0;
  }
  if (*(short *)(DAT_013b71e8 + 0x7a8) == 0) {
    local_135c = 0;
    (**(code **)(**(int **)(extraout_ECX + 0x27d78) + 0x60))();
    (**(code **)(**(int **)(extraout_ECX + 0x27d7c) + 0x60))();
  }
  (**(code **)(**(int **)(extraout_ECX + 0x27d5c) + 0x60))();
  (**(code **)(**(int **)(extraout_ECX + 0x27d84) + 0x60))();
  local_1330 = 0;
  local_1334 = 0;
  local_1354 = (char *)0x0;
  local_1358 = 0;
  _memset(local_1010,0,0x1000);
  *(int *)(extraout_ECX + 0x27d6c) = param_1;
  _memset(local_102c,0,0x10);
  _memset(local_101c,0,0xc);
  FUN_0058f078();
  FUN_0058f078();
  _memset(local_112c,0,0x100);
  FUN_0058f078();
  local_1340 = _strlen(local_102c);
  local_134c = _strlen(local_101c);
  local_1350 = _strlen(local_112c);
  for (local_1338 = 0; (local_1338 < (int)local_134c && (local_1338 < (int)local_1350));
      local_1338 = local_1338 + 1) {
    local_101c[local_1338] = local_101c[local_1338] + local_112c[local_1338] + -0x30;
  }
  for (local_1338 = 0; (local_1338 < (int)local_1340 && (local_1338 < (int)local_1350));
      local_1338 = local_1338 + 1) {
    local_102c[local_1338] = local_102c[local_1338] + local_112c[local_1338] + -0x30;
  }
  if (*(int *)(extraout_ECX + 0x27d70) < 0) {
    pcStackY_38 = local_102c;
    puStackY_3c = &DAT_00a39c78;
    pcStackY_40 = s__s_asp_id__s_pw__s_server__d_gui_005b8378;
    pcStackY_44 = local_132c;
    pcStackY_48 = (char *)0x473be6;
    FUN_0058f078();
  }
  else {
    pcStackY_38 = local_102c;
    puStackY_3c = *(undefined **)(extraout_ECX + 0x27d70);
    pcStackY_40 = &DAT_00a39c78;
    pcStackY_44 = s__s_d_asp_id__s_pw__s_server__d_g_005b833c;
    pcStackY_48 = local_132c;
    uStackY_4c = 0x473b89;
    FUN_0058f078();
  }
  if (param_2 == 0) {
    FUN_00557a41();
  }
  else {
    FUN_0058f220();
  }
  local_1348 = _strchr(local_1010 + (int)local_1354,10);
  if (local_1348 == (char *)0x0) {
    FUN_00403df2();
    (**(code **)(**(int **)(extraout_ECX + 0x84) + 0x88))();
  }
  else {
    local_1334 = FUN_005909d2();
    if (local_1334 == -1) {
      FUN_00403df2();
      (**(code **)(**(int **)(extraout_ECX + 0x84) + 0x88))();
    }
    else {
      if (local_1348[-1] == '\r') {
        local_1330 = 1;
      }
      local_1354 = local_1348 + (local_1330 - (int)local_1010);
      if ((local_1358 < 0) || (10 < local_1358)) {
        FUN_00403df2();
        (**(code **)(**(int **)(extraout_ECX + 0x84) + 0x88))();
      }
      else {
        for (local_1338 = 0; local_1338 < local_1358; local_1338 = local_1338 + 1) {
          _memset(local_176c,0,0x100);
          _memset(local_1568,0,0x100);
          _memset(local_1468,0,0x100);
          _memset(local_1668,0,0x100);
          local_1348 = _strchr(local_1010 + (int)local_1354,10);
          if (local_1348 == (char *)0x0) {
            ExceptionList = local_10;
            return;
          }
          local_1334 = FUN_005909d2();
          if (local_1334 == -1) {
            ExceptionList = local_10;
            return;
          }
          local_1354 = local_1348 + (local_1330 - (int)local_1010);
          iVar1 = _strcmp(local_122c,&DAT_005b83b8);
          if (iVar1 != 0) break;
          local_1348 = _strchr(local_1010 + (int)local_1354,10);
          if (local_1348 == (char *)0x0) {
            ExceptionList = local_10;
            return;
          }
          local_1334 = FUN_005909d2();
          if (local_1334 == -1) {
            ExceptionList = local_10;
            return;
          }
          local_1354 = local_1348 + (local_1330 - (int)local_1010);
          local_1348 = _strchr(local_1010 + (int)local_1354,10);
          if (local_1348 == (char *)0x0) {
            ExceptionList = local_10;
            return;
          }
          _strncpy(local_1468,local_1010 + (int)local_1354,
                   (size_t)(local_1348 + (-(int)(local_1010 + (int)local_1354) - local_1330)));
          local_1354 = local_1348 + (local_1330 - (int)local_1010);
          local_1348 = _strchr(local_1010 + (int)local_1354,10);
          if (local_1348 == (char *)0x0) {
            ExceptionList = local_10;
            return;
          }
          local_1334 = FUN_005909d2();
          if (local_1334 == -1) {
            ExceptionList = local_10;
            return;
          }
          local_1354 = local_1348 + (local_1330 - (int)local_1010);
          local_1760 = 0;
          local_155e = 0;
          local_1432 = 0;
          FUN_0058ee30();
          for (local_1774 = _strlen(local_1668); (int)local_1774 < 0xc; local_1774 = local_1774 + 1)
          {
            local_1668[local_1774] = ' ';
          }
          FUN_0058ee30();
          FUN_0058ee30();
          for (local_1774 = _strlen(local_1668); (int)local_1774 < 0x46; local_1774 = local_1774 + 1
              ) {
            local_1668[local_1774] = ' ';
          }
          FUN_0058ee30();
          pvVar2 = operator_new(0xe50);
          local_8 = 0;
          if (pvVar2 == (void *)0x0) {
            local_1794 = 0;
          }
          else {
            pcStackY_38 = (char *)0x43960000;
            puStackY_3c = (undefined *)0x0;
            pcStackY_40 = (char *)0x0;
            pcStackY_44 = (char *)0xffffffaa;
            pcStackY_48 = local_1668;
            uStackY_4c = 0x47417f;
            local_1794 = FUN_00407203();
          }
          local_8 = 0xffffffff;
          local_166c = local_1794;
          FUN_00408d33();
        }
        iVar1 = _strcmp(local_122c,s_<title>_005b83cc);
        if (iVar1 != 0) {
          local_1348 = _strchr(local_1010 + (int)local_1354,10);
          if (local_1348 == (char *)0x0) {
            ExceptionList = local_10;
            return;
          }
          local_1334 = FUN_005909d2();
          if (local_1334 == -1) {
            ExceptionList = local_10;
            return;
          }
          local_1354 = local_1348 + (local_1330 - (int)local_1010);
        }
        iVar1 = _strcmp(local_122c,s_<title>_005b83d8);
        if (iVar1 == 0) {
          _memset(local_122c,0,0x100);
          local_1348 = _strchr(local_1010 + (int)local_1354,10);
          if (local_1348 != (char *)0x0) {
            local_1344 = local_1348 + (-(int)(local_1010 + (int)local_1354) - local_1330);
            _strncpy(local_122c,local_1010 + (int)local_1354,(size_t)local_1344);
            local_1354 = local_1348 + (local_1330 - (int)local_1010);
            (**(code **)(**(int **)(extraout_ECX + 0x27d64) + 0x80))();
            iVar1 = _strcmp(local_122c,s_<content>_005b83e0);
            if (iVar1 != 0) {
              local_1348 = _strchr(local_1010 + (int)local_1354,10);
              if (local_1348 == (char *)0x0) {
                ExceptionList = local_10;
                return;
              }
              pcStackY_38 = (char *)0x4743ea;
              local_1334 = FUN_005909d2();
              if (local_1334 == -1) {
                ExceptionList = local_10;
                return;
              }
              local_1354 = local_1348 + (local_1330 - (int)local_1010);
            }
            iVar1 = _strcmp(local_122c,s_<content>_005b83f0);
            if (iVar1 == 0) {
              for (local_1338 = 0; local_1338 < 0x14; local_1338 = local_1338 + 1) {
                pcStackY_38 = (char *)0x4744b2;
                _memset(local_122c,0,0x100);
                local_1348 = _strchr(local_1010 + (int)local_1354,10);
                if (local_1348 == (char *)0x0) {
                  ExceptionList = local_10;
                  return;
                }
                pcStackY_38 = (char *)0x474523;
                _strncpy(local_122c,local_1010 + (int)local_1354,
                         (size_t)(local_1348 + (-(int)(local_1010 + (int)local_1354) - local_1330)))
                ;
                local_1354 = local_1348 + (local_1330 - (int)local_1010);
                pvVar2 = operator_new(0xe50);
                local_8 = 1;
                if (pvVar2 != (void *)0x0) {
                  pcStackY_38 = (char *)0x0;
                  puStackY_3c = (undefined *)0x41800000;
                  pcStackY_40 = (char *)0x43960000;
                  pcStackY_44 = (char *)0x0;
                  pcStackY_48 = (char *)0x0;
                  uStackY_4c = 0xffffffff;
                  pcStackY_50 = local_122c;
                  uStackY_54 = 0x474590;
                  FUN_00407203();
                }
                local_8 = 0xffffffff;
                FUN_00408d33();
              }
              (**(code **)(**(int **)(*(int *)(extraout_ECX + 0x27d50) + 0x20c) + 0x80))();
            }
            else {
              FUN_00403df2();
              (**(code **)(**(int **)(extraout_ECX + 0x84) + 0x88))();
            }
          }
        }
        else {
          FUN_00403df2();
          (**(code **)(**(int **)(extraout_ECX + 0x84) + 0x88))();
        }
      }
    }
  }
  ExceptionList = local_10;
  return;
}

