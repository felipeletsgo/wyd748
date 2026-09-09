// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004754e0 | Name: FUN_004754e0


void FUN_004754e0(void)

{
  size_t sVar1;
  size_t sVar2;
  size_t sVar3;
  int iVar4;
  int extraout_ECX;
  int local_2124;
  char local_2120 [4096];
  char local_1120 [256];
  char local_1020 [16];
  char local_1010 [12];
  char local_1004 [4048];
  undefined4 uStackY_34;
  char *pcStackY_30;
  char *pcStackY_2c;
  char *pcStackY_28;
  undefined *puStackY_24;
  char *pcStackY_20;
  char *pcStackY_1c;
  undefined4 uStackY_18;
  uint uStackY_14;
  
  FUN_0058f730();
  if (-1 < *(int *)(extraout_ECX + 0x27d6c)) {
    uStackY_14 = 0x47551a;
    _memset(local_2120,0,0x1000);
    uStackY_14 = 0x475530;
    _memset(local_1004,0,0x1000);
    uStackY_14 = 0x475543;
    _memset(local_1020,0,0x10);
    uStackY_14 = 0x475556;
    _memset(local_1010,0,0xc);
    FUN_0058f078();
    FUN_0058f078();
    uStackY_14 = 0x4755a4;
    _memset(local_1120,0,0x100);
    uStackY_14 = 0x4755c8;
    FUN_0058f078();
    sVar1 = _strlen(local_1020);
    sVar2 = _strlen(local_1010);
    sVar3 = _strlen(local_1120);
    for (local_2124 = 0; (local_2124 < (int)sVar2 && (local_2124 < (int)sVar3));
        local_2124 = local_2124 + 1) {
      local_1010[local_2124] = local_1010[local_2124] + local_1120[local_2124] + -0x30;
    }
    for (local_2124 = 0; (local_2124 < (int)sVar1 && (local_2124 < (int)sVar3));
        local_2124 = local_2124 + 1) {
      local_1020[local_2124] = local_1020[local_2124] + local_1120[local_2124] + -0x30;
    }
    if (*(int *)(extraout_ECX + 0x27d70) < 0) {
      uStackY_14 = (uint)*(ushort *)(extraout_ECX + 0x27d74);
      uStackY_18 = *(undefined4 *)(DAT_013b71e8 + 0x1af4c);
      pcStackY_1c = local_1010;
      pcStackY_20 = local_1020;
      puStackY_24 = &DAT_00a39e78;
      pcStackY_28 = s__s_asp_id__s_pw__s_server__d_gui_005b863c;
      pcStackY_2c = local_2120;
      pcStackY_30 = (char *)0x4757dc;
      FUN_0058f078();
    }
    else {
      uStackY_14 = (uint)*(ushort *)(extraout_ECX + 0x27d74);
      uStackY_18 = *(undefined4 *)(DAT_013b71e8 + 0x1af4c);
      pcStackY_1c = local_1010;
      pcStackY_20 = local_1020;
      puStackY_24 = *(undefined **)(extraout_ECX + 0x27d70);
      pcStackY_28 = &DAT_00a39e78;
      pcStackY_2c = s__s_d_asp_id__s_pw__s_server__d_g_005b85f8;
      pcStackY_30 = local_2120;
      uStackY_34 = 0x475766;
      FUN_0058f078();
    }
    uStackY_14 = 0x4757f7;
    FUN_00557a41();
    uStackY_14 = 0x47580d;
    iVar4 = _strncmp(local_1004,s_false_005b8680,5);
    if (iVar4 == 0) {
      FUN_00403df2();
      (**(code **)(**(int **)(extraout_ECX + 0x84) + 0x88))();
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
      FUN_004729d4();
    }
  }
  return;
}

