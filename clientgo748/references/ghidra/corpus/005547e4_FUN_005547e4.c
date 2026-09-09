// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005547e4 | Name: FUN_005547e4


undefined4 FUN_005547e4(undefined4 param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  int local_148c;
  char local_1488 [62];
  undefined1 local_144a;
  undefined1 local_1449;
  undefined4 local_1388;
  size_t local_1384;
  undefined4 local_1380;
  int local_137c;
  int local_1378;
  undefined4 local_1374;
  int local_1370;
  undefined4 local_136c;
  undefined4 local_1368;
  undefined2 local_1364 [2];
  undefined1 local_1360 [4];
  undefined1 local_135c [4];
  undefined1 local_1358 [4];
  undefined1 local_1354 [4];
  undefined1 local_1350 [4];
  undefined1 local_134c [4];
  undefined1 local_1348 [4];
  char local_1344 [4];
  undefined4 local_1340 [3];
  undefined4 local_1334;
  char local_1330 [256];
  undefined1 local_1230 [256];
  undefined1 local_1130 [256];
  undefined1 local_1030 [256];
  undefined1 local_f30 [256];
  undefined1 local_e30 [256];
  undefined1 local_d30 [256];
  undefined1 local_c30 [256];
  undefined1 local_b30 [256];
  char local_a30 [768];
  undefined4 local_730;
  undefined4 local_72c;
  int local_728;
  CHAR local_724 [1024];
  char local_324 [256];
  undefined4 local_224;
  undefined4 local_220;
  undefined4 local_21c;
  char local_218 [256];
  undefined4 local_118;
  undefined4 local_114;
  undefined4 local_110;
  int local_10c;
  int local_108;
  undefined4 uStackY_94;
  CHAR *pCStackY_90;
  char *pcStackY_8c;
  int *piStackY_88;
  char *pcStackY_84;
  char *pcStackY_80;
  char *pcStackY_7c;
  undefined4 *puStackY_78;
  undefined4 *puStackY_74;
  undefined4 *puStackY_70;
  undefined4 *puStackY_6c;
  undefined4 *puStackY_68;
  char *pcStackY_64;
  undefined2 *puStackY_60;
  undefined1 *puStackY_5c;
  undefined1 *puStackY_58;
  undefined1 *puStackY_54;
  undefined1 *puStackY_50;
  undefined1 *puStackY_4c;
  undefined1 *puStackY_48;
  undefined1 *puStackY_44;
  undefined1 *puStackY_40;
  undefined1 *puStackY_3c;
  undefined1 *puStackY_38;
  undefined1 *puStackY_34;
  undefined1 *puStackY_30;
  undefined1 *puStackY_2c;
  undefined1 *puStackY_28;
  undefined1 *puStackY_24;
  char *pcStackY_20;
  char *pcStackY_1c;
  undefined4 *puStackY_18;
  
  FUN_0058f730();
  if (param_2 != 0) {
    FUN_0054f7b6();
    local_108 = FUN_0058f716();
    if (local_108 == 0) {
      puStackY_18 = (undefined4 *)0x554839;
      MessageBoxA((HWND)0x0,s_Can_t_write_extraitem_bin_005c43a0,s_ERROR_005c4398,0x1000);
      return 0;
    }
  }
  FUN_0058f078();
  local_10c = FUN_0058f716();
  if (local_10c == 0) {
    FUN_0058f078();
    local_10c = FUN_0058f716();
  }
  if (local_10c == 0) {
    uVar1 = 0;
  }
  else {
    while( true ) {
      local_728 = -1;
      _memset(local_1488,0,0x100);
      _memset(local_1330,0,0xc00);
      _memset(local_1364,0,0x30);
      local_1334 = 0x30;
      local_730 = 0xc00;
      local_1370 = FUN_00590a06();
      if (local_1370 == 0) break;
      for (local_1378 = 0; local_1378 < 0x400; local_1378 = local_1378 + 1) {
        if (local_724[local_1378] == ',') {
          local_724[local_1378] = ' ';
        }
        if (local_724[local_1378] == '\0') break;
      }
      local_218[0] = '\0';
      local_324[0] = '\0';
      local_224 = 0;
      local_220 = 0;
      local_1388 = 0;
      local_21c = 0;
      local_1380 = 0;
      puStackY_18 = local_1340;
      pcStackY_1c = local_a30;
      pcStackY_20 = local_1344;
      puStackY_24 = local_b30;
      puStackY_28 = local_1348;
      puStackY_2c = local_c30;
      puStackY_30 = local_134c;
      puStackY_34 = local_d30;
      puStackY_38 = local_1350;
      puStackY_3c = local_e30;
      puStackY_40 = local_1354;
      puStackY_44 = local_f30;
      puStackY_48 = local_1358;
      puStackY_4c = local_1030;
      puStackY_50 = local_135c;
      puStackY_54 = local_1130;
      puStackY_58 = local_1360;
      puStackY_5c = local_1230;
      puStackY_60 = local_1364;
      pcStackY_64 = local_1330;
      puStackY_68 = &local_21c;
      puStackY_6c = &local_1380;
      puStackY_70 = &local_1388;
      puStackY_74 = &local_220;
      puStackY_78 = &local_224;
      pcStackY_7c = local_324;
      pcStackY_80 = local_218;
      pcStackY_84 = local_1488;
      piStackY_88 = &local_728;
      pcStackY_8c = s__d__s__s__s__d__d__d__d__d__s__d_005c43dc;
      pCStackY_90 = local_724;
      uStackY_94 = 0x554aec;
      FUN_005909d2();
      if (local_728 != -1) {
        if (local_728 < 0x1964) {
          local_1384 = _strlen(local_1488);
          if (0x3e < (int)local_1384) {
            puStackY_18 = (undefined4 *)0x554b5b;
            MessageBoxA((HWND)0x0,s_too_long_Item_Name_005c4454,local_1488,0);
            local_1449 = 0;
            local_144a = 0;
          }
          FUN_0058ee20();
          local_1374 = 0;
          local_136c = 0;
          local_114 = 0;
          local_110 = 0;
          local_1368 = 0;
          local_72c = 0;
          local_118 = 0;
          if (local_218[0] == '.') {
            local_218[0] = ' ';
          }
          if (local_324[0] == '.') {
            local_324[0] = ' ';
          }
          puStackY_18 = (undefined4 *)0x554c16;
          FUN_005909d2();
          puStackY_18 = &local_114;
          pcStackY_1c = s__d__d__d__d__d_005c4470;
          pcStackY_20 = local_324;
          puStackY_24 = (undefined1 *)0x554c4d;
          FUN_005909d2();
          *(undefined2 *)(&DAT_00d44990 + local_728 * 0x8c) = (undefined2)local_1374;
          *(undefined2 *)(&DAT_00d44992 + local_728 * 0x8c) = (undefined2)local_136c;
          *(undefined2 *)(&DAT_00d44996 + local_728 * 0x8c) = (undefined2)local_114;
          *(undefined2 *)(&DAT_00d44998 + local_728 * 0x8c) = (undefined2)local_110;
          *(undefined2 *)(&DAT_00d4499a + local_728 * 0x8c) = (undefined2)local_1368;
          *(undefined2 *)(&DAT_00d4499c + local_728 * 0x8c) = (undefined2)local_72c;
          *(undefined2 *)(&DAT_00d4499e + local_728 * 0x8c) = (undefined2)local_118;
          *(undefined2 *)(&DAT_00d449d4 + local_728 * 0x8c) = (undefined2)local_224;
          *(undefined4 *)(&DAT_00d449d0 + local_728 * 0x8c) = local_220;
          *(undefined2 *)(&DAT_00d449d6 + local_728 * 0x8c) = (undefined2)local_1388;
          *(undefined2 *)(&DAT_00d449da + local_728 * 0x8c) = (undefined2)local_21c;
          *(undefined2 *)(local_728 * 0x8c + 0xd449d8) = (undefined2)local_1380;
          *(undefined2 *)(local_728 * 0x8c + 0xd44994) = 0;
          for (local_137c = 0; local_137c < 0xc; local_137c = local_137c + 1) {
            if (local_1330[local_137c * 0x100] != '\0') {
              local_148c = 0;
              while ((local_148c < 0x7f &&
                     (iVar2 = _strcmp(&DAT_00e32f2c + local_148c * 0x18,
                                      local_1330 + local_137c * 0x100), iVar2 != 0))) {
                local_148c = local_148c + 1;
              }
              if (local_137c == 0x7f) {
                puStackY_18 = (undefined4 *)0x554e57;
                MessageBoxA((HWND)0x0,&stack0x00006bd0,s_Unregisterd_effect_005c4480,0);
              }
              else {
                *(undefined2 *)(&DAT_00d449a0 + local_137c * 4 + local_728 * 0x8c) =
                     (undefined2)local_148c;
                *(undefined2 *)(&DAT_00d449a2 + local_137c * 4 + local_728 * 0x8c) =
                     local_1364[local_137c * 2];
              }
            }
          }
          if (param_2 != 0) {
            puStackY_18 = (undefined4 *)0x554ec7;
            FUN_005904b6();
            puStackY_18 = (undefined4 *)0x554eef;
            FUN_005904b6();
          }
        }
        else {
          puStackY_18 = (undefined4 *)0x554b22;
          MessageBoxA((HWND)0x0,local_724,s_check_MAX_ITEMLIST_005c4440,0);
        }
      }
    }
    FUN_0058f62c();
    if (param_2 != 0) {
      FUN_0058f62c();
      FUN_0054f7b6();
    }
    uVar1 = 1;
  }
  return uVar1;
}

