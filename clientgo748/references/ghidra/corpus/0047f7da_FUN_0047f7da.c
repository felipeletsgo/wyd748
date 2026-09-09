// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0047f7da | Name: FUN_0047f7da


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0047f7da(int param_1,int param_2)

{
  undefined4 uVar1;
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
  int local_50;
  undefined1 local_24 [4];
  undefined2 local_20;
  undefined2 local_1e;
  int local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f958;
  local_10 = ExceptionList;
  if (*(int *)(param_1 + 0x4c) != 0) {
    ExceptionList = &local_10;
    *(undefined4 *)(param_1 + 0x26e24) = 2;
    *(undefined4 *)(param_1 + 0x26e1c) = 1;
    *(undefined4 *)(param_1 + 0x26e48) = *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x740);
    *(undefined4 *)(param_1 + 0x26e20) = *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x7a0);
    FUN_00527848();
    *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x888) = 1;
    uVar1 = (**(code **)(*DAT_0092e654 + 8))();
    *(undefined4 *)(param_1 + 0x26e28) = uVar1;
    *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x88c) = 0;
    *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x890) = 0;
    pvVar2 = operator_new(0x90);
    local_8 = 0;
    if (pvVar2 == (void *)0x0) {
      local_50 = 0;
    }
    else {
      uVar14 = 1000;
      puVar3 = (undefined4 *)FUN_00430f50(0,0,0);
      uVar1 = *puVar3;
      uVar12 = puVar3[1];
      uVar13 = puVar3[2];
      uVar11 = 1;
      uVar10 = 0x3f800000;
      uVar9 = 0x38;
      uVar8 = 1;
      uVar7 = 0;
      uVar6 = 0x40400000;
      uVar5 = 10;
      uVar4 = 1;
      puVar3 = (undefined4 *)
               FUN_00430f50(*(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x28),
                            *(float *)(*(int *)(param_1 + 0x4c) + 0x30) + _DAT_005a3660,
                            *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x2c));
      local_50 = FUN_004d4de0(*puVar3,puVar3[1],puVar3[2],uVar4,uVar5,uVar6,uVar7,uVar8,uVar9,uVar10
                              ,uVar11,uVar1,uVar12,uVar13,uVar14);
    }
    local_8 = 0xffffffff;
    if (local_50 != 0) {
      FUN_0054ac09(local_50);
    }
    *(float *)(param_1 + 0x26e30) = (float)*(int *)(&DAT_005b4fc0 + param_2 * 0x50);
    *(float *)(param_1 + 0x26e34) = (float)*(int *)(&DAT_005b4fc4 + param_2 * 0x50);
    _memset(local_24,0,0x14);
    local_20 = 0xad9;
    local_1e = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
    local_18 = param_2;
    local_14 = 1;
    FUN_0055f2dd(local_24,0x14);
    *(int *)(param_1 + 0x26e40) = param_2;
    *(undefined4 *)(param_1 + 0x26e44) = 0;
    *(undefined4 *)(param_1 + 0x26e38) = 0x3e4ccccd;
  }
  ExceptionList = local_10;
  return;
}

