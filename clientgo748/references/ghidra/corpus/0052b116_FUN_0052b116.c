// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052b116 | Name: FUN_0052b116


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_0052b116(int *param_1,int param_2)

{
  int iVar1;
  void *pvVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  undefined4 uVar15;
  int local_b4;
  int *local_b0;
  int local_ac;
  undefined1 local_68 [32];
  undefined4 local_48;
  int local_44;
  int local_40;
  int local_3c;
  int *local_38;
  int local_34;
  int local_30;
  ushort local_2c [4];
  int local_24;
  ushort local_20 [4];
  float local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a238d;
  local_10 = ExceptionList;
  local_30 = param_2;
  ExceptionList = &local_10;
  _memset(local_20,0,8);
  local_20[0] = *(ushort *)(local_30 + 0xc) & 0xfff;
  if ((uint)*(ushort *)((int)param_1 + 0x1d2) ==
      (int)*(short *)(&DAT_00d44990 + (*(ushort *)(local_30 + 0xc) & 0xfff) * 0x8c)) {
    iVar1 = FUN_0054cd07(local_20,0x12);
    if (iVar1 == param_1[0x93]) goto LAB_0052b46c;
  }
  pvVar2 = operator_new(0x90);
  local_8 = 0;
  if (pvVar2 == (void *)0x0) {
    local_ac = 0;
  }
  else {
    uVar14 = 1000;
    puVar3 = (undefined4 *)FUN_00430f50(0,0,0);
    uVar13 = *puVar3;
    uVar15 = puVar3[1];
    uVar12 = puVar3[2];
    uVar11 = 1;
    uVar10 = 0x3f800000;
    uVar9 = 0x38;
    uVar8 = 1;
    uVar7 = 0;
    uVar6 = 0x40400000;
    uVar5 = 10;
    uVar4 = 1;
    puVar3 = (undefined4 *)
             FUN_00430f50(param_1[10],(float)param_1[0xc] + _DAT_005a3660,param_1[0xb]);
    local_ac = FUN_004d4de0(*puVar3,puVar3[1],puVar3[2],uVar4,uVar5,uVar6,uVar7,uVar8,uVar9,uVar10,
                            uVar11,uVar13,uVar15,uVar12,uVar14);
  }
  local_8 = 0xffffffff;
  local_34 = local_ac;
  if (local_ac != 0) {
    FUN_0054ac09(local_ac);
  }
  if (DAT_005ccf98 != 0) {
    iVar1 = FUN_00429a6d(0x9e);
    if (iVar1 != 0) {
      uVar15 = 0;
      uVar13 = 0;
      FUN_00429a6d(0x9e);
      FUN_0042ad2b(uVar13,uVar15);
    }
  }
  pvVar2 = operator_new(0xc0);
  local_8 = 1;
  if (pvVar2 == (void *)0x0) {
    local_b0 = (int *)0x0;
  }
  else {
    local_b0 = (int *)FUN_004df250(4,7,0x3f800000);
  }
  local_8 = 0xffffffff;
  local_38 = local_b0;
  if (local_b0 != (int *)0x0) {
    local_b0[0x1b] = 1;
    (**(code **)(*local_b0 + 0x3c))(param_1[10],param_1[0xb]);
    local_38[0x28] = 3000;
    (**(code **)(*local_38 + 0x44))(0xaaaaaaaa);
    FUN_0054ac09(local_38);
  }
  FUN_0051dba4(0xe,0);
  if (*(char *)((int)param_1 + 0x795) == '\0') {
    pvVar2 = operator_new(0x90);
    local_8 = 2;
    if (pvVar2 == (void *)0x0) {
      local_b4 = 0;
    }
    else {
      uVar13 = 0;
      puVar3 = (undefined4 *)FUN_00430f50(param_1[10],param_1[0xc],param_1[0xb]);
      local_b4 = FUN_004d19a0(*puVar3,puVar3[1],puVar3[2],uVar13);
    }
    local_8 = 0xffffffff;
    local_3c = local_b4;
    if (local_b4 != 0) {
      FUN_0054ac09(local_b4);
    }
  }
LAB_0052b46c:
  iVar1 = DAT_013b71e8;
  if (*(int **)(DAT_0067cf38 + 0x4c) == param_1) {
    local_40 = DAT_013b71e8 + 0x6ec;
    *(ushort *)(DAT_013b71e8 + 0x748) = *(ushort *)(local_30 + 0xc) & 0xfff;
    if ((char)param_1[0x1e7] == '\x01') {
      local_48 = FUN_0054cd07(iVar1 + 0x7b8,0x50);
      (**(code **)(*(int *)param_1[0x10e] + 0x88))(local_48);
      local_44 = DAT_0067cf38;
      if ((DAT_005b892c == 2) && (DAT_0067cf38 != 0)) {
        (**(code **)(**(int **)(DAT_0067cf38 + 0x27ad4) + 0x88))(local_48);
        FUN_0058f078(local_68,&DAT_005bb800,local_48);
        (**(code **)(**(int **)(local_44 + 0x27ac0) + 0x80))(local_68,0);
      }
    }
  }
  FUN_00526140(local_30 + 0xc);
  FUN_00527032(local_30 + 0x2c);
  local_18 = (float)(int)*(short *)((int)param_1 + 0x45a);
  FUN_005277a7((float)(int)*(short *)((int)param_1 + 0x45a));
  FUN_004faf13(*(ushort *)(local_30 + 0xc) & 0xfff);
  _memset(local_2c,0,8);
  local_2c[0] = *(ushort *)(local_30 + 0x18) & 0xfff;
  local_14 = FUN_0054cd07(local_2c,0x15);
  if (local_14 == 0x29) {
    *(undefined2 *)((int)param_1 + 0x1ea) = *(undefined2 *)((int)param_1 + 0x1ee);
    *(short *)(param_1 + 0x7b) = (short)param_1[0x7c];
    *(undefined1 *)(param_1 + 0x7e) = *(undefined1 *)((int)param_1 + 0x1f9);
    *(undefined1 *)(param_1 + 0x80) = *(undefined1 *)((int)param_1 + 0x201);
  }
  (**(code **)(*param_1 + 0x38))();
  FUN_0051bb41(*(ushort *)(local_30 + 0x18) & 0xfff,*(ushort *)(local_30 + 0x1a) & 0xfff);
  (**(code **)(*param_1 + 0x40))(0,param_1[0xd],0);
  local_24 = 0;
  iVar1 = FUN_00494dcf();
  if (iVar1 == 30000) {
    local_24 = DAT_0067cf38;
  }
  if (local_24 != 0) {
    FUN_004431e4(0);
  }
  DAT_005b12bc = 0xffff;
  ExceptionList = local_10;
  return 1;
}

