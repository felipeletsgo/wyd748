// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00417d9b | Name: FUN_00417d9b


void __thiscall FUN_00417d9b(undefined4 param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  undefined1 uVar3;
  int iVar4;
  undefined1 local_114 [256];
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  if (((DAT_005b12c0 != 0) && (-1 < param_2)) && (param_2 < 0xc)) {
    piVar1 = *(int **)(DAT_0067cf38 + 0x278d8);
    local_10 = 0;
    iVar2 = (int)*(short *)(DAT_013b71e8 + 0x9f0 + param_2 * 0x34);
    local_8 = -1;
    if (iVar2 % 10 < 6) {
      iVar2 = iVar2 / 10;
    }
    else {
      iVar2 = iVar2 % 10 + -6;
      local_8 = (int)*(short *)(DAT_013b71e8 + 0x9f0 + param_2 * 0x34) / 10;
    }
    iVar4 = DAT_0067cf38;
    FUN_0058f078(local_114,&DAT_005b12d0,&DAT_00dba434,param_1,DAT_0067cf38,iVar2);
    (**(code **)(*piVar1 + 0x80))(local_114,0);
    (**(code **)(*piVar1 + 0x84))(0xffffffaa);
    if (local_8 < 0) {
      FUN_0058f078(local_114,s__s____s_005b12d4,&DAT_00a365f8,&DAT_00a34178 + (iVar2 + 0x79) * 0x80)
      ;
    }
    else {
      FUN_0058f078(local_114,s__s____s__s__005b12dc,&DAT_00a365f8,
                   &DAT_00a34178 + (iVar2 + 0x79) * 0x80,&DAT_00a34178 + (local_8 + 0x79) * 0x80);
    }
    (**(code **)(**(int **)(iVar4 + 0x278e0 + local_10 * 4) + 0x80))(local_114,0);
    (**(code **)(**(int **)(iVar4 + 0x278e0 + local_10 * 4) + 0x84))(0xffaaffff);
    local_10 = local_10 + 1;
    FUN_0058f078(local_114,s__s____d_005b12e8,&DAT_00a394f8,
                 *(short *)(DAT_013b71e8 + 0x9f2 + param_2 * 0x34) + 1);
    (**(code **)(**(int **)(iVar4 + 0x278e0 + local_10 * 4) + 0x80))(local_114,0);
    (**(code **)(**(int **)(iVar4 + 0x278e0 + local_10 * 4) + 0x84))(0xffffffff);
    local_10 = local_10 + 1;
    FUN_0058f078(local_114,s__s____d_005b12f0,&DAT_00a37378,
                 (int)*(short *)(DAT_013b71e8 + 0x9f4 + param_2 * 0x34));
    (**(code **)(**(int **)(iVar4 + 0x278e0 + local_10 * 4) + 0x80))(local_114,0);
    (**(code **)(**(int **)(iVar4 + 0x278e0 + local_10 * 4) + 0x84))(0xffffffff);
    local_10 = local_10 + 1;
    FUN_0058f078(local_114,s__s____d_005b12f8,&DAT_00a373f8,
                 (int)*(short *)(DAT_013b71e8 + 0x9f6 + param_2 * 0x34));
    (**(code **)(**(int **)(iVar4 + 0x278e0 + local_10 * 4) + 0x80))(local_114,0);
    (**(code **)(**(int **)(iVar4 + 0x278e0 + local_10 * 4) + 0x84))(0xffffffff);
    local_10 = local_10 + 1;
    FUN_0058f078(local_114,s__s____d_005b1300,&DAT_00a37478,
                 (int)*(short *)(DAT_013b71e8 + 0x9f8 + param_2 * 0x34));
    (**(code **)(**(int **)(iVar4 + 0x278e0 + local_10 * 4) + 0x80))(local_114,0);
    (**(code **)(**(int **)(iVar4 + 0x278e0 + local_10 * 4) + 0x84))(0xffffffff);
    local_10 = local_10 + 1;
    FUN_0058f078(local_114,s__s____d_005b1308,&DAT_00a374f8,
                 (int)*(short *)(DAT_013b71e8 + 0x9fa + param_2 * 0x34));
    (**(code **)(**(int **)(iVar4 + 0x278e0 + local_10 * 4) + 0x80))(local_114,0);
    (**(code **)(**(int **)(iVar4 + 0x278e0 + local_10 * 4) + 0x84))(0xffffffff);
    local_10 = local_10 + 1;
    local_14 = (int)*(short *)(DAT_013b71e8 + 0xa00 + param_2 * 0x34);
    local_c = (int)*(short *)(DAT_013b71e8 + 0xa02 + param_2 * 0x34);
    iVar2 = (int)*(short *)(DAT_013b71e8 + 0xa04 + param_2 * 0x34);
    _memset(local_114,0,4);
    if ((0 < local_14) && (local_14 < 0x6e)) {
      FUN_0058ee30(local_114,&DAT_00d44950 + (local_14 + 5000) * 0x8c);
    }
    FUN_0058ee30(local_114,&DAT_005b1310);
    if ((0 < local_c) && (local_c < 0x6e)) {
      FUN_0058ee30(local_114,&DAT_00d44950 + (local_c + 5000) * 0x8c);
    }
    FUN_0058ee30(local_114,&DAT_005b1314);
    if ((0 < iVar2) && (iVar2 < 0x6e)) {
      FUN_0058ee30(local_114,&DAT_00d44950 + (iVar2 + 5000) * 0x8c);
    }
    (**(code **)(**(int **)(iVar4 + 0x278e0 + local_10 * 4) + 0x80))(local_114,0);
    (**(code **)(**(int **)(iVar4 + 0x278e0 + local_10 * 4) + 0x84))(0xffffbbff);
    local_10 = local_10 + 1;
    local_14 = (int)*(short *)(DAT_013b71e8 + 0xa06 + param_2 * 0x34);
    local_c = (int)*(short *)(DAT_013b71e8 + 0xa08 + param_2 * 0x34);
    iVar2 = (int)*(short *)(DAT_013b71e8 + 0xa0a + param_2 * 0x34);
    _memset(local_114,0,4);
    if ((0 < local_14) && (local_14 < 0x6e)) {
      FUN_0058ee30(local_114,&DAT_00d44950 + (local_14 + 5000) * 0x8c);
    }
    FUN_0058ee30(local_114,&DAT_005b1318);
    if ((0 < local_c) && (local_c < 0x6e)) {
      FUN_0058ee30(local_114,&DAT_00d44950 + (local_c + 5000) * 0x8c);
    }
    FUN_0058ee30(local_114,&DAT_005b131c);
    if ((0 < iVar2) && (iVar2 < 0x6e)) {
      FUN_0058ee30(local_114,&DAT_00d44950 + (iVar2 + 5000) * 0x8c);
    }
    (**(code **)(**(int **)(iVar4 + 0x278e0 + local_10 * 4) + 0x80))(local_114,0);
    (**(code **)(**(int **)(iVar4 + 0x278e0 + local_10 * 4) + 0x84))(0xffffbbff);
    local_10 = local_10 + 1;
    local_14 = (int)*(short *)(DAT_013b71e8 + 0xa0c + param_2 * 0x34);
    local_c = (int)*(short *)(DAT_013b71e8 + 0xa0e + param_2 * 0x34);
    iVar2 = (int)*(short *)(DAT_013b71e8 + 0xa10 + param_2 * 0x34);
    _memset(local_114,0,4);
    if ((0 < local_14) && (local_14 < 0x6e)) {
      FUN_0058ee30(local_114,&DAT_00d44950 + (local_14 + 5000) * 0x8c);
    }
    FUN_0058ee30(local_114,&DAT_005b1320);
    if ((0 < local_c) && (local_c < 0x6e)) {
      FUN_0058ee30(local_114,&DAT_00d44950 + (local_c + 5000) * 0x8c);
    }
    FUN_0058ee30(local_114,&DAT_005b1324);
    if ((0 < iVar2) && (iVar2 < 0x6e)) {
      FUN_0058ee30(local_114,&DAT_00d44950 + (iVar2 + 5000) * 0x8c);
    }
    (**(code **)(**(int **)(iVar4 + 0x278e0 + local_10 * 4) + 0x80))(local_114,0);
    (**(code **)(**(int **)(iVar4 + 0x278e0 + local_10 * 4) + 0x84))(0xffffbbff);
    local_10 = local_10 + 1;
    uVar3 = (*(ushort *)(DAT_013b71e8 + 0xa12 + param_2 * 0x34) & 1) != 0;
    if ((*(ushort *)(DAT_013b71e8 + 0xa12 + param_2 * 0x34) & 0x11) != 0) {
      uVar3 = 2;
    }
    if ((*(ushort *)(DAT_013b71e8 + 0xa12 + param_2 * 0x34) & 0x111) != 0) {
      uVar3 = 3;
    }
    if ((*(ushort *)(DAT_013b71e8 + 0xa12 + param_2 * 0x34) & 0x1111) != 0) {
      uVar3 = 4;
    }
    FUN_0058f078(local_114,s__s____d_005b1328,&DAT_00a3f478,uVar3);
    (**(code **)(**(int **)(iVar4 + 0x278e0 + local_10 * 4) + 0x80))(local_114,0);
    (**(code **)(**(int **)(iVar4 + 0x278e0 + local_10 * 4) + 0x84))(0xffffffff);
  }
  return;
}

