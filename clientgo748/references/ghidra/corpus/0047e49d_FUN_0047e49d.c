// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0047e49d | Name: FUN_0047e49d


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0047e49d(int param_1,uint param_2)

{
  float fVar1;
  float fVar2;
  int iVar3;
  int iVar4;
  void *pvVar5;
  undefined4 *puVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  int local_b4;
  int local_b0;
  int local_ac;
  int local_a8;
  int local_a4;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f943;
  local_10 = ExceptionList;
  if ((0 < *(int *)(param_1 + 0x28e44)) && (*(int *)(param_1 + 0x28e40) + 300U < param_2)) {
    ExceptionList = &local_10;
    iVar3 = _rand();
    iVar4 = _rand();
    fVar1 = (float)(iVar3 % 9) + _DAT_005a43e4;
    fVar2 = (float)(iVar4 % 9) + _DAT_005a43e0;
    pvVar5 = operator_new(0x90);
    local_8 = 0;
    if (pvVar5 == (void *)0x0) {
      local_a4 = 0;
    }
    else {
      uVar8 = 3;
      puVar6 = (undefined4 *)FUN_00430f50(fVar1,0xc0d9999a,fVar2);
      local_a4 = FUN_004ee190(*puVar6,puVar6[1],puVar6[2],uVar8);
    }
    local_8 = 0xffffffff;
    if (local_a4 != 0) {
      FUN_0054ac09(local_a4);
    }
    pvVar5 = operator_new(0xd0);
    local_8 = 1;
    if (pvVar5 == (void *)0x0) {
      local_a8 = 0;
    }
    else {
      uVar9 = 0;
      uVar8 = 0x42480000;
      puVar6 = (undefined4 *)FUN_00430f50(fVar1,0xc0d9999a,fVar2);
      local_a8 = FUN_004cf150(*puVar6,puVar6[1],puVar6[2],uVar8,uVar9);
    }
    local_8 = 0xffffffff;
    if (local_a8 != 0) {
      FUN_0054ac09(local_a8);
    }
    pvVar5 = operator_new(0xd0);
    local_8 = 2;
    if (pvVar5 == (void *)0x0) {
      local_ac = 0;
    }
    else {
      uVar9 = 0;
      uVar8 = 0x42480000;
      puVar6 = (undefined4 *)FUN_00430f50(fVar1 - _DAT_005a40ec,0xc0d9999a,fVar2 - _DAT_005a40ec);
      local_ac = FUN_004cf150(*puVar6,puVar6[1],puVar6[2],uVar8,uVar9);
    }
    local_8 = 0xffffffff;
    if (local_ac != 0) {
      FUN_0054ac09(local_ac);
    }
    pvVar5 = operator_new(0xd0);
    local_8 = 3;
    if (pvVar5 == (void *)0x0) {
      local_b0 = 0;
    }
    else {
      uVar9 = 0;
      uVar8 = 0x42480000;
      puVar6 = (undefined4 *)FUN_00430f50(fVar1 + _DAT_005a40ec,0xc0d9999a,fVar2 + _DAT_005a40ec);
      local_b0 = FUN_004cf150(*puVar6,puVar6[1],puVar6[2],uVar8,uVar9);
    }
    local_8 = 0xffffffff;
    if (local_b0 != 0) {
      FUN_0054ac09(local_b0);
    }
    iVar3 = _rand();
    *(int *)(param_1 + 0x28e40) = *(int *)(param_1 + 0x28e40) + 300 + (iVar3 % 10 + 1) * 100;
    *(int *)(param_1 + 0x28e44) = *(int *)(param_1 + 0x28e44) + 1;
    if (9 < *(int *)(param_1 + 0x28e44)) {
      pvVar5 = operator_new(0x90);
      local_8 = 4;
      if (pvVar5 == (void *)0x0) {
        local_b4 = 0;
      }
      else {
        uVar10 = 0;
        uVar7 = 1;
        uVar9 = 0x19;
        uVar8 = 0xffcc6666;
        puVar6 = (undefined4 *)FUN_00430f50(0x4513d000,0xc11ccccd,0x4575a000);
        local_b4 = FUN_004ec9a0(*puVar6,puVar6[1],puVar6[2],uVar8,uVar9,uVar7,uVar10);
      }
      local_8 = 0xffffffff;
      if (local_b4 != 0) {
        FUN_0054ac09(local_b4);
      }
      *(undefined4 *)(param_1 + 0x28e44) = 0;
      *(undefined4 *)(param_1 + 0x28e40) = 0;
    }
  }
  ExceptionList = local_10;
  return;
}

