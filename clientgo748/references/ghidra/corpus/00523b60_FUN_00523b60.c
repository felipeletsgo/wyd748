// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00523b60 | Name: FUN_00523b60


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00523b60(int param_1)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  bool bVar6;
  int iVar7;
  void *pvVar8;
  undefined4 uVar9;
  undefined4 *puVar10;
  int iVar11;
  int iVar12;
  undefined4 uVar13;
  int local_120;
  int local_118;
  int local_114;
  int local_110;
  int local_10c;
  int *local_108;
  int *local_104;
  int *local_100;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  iVar7 = DAT_0067cf38;
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a221a;
  local_10 = ExceptionList;
  if (DAT_0067cf38 == 0) {
    return 0;
  }
  if (*(int *)(DAT_0067cf38 + 0x28e38) != 0) {
    fVar1 = *(float *)(*(int *)(DAT_0067cf38 + 0x28e38) + 0x74);
    fVar2 = *(float *)(*(int *)(DAT_0067cf38 + 0x28e38) + 0x7c);
    fVar3 = *(float *)(param_1 + 0x28);
    fVar5 = *(float *)(param_1 + 0x30) + _DAT_005a430c;
    fVar4 = *(float *)(param_1 + 0x2c);
    ExceptionList = &local_10;
    *(undefined4 *)(*(int *)(DAT_0067cf38 + 0x28e38) + 0x15c) = 2;
    pvVar8 = operator_new(0x180);
    local_8 = 0;
    if (pvVar8 == (void *)0x0) {
      local_100 = (int *)0x0;
    }
    else {
      local_100 = (int *)FUN_004c8f70(0x1a7,0,0x40800000,0x40800000,0x402ccccd,0,1,0x50);
    }
    local_8 = 0xffffffff;
    if (local_100 != (int *)0x0) {
      local_100[0x41] = 1;
      (**(code **)(*local_100 + 0x3c))(0x88ffffff);
      local_100[0x1b] = 1;
      uVar9 = FUN_00430f50(fVar1 - _DAT_005a415c,0xc0975c29,fVar2 - _DAT_005a40ec);
      FUN_004310a0(uVar9);
      local_100[0x57] = 3;
      (**(code **)(*local_100 + 0x40))(0xed8);
      FUN_0054ac09(local_100);
    }
    pvVar8 = operator_new(0x180);
    local_8 = 1;
    if (pvVar8 == (void *)0x0) {
      local_104 = (int *)0x0;
    }
    else {
      local_104 = (int *)FUN_004c8f70(0x1a7,0,0x40800000,0x40800000,0x402ccccd,0,1,0x50);
    }
    local_8 = 0xffffffff;
    if (local_104 != (int *)0x0) {
      local_104[0x41] = 1;
      (**(code **)(*local_104 + 0x3c))(0x88ffffff);
      local_104[0x1b] = 1;
      uVar9 = FUN_00430f50(fVar1 + _DAT_005a415c,0xc0975c29,fVar2 + _DAT_005a40ec);
      FUN_004310a0(uVar9);
      local_104[0x57] = 3;
      (**(code **)(*local_104 + 0x40))(0xed8);
      FUN_0054ac09(local_104);
    }
    pvVar8 = operator_new(0x180);
    local_8 = 2;
    if (pvVar8 == (void *)0x0) {
      local_108 = (int *)0x0;
    }
    else {
      local_108 = (int *)FUN_004c8f70(0x1a7,0,0x40800000,0x40800000,0x402ccccd,0,1,0x50);
    }
    local_8 = 0xffffffff;
    if (local_108 != (int *)0x0) {
      local_108[0x41] = 1;
      (**(code **)(*local_108 + 0x3c))(0x88ffffff);
      local_108[0x1b] = 1;
      uVar9 = FUN_00430f50(fVar1,_DAT_005a415c + -4.73,fVar2);
      FUN_004310a0(uVar9);
      local_108[0x57] = 3;
      (**(code **)(*local_108 + 0x40))(0xed8);
      FUN_0054ac09(local_108);
    }
    pvVar8 = operator_new(0x90);
    local_8 = 3;
    if (pvVar8 == (void *)0x0) {
      local_10c = 0;
    }
    else {
      uVar9 = 5;
      puVar10 = (undefined4 *)FUN_00430f50(fVar3,fVar5,fVar4);
      local_10c = FUN_004ee190(*puVar10,puVar10[1],puVar10[2],uVar9);
    }
    local_8 = 0xffffffff;
    if (local_10c != 0) {
      FUN_0054ac09(local_10c);
    }
    pvVar8 = operator_new(0xd0);
    local_8 = 4;
    if (pvVar8 == (void *)0x0) {
      local_110 = 0;
    }
    else {
      uVar13 = 0;
      uVar9 = 0x42480000;
      puVar10 = (undefined4 *)FUN_00430f50(fVar3,fVar5,fVar4);
      local_110 = FUN_004cf150(*puVar10,puVar10[1],puVar10[2],uVar9,uVar13);
    }
    local_8 = 0xffffffff;
    if (local_110 != 0) {
      FUN_0054ac09(local_110);
    }
    pvVar8 = operator_new(0xd0);
    local_8 = 5;
    if (pvVar8 == (void *)0x0) {
      local_114 = 0;
    }
    else {
      uVar13 = 0;
      uVar9 = 0x42480000;
      puVar10 = (undefined4 *)FUN_00430f50(fVar3 - _DAT_005a40ec,fVar5,fVar4 - _DAT_005a40ec);
      local_114 = FUN_004cf150(*puVar10,puVar10[1],puVar10[2],uVar9,uVar13);
    }
    local_8 = 0xffffffff;
    if (local_114 != 0) {
      FUN_0054ac09(local_114);
    }
    pvVar8 = operator_new(0xd0);
    local_8 = 6;
    if (pvVar8 == (void *)0x0) {
      local_118 = 0;
    }
    else {
      uVar13 = 0;
      uVar9 = 0x42480000;
      puVar10 = (undefined4 *)FUN_00430f50(fVar3 + _DAT_005a40ec,fVar5,fVar4 + _DAT_005a40ec);
      local_118 = FUN_004cf150(*puVar10,puVar10[1],puVar10[2],uVar9,uVar13);
    }
    local_8 = 0xffffffff;
    if (local_118 != 0) {
      FUN_0054ac09(local_118);
    }
    iVar11 = __ftol();
    if ((iVar11 >> 7 == 0x12) && (iVar11 = __ftol(), iVar11 >> 7 == 0x1e)) {
      bVar6 = true;
    }
    else {
      bVar6 = false;
    }
    if ((bVar6) && (iVar11 = *(int *)(iVar7 + 0x40), iVar11 != 0)) {
      iVar12 = (**(code **)(*DAT_0092e654 + 8))();
      *(int *)(iVar11 + 0x20a6c) = iVar12 + -100;
      uVar9 = FUN_00430f20(fVar3,fVar4);
      FUN_0040bd30(uVar9);
    }
    pvVar8 = operator_new(0x94);
    local_8 = 7;
    if (pvVar8 == (void *)0x0) {
      local_120 = 0;
    }
    else {
      uVar13 = 0x3dcccccd;
      uVar9 = 5;
      puVar10 = (undefined4 *)FUN_00430f50(fVar3,fVar5 - _DAT_005a365c,fVar4);
      local_120 = FUN_004e6590(*puVar10,puVar10[1],puVar10[2],uVar9,uVar13);
    }
    local_8 = 0xffffffff;
    if (local_120 != 0) {
      FUN_0054ac09(local_120);
    }
    uVar9 = (**(code **)(*DAT_0092e654 + 8))();
    *(undefined4 *)(iVar7 + 0x28e40) = uVar9;
    *(undefined4 *)(iVar7 + 0x28e44) = 1;
    ExceptionList = local_10;
    return 1;
  }
  return 0;
}

