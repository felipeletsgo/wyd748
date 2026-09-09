// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0047fa1d | Name: FUN_0047fa1d


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0047fa1d(int param_1)

{
  void *pvVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
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
  int local_5c;
  undefined1 local_24 [4];
  undefined2 local_20;
  undefined2 local_1e;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f96d;
  local_10 = ExceptionList;
  if (*(int *)(param_1 + 0x4c) != 0) {
    ExceptionList = &local_10;
    *(float *)(*(int *)(param_1 + 0x4c) + 0x28) =
         *(float *)(*(int *)(param_1 + 0x4c) + 0x28) + *(float *)(*(int *)(param_1 + 0x4c) + 0x88c);
    *(float *)(*(int *)(param_1 + 0x4c) + 0x2c) =
         *(float *)(*(int *)(param_1 + 0x4c) + 0x2c) + *(float *)(*(int *)(param_1 + 0x4c) + 0x890);
    *(undefined4 *)(param_1 + 0x26e24) = 0xffffffff;
    *(undefined4 *)(param_1 + 0x26e1c) = 0;
    if (*(int *)(param_1 + 0x26e20) < 1) {
      if (*(int *)(*(int *)(param_1 + 0x4c) + 0x19c) != 0) {
        if (*(int *)(*(int *)(param_1 + 0x4c) + 0x19c) != 0) {
          FUN_00493d20(1);
        }
        *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x19c) = 0;
      }
      *(undefined1 *)(*(int *)(param_1 + 0x4c) + 0x79c) = 0;
      *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x7a0) = 0;
    }
    else {
      *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x5c) = *(undefined4 *)(param_1 + 0x26e20);
      *(undefined4 *)(param_1 + 0x26e20) = 0xffffffff;
      FUN_00527848();
    }
    FUN_00480a83();
    *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x888) = 0;
    *(undefined4 *)(param_1 + 0x26e28) = 0;
    FUN_0051dba4(*(undefined4 *)(param_1 + 0x26e48),1);
    pvVar1 = operator_new(0x90);
    local_8 = 0;
    if (pvVar1 == (void *)0x0) {
      local_5c = 0;
    }
    else {
      uVar14 = 1000;
      puVar2 = (undefined4 *)FUN_00430f50(0,0,0);
      uVar11 = *puVar2;
      uVar12 = puVar2[1];
      uVar13 = puVar2[2];
      uVar10 = 1;
      uVar9 = 0x3f800000;
      uVar8 = 0x38;
      uVar7 = 1;
      uVar6 = 0;
      uVar5 = 0x40400000;
      uVar4 = 10;
      uVar3 = 1;
      puVar2 = (undefined4 *)
               FUN_00430f50(*(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x28),
                            *(float *)(*(int *)(param_1 + 0x4c) + 0x30) + _DAT_005a3660,
                            *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x2c));
      local_5c = FUN_004d4de0(*puVar2,puVar2[1],puVar2[2],uVar3,uVar4,uVar5,uVar6,uVar7,uVar8,uVar9,
                              uVar10,uVar11,uVar12,uVar13,uVar14);
    }
    local_8 = 0xffffffff;
    if (local_5c != 0) {
      FUN_0054ac09(local_5c);
    }
    if ((*(int *)(param_1 + 0x26e40) < 0) || (10 < *(int *)(param_1 + 0x26e40))) {
      *(undefined4 *)(param_1 + 0x26e40) = 0;
    }
    _memset(local_24,0,0x14);
    local_20 = 0xad9;
    local_1e = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
    local_18 = *(undefined4 *)(param_1 + 0x26e40);
    local_14 = 2;
    FUN_0055f2dd(local_24,0x14);
  }
  ExceptionList = local_10;
  return;
}

