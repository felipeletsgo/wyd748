// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004729d4 | Name: FUN_004729d4


void FUN_004729d4(undefined4 param_1,int param_2)

{
  int iVar1;
  void *pvVar2;
  int extraout_ECX;
  undefined4 local_1984;
  char local_1974 [11];
  undefined1 local_1969;
  char *local_1874;
  char local_1870 [12];
  undefined1 local_1864;
  undefined4 local_1770;
  char local_176c [11];
  undefined1 local_1761;
  undefined1 local_166c [11];
  undefined1 local_1661;
  undefined1 local_156c [10];
  undefined1 local_1562;
  char local_146c [41];
  undefined1 local_1443;
  int local_136c;
  char *local_1368;
  size_t local_1364;
  undefined4 local_1360;
  size_t local_135c;
  char *local_1358;
  size_t local_1354;
  int local_1350;
  int local_134c;
  int local_1348;
  char local_1344 [256];
  undefined4 local_1244;
  undefined4 local_1240;
  char local_123c [256];
  char local_113c [256];
  undefined4 local_103c;
  undefined4 local_1038;
  char local_1034 [16];
  undefined4 local_1024;
  char local_1020 [12];
  undefined4 local_1014;
  char local_1010 [4056];
  undefined4 uStackY_38;
  char *pcStackY_34;
  char *pcStackY_30;
  char *pcStackY_2c;
  char *pcStackY_28;
  char *pcStackY_24;
  char *pcStackY_20;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f74b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0058f730();
  if (*(int *)(extraout_ECX + 0x27d4c) != 0) {
    if (*(short *)(DAT_013b71e8 + 0x7a8) == 0) {
      if (*(int *)(extraout_ECX + 0x27d88) != 0) {
        (**(code **)(**(int **)(extraout_ECX + 0x27d88) + 0x60))();
      }
    }
    else if (*(int *)(extraout_ECX + 0x27d88) != 0) {
      (**(code **)(**(int **)(extraout_ECX + 0x27d88) + 0x60))();
    }
    local_1348 = 0;
    local_134c = 0;
    local_1368 = (char *)0x0;
    pcStackY_20 = (char *)0x472ab0;
    _memset(local_1010,0,0x1000);
    pcStackY_20 = (char *)0x472ac3;
    _memset(local_1034,0,0x10);
    pcStackY_20 = (char *)0x472ad6;
    _memset(local_1020,0,0xc);
    FUN_0058f078();
    FUN_0058f078();
    pcStackY_20 = (char *)0x472b22;
    _memset(local_113c,0,0x100);
    pcStackY_20 = (char *)0x472b46;
    FUN_0058f078();
    local_1354 = _strlen(local_1034);
    local_135c = _strlen(local_1020);
    local_1364 = _strlen(local_113c);
    for (local_1350 = 0; (local_1350 < (int)local_135c && (local_1350 < (int)local_1364));
        local_1350 = local_1350 + 1) {
      local_1020[local_1350] = local_1020[local_1350] + local_113c[local_1350] + -0x30;
    }
    for (local_1350 = 0; (local_1350 < (int)local_1354 && (local_1350 < (int)local_1364));
        local_1350 = local_1350 + 1) {
      local_1034[local_1350] = local_1034[local_1350] + local_113c[local_1350] + -0x30;
    }
    if (*(int *)(extraout_ECX + 0x27d70) < 0) {
      pcStackY_20 = local_1020;
      pcStackY_24 = local_1034;
      pcStackY_28 = &DAT_00a39bf8;
      pcStackY_2c = s__s_asp_id__s_pw__s_server__d_gui_005b82a0;
      pcStackY_30 = local_1344;
      pcStackY_34 = (char *)0x472cff;
      FUN_0058f078();
    }
    else {
      pcStackY_20 = local_1020;
      pcStackY_24 = local_1034;
      pcStackY_28 = *(char **)(extraout_ECX + 0x27d70);
      pcStackY_2c = &DAT_00a39bf8;
      pcStackY_30 = s__s_d_asp_id__s_pw__s_server__d_g_005b8270;
      pcStackY_34 = local_1344;
      uStackY_38 = 0x472cb5;
      FUN_0058f078();
    }
    if (param_2 == 0) {
      pcStackY_20 = (char *)0x472d20;
      FUN_00557a41();
    }
    else {
      pcStackY_20 = (char *)0x472d3a;
      FUN_0058f220();
    }
    *(undefined4 *)(extraout_ECX + 0x27d6c) = 0xffffffff;
    FUN_004091ab();
    local_1358 = _strchr(local_1010 + (int)local_1368,10);
    if (local_1358 == (char *)0x0) {
      FUN_00403df2();
      (**(code **)(**(int **)(extraout_ECX + 0x84) + 0x88))();
    }
    else {
      pcStackY_20 = (undefined1 *)0x472de8;
      local_134c = FUN_005909d2();
      if (local_134c == -1) {
        FUN_00403df2();
        (**(code **)(**(int **)(extraout_ECX + 0x84) + 0x88))();
      }
      else {
        if (local_1358[-1] == '\r') {
          local_1348 = 1;
        }
        local_1368 = local_1358 + (local_1348 - (int)local_1010);
        iVar1 = _strcmp(local_123c,s_<table>_005b82d4);
        if ((iVar1 == 0) || (iVar1 = _strcmp(local_123c,s_<page>_005b82dc), iVar1 == 0)) {
          iVar1 = _strcmp(local_123c,s_<page>_005b82e4);
          if (iVar1 != 0) {
            for (local_136c = 0; local_136c < 0xf; local_136c = local_136c + 1) {
              pcStackY_20 = (char *)0x472f3c;
              _memset(local_176c,0,0x100);
              pcStackY_20 = (char *)0x472f52;
              _memset(local_1974,0,0x100);
              pcStackY_20 = (char *)0x472f68;
              _memset(local_146c,0,0x100);
              pcStackY_20 = (char *)0x472f7e;
              _memset(local_1870,0,0x100);
              pcStackY_20 = (char *)0x472f94;
              _memset(local_166c,0,0x100);
              pcStackY_20 = (char *)0x472faa;
              _memset(local_156c,0,0x100);
              local_1358 = _strchr(local_1010 + (int)local_1368,10);
              if (local_1358 == (char *)0x0) {
                ExceptionList = local_10;
                return;
              }
              pcStackY_20 = (undefined1 *)0x472ff8;
              local_134c = FUN_005909d2();
              if (local_134c == -1) {
                ExceptionList = local_10;
                return;
              }
              local_1368 = local_1358 + (local_1348 - (int)local_1010);
              iVar1 = _strcmp(local_123c,&DAT_005b82f0);
              if (iVar1 != 0) break;
              local_1358 = _strchr(local_1010 + (int)local_1368,10);
              if (local_1358 == (char *)0x0) {
                ExceptionList = local_10;
                return;
              }
              pcStackY_20 = (char *)0x473091;
              local_134c = FUN_005909d2();
              if (local_134c == -1) {
                ExceptionList = local_10;
                return;
              }
              local_1368 = local_1358 + (local_1348 - (int)local_1010);
              local_1358 = _strchr(local_1010 + (int)local_1368,10);
              if (local_1358 == (char *)0x0) {
                ExceptionList = local_10;
                return;
              }
              pcStackY_20 = (char *)0x47310d;
              local_134c = FUN_005909d2();
              if (local_134c == -1) {
                ExceptionList = local_10;
                return;
              }
              local_1368 = local_1358 + (local_1348 - (int)local_1010);
              local_1358 = _strchr(local_1010 + (int)local_1368,10);
              if (local_1358 == (char *)0x0) {
                ExceptionList = local_10;
                return;
              }
              local_1874 = local_1358 + (-(int)(local_1010 + (int)local_1368) - local_1348);
              pcStackY_20 = (char *)0x4731ac;
              _strncpy(local_146c,local_1010 + (int)local_1368,(size_t)local_1874);
              local_1368 = local_1358 + (local_1348 - (int)local_1010);
              local_1358 = _strchr(local_1010 + (int)local_1368,10);
              if (local_1358 == (char *)0x0) {
                ExceptionList = local_10;
                return;
              }
              pcStackY_20 = (char *)0x473214;
              local_134c = FUN_005909d2();
              if (local_134c == -1) {
                ExceptionList = local_10;
                return;
              }
              local_1368 = local_1358 + (local_1348 - (int)local_1010);
              local_1358 = _strchr(local_1010 + (int)local_1368,10);
              if (local_1358 == (char *)0x0) {
                ExceptionList = local_10;
                return;
              }
              pcStackY_20 = (char *)0x473290;
              local_134c = FUN_005909d2();
              if (local_134c == -1) {
                ExceptionList = local_10;
                return;
              }
              local_1368 = local_1358 + (local_1348 - (int)local_1010);
              local_1358 = _strchr(local_1010 + (int)local_1368,10);
              if (local_1358 == (char *)0x0) {
                ExceptionList = local_10;
                return;
              }
              pcStackY_20 = (undefined1 *)0x47330c;
              local_134c = FUN_005909d2();
              if (local_134c == -1) {
                ExceptionList = local_10;
                return;
              }
              local_1368 = local_1358 + (local_1348 - (int)local_1010);
              local_1761 = 0;
              local_1969 = 0;
              local_1443 = 0;
              local_1864 = 0;
              local_1661 = 0;
              local_1562 = 0;
              pvVar2 = operator_new(0xf60);
              local_8 = 0;
              if (pvVar2 == (void *)0x0) {
                local_1984 = 0;
              }
              else {
                pcStackY_20 = local_166c;
                pcStackY_24 = local_1870;
                pcStackY_28 = local_146c;
                pcStackY_2c = local_1974;
                pcStackY_30 = local_176c;
                pcStackY_34 = (char *)0x4733c6;
                local_1984 = FUN_00407d62();
              }
              local_8 = 0xffffffff;
              local_1770 = local_1984;
              FUN_00408d33();
            }
          }
          iVar1 = _strcmp(local_123c,s_<page>_005b830c);
          if (iVar1 != 0) {
            local_1358 = _strchr(local_1010 + (int)local_1368,10);
            if (local_1358 == (char *)0x0) {
              ExceptionList = local_10;
              return;
            }
            pcStackY_20 = (undefined1 *)0x473477;
            local_134c = FUN_005909d2();
            if (local_134c == -1) {
              ExceptionList = local_10;
              return;
            }
            local_1368 = local_1358 + (local_1348 - (int)local_1010);
          }
          iVar1 = _strcmp(local_123c,s_<page>_005b8318);
          if (iVar1 == 0) {
            local_1244 = 0;
            local_1014 = 0;
            local_1360 = 0;
            local_103c = 0;
            local_1038 = 0;
            local_1240 = 0;
            local_1024 = 0;
            pcStackY_20 = (char *)&local_103c;
            pcStackY_24 = (char *)&local_1360;
            pcStackY_28 = (char *)&local_1014;
            pcStackY_2c = (char *)&local_1244;
            pcStackY_30 = s__d__d__d__d__d__d__d_005b8320;
            pcStackY_34 = local_1010 + (int)local_1368;
            uStackY_38 = 0x473554;
            local_134c = FUN_005909d2();
            if (local_134c != -1) {
              pcStackY_20 = (char *)local_1014;
              pcStackY_24 = (char *)local_1024;
              pcStackY_28 = (char *)local_1240;
              pcStackY_2c = (char *)local_1038;
              pcStackY_30 = (char *)0x4735aa;
              FUN_004085cd();
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

