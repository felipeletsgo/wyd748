// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00474ce2 | Name: FUN_00474ce2


void FUN_00474ce2(undefined4 param_1)

{
  char *_Str;
  size_t sVar1;
  size_t sVar2;
  size_t sVar3;
  int *piVar4;
  int iVar5;
  int extraout_ECX;
  undefined1 local_3128 [4096];
  int local_2128;
  char local_2124 [4096];
  char local_1124 [256];
  char *local_1024;
  char local_1020 [16];
  char local_1010 [12];
  char local_1004 [4040];
  undefined4 uStackY_3c;
  char *pcStackY_38;
  char *pcStackY_34;
  char *pcStackY_30;
  char *pcStackY_2c;
  char *pcStackY_28;
  char *pcStackY_24;
  char *pcStackY_20;
  uint uStackY_1c;
  uint uStackY_18;
  int iStackY_14;
  
  FUN_0058f730();
  if (((*(int *)(extraout_ECX + 0x27d58) != 0) && (*(int *)(extraout_ECX + 0x27d60) != 0)) &&
     (*(int *)(*(int *)(extraout_ECX + 0x27d58) + 0x208) != 0)) {
    local_1024 = (char *)(**(code **)(**(int **)(extraout_ECX + 0x27d60) + 0x88))();
    _Str = (char *)(**(code **)(**(int **)(*(int *)(extraout_ECX + 0x27d58) + 0x208) + 0x88))();
    if (((local_1024 != (char *)0x0) && (sVar1 = _strlen(local_1024), sVar1 != 0)) &&
       ((0 < *(int *)(*(int *)(extraout_ECX + 0x27d58) + 0x210) ||
        (sVar1 = _strlen(_Str), sVar1 != 0)))) {
      iStackY_14 = 0x474df0;
      _memset(local_2124,0,0x1000);
      iStackY_14 = 0x474e06;
      _memset(local_3128,0,0x1000);
      iStackY_14 = 0x474e1c;
      _memset(local_1004,0,0x1000);
      iStackY_14 = 0x474e2f;
      _memset(local_1020,0,0x10);
      iStackY_14 = 0x474e42;
      _memset(local_1010,0,0xc);
      FUN_0058f078();
      FUN_0058f078();
      iStackY_14 = 0x474e8e;
      _memset(local_1124,0,0x100);
      iStackY_14 = 0x474eb2;
      FUN_0058f078();
      sVar1 = _strlen(local_1020);
      sVar2 = _strlen(local_1010);
      sVar3 = _strlen(local_1124);
      for (local_2128 = 0; (local_2128 < (int)sVar2 && (local_2128 < (int)sVar3));
          local_2128 = local_2128 + 1) {
        local_1010[local_2128] = local_1010[local_2128] + local_1124[local_2128] + -0x30;
      }
      for (local_2128 = 0; (local_2128 < (int)sVar1 && (local_2128 < (int)sVar3));
          local_2128 = local_2128 + 1) {
        local_1020[local_2128] = local_1020[local_2128] + local_1124[local_2128] + -0x30;
      }
      for (local_2128 = 0; local_2128 < *(int *)(*(int *)(extraout_ECX + 0x27d58) + 0x210);
          local_2128 = local_2128 + 1) {
        piVar4 = (int *)FUN_00409046();
        (**(code **)(*piVar4 + 0x88))();
        FUN_0058ee30();
        FUN_0058ee30();
      }
      if (*(int *)(*(int *)(extraout_ECX + 0x27d58) + 0x208) != 0) {
        FUN_0058ee30();
      }
      FUN_0058ee30();
      if (*(int *)(extraout_ECX + 0x27d70) < 0) {
        if (*(int *)(extraout_ECX + 0x27d6c) < 0) {
          iStackY_14 = param_1;
          uStackY_18 = (uint)*(ushort *)(extraout_ECX + 0x27d74);
          uStackY_1c = *(uint *)(DAT_013b71e8 + 0x1af4c);
          pcStackY_20 = local_1010;
          pcStackY_24 = local_1020;
          pcStackY_28 = &DAT_00a39df8;
          pcStackY_2c = s__s_asp_id__s_pw__s_server__d_gui_005b85a0;
          pcStackY_30 = local_2124;
          pcStackY_34 = (char *)0x4752b5;
          FUN_0058f078();
        }
        else {
          iStackY_14 = *(int *)(extraout_ECX + 0x4c) + 0x168;
          uStackY_18 = *(uint *)(extraout_ECX + 0x27d6c);
          uStackY_1c = (uint)*(ushort *)(extraout_ECX + 0x27d74);
          pcStackY_20 = *(char **)(DAT_013b71e8 + 0x1af4c);
          pcStackY_24 = local_1010;
          pcStackY_28 = local_1020;
          pcStackY_2c = &DAT_00a39d78;
          pcStackY_30 = s__s_asp_id__s_pw__s_server__d_gui_005b8548;
          pcStackY_34 = local_2124;
          pcStackY_38 = (char *)0x47524d;
          FUN_0058f078();
        }
      }
      else if (*(int *)(extraout_ECX + 0x27d6c) < 0) {
        iStackY_14 = param_1;
        uStackY_18 = (uint)*(ushort *)(extraout_ECX + 0x27d74);
        uStackY_1c = *(uint *)(DAT_013b71e8 + 0x1af4c);
        pcStackY_20 = local_1010;
        pcStackY_24 = local_1020;
        pcStackY_28 = *(char **)(extraout_ECX + 0x27d70);
        pcStackY_2c = &DAT_00a39df8;
        pcStackY_30 = s__s_d_asp_id__s_pw__s_server__d_g_005b84fc;
        pcStackY_34 = local_2124;
        pcStackY_38 = (char *)0x4751b4;
        FUN_0058f078();
      }
      else {
        iStackY_14 = *(int *)(extraout_ECX + 0x4c) + 0x168;
        uStackY_18 = *(uint *)(extraout_ECX + 0x27d6c);
        uStackY_1c = (uint)*(ushort *)(extraout_ECX + 0x27d74);
        pcStackY_20 = *(char **)(DAT_013b71e8 + 0x1af4c);
        pcStackY_24 = local_1010;
        pcStackY_28 = local_1020;
        pcStackY_2c = *(char **)(extraout_ECX + 0x27d70);
        pcStackY_30 = &DAT_00a39d78;
        pcStackY_34 = s__s_d_asp_id__s_pw__s_server__d_g_005b84a4;
        pcStackY_38 = local_2124;
        uStackY_3c = 0x475140;
        FUN_0058f078();
      }
      iStackY_14 = 0x4752d0;
      FUN_00557a41();
      iStackY_14 = 0x4752e6;
      iVar5 = _strncmp(local_1004,s_false_005b85ec,5);
      if (iVar5 == 0) {
        FUN_00403df2();
        (**(code **)(**(int **)(extraout_ECX + 0x84) + 0x88))();
      }
      else if (*(int *)(extraout_ECX + 0x27d6c) < 0) {
        if (*(int *)(extraout_ECX + 0x27d40) != 0) {
          (**(code **)(**(int **)(extraout_ECX + 0x27d40) + 0x60))();
        }
        if (*(int *)(extraout_ECX + 0x27d48) != 0) {
          (**(code **)(**(int **)(extraout_ECX + 0x27d48) + 0x60))();
        }
        if (*(int *)(extraout_ECX + 0x27d44) != 0) {
          (**(code **)(**(int **)(extraout_ECX + 0x27d44) + 0x60))();
        }
        if (*(int *)(extraout_ECX + 0x27d4c) != 0) {
          *(undefined4 *)(*(int *)(extraout_ECX + 0x27d4c) + 0x1fc) = 0xffffffff;
        }
        FUN_004729d4();
      }
      else {
        if (*(int *)(extraout_ECX + 0x27d40) != 0) {
          (**(code **)(**(int **)(extraout_ECX + 0x27d40) + 0x60))();
        }
        if (*(int *)(extraout_ECX + 0x27d48) != 0) {
          (**(code **)(**(int **)(extraout_ECX + 0x27d48) + 0x60))();
        }
        if (*(int *)(extraout_ECX + 0x27d44) != 0) {
          (**(code **)(**(int **)(extraout_ECX + 0x27d44) + 0x60))();
        }
        if (*(int *)(extraout_ECX + 0x27d4c) != 0) {
          *(undefined4 *)(*(int *)(extraout_ECX + 0x27d4c) + 0x1fc) = 0xffffffff;
        }
        FUN_004735ba();
      }
    }
  }
  return;
}

