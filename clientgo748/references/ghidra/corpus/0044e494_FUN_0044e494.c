// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044e494 | Name: FUN_0044e494


void __thiscall FUN_0044e494(int param_1,int param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  undefined4 *puVar3;
  void *pvVar4;
  void *pvVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  int local_90;
  int local_88;
  int local_34;
  undefined1 local_30 [4];
  undefined2 local_2c;
  undefined2 local_2a;
  char local_24 [20];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f5fa;
  local_10 = ExceptionList;
  if ((param_3 != 0) &&
     (ExceptionList = &local_10, iVar2 = FUN_0040db40(**(short **)(param_3 + 0x670) + -5000),
     iVar2 != 1)) {
    if (param_2 < 10) {
      piVar1 = *(int **)(param_1 + 0x28718);
      puVar3 = (undefined4 *)(**(code **)(*piVar1 + 0xa4))(param_2,0);
      pvVar4 = operator_new(8);
      FUN_0058f220(pvVar4,*(undefined4 *)(param_3 + 0x670),8);
      pvVar5 = operator_new(0x678);
      local_8 = 0;
      if (pvVar5 == (void *)0x0) {
        local_88 = 0;
      }
      else {
        local_88 = FUN_0040d13e(0,pvVar4,0,0);
      }
      local_8 = 0xffffffff;
      (**(code **)(*piVar1 + 0x8c))(local_88,param_2,0);
      if ((*(char *)(DAT_013b71e8 + 0xef5) == param_2) &&
         (*(undefined4 *)(local_88 + 0xa0) = 2, DAT_005b892c == 2)) {
        *(undefined4 *)(local_88 + 0xa0) = 200;
      }
      (**(code **)(*DAT_005ccec0 + 0x98))();
      if ((puVar3 != (undefined4 *)0x0) && (puVar3 != (undefined4 *)0x0)) {
        (**(code **)*puVar3)(1);
      }
    }
    else {
      piVar1 = *(int **)(param_1 + 0x2871c);
      puVar3 = (undefined4 *)(**(code **)(*piVar1 + 0xa4))(param_2 + -10,0);
      pvVar4 = operator_new(8);
      FUN_0058f220(pvVar4,*(undefined4 *)(param_3 + 0x670),8);
      pvVar5 = operator_new(0x678);
      local_8 = 1;
      if (pvVar5 == (void *)0x0) {
        local_90 = 0;
      }
      else {
        local_90 = FUN_0040d13e(0,pvVar4,0,0);
      }
      local_8 = 0xffffffff;
      (**(code **)(*piVar1 + 0x8c))(local_90,param_2 + -10,0);
      if ((*(char *)(DAT_013b71e8 + 0xef5) == param_2) &&
         (*(undefined4 *)(local_90 + 0xa0) = 2, DAT_005b892c == 2)) {
        *(undefined4 *)(local_90 + 0xa0) = 200;
      }
      (**(code **)(*DAT_005ccec0 + 0x98))();
      if ((puVar3 != (undefined4 *)0x0) && (puVar3 != (undefined4 *)0x0)) {
        (**(code **)*puVar3)(1);
      }
    }
    *(undefined *)(DAT_013b71e8 + param_2 + 0xef6) =
         (&DAT_00d44992)[**(short **)(param_3 + 0x670) * 0x8c];
    _memset(local_30,0,0x20);
    local_2a = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
    local_2c = 0x378;
    FUN_0058f220(local_24,DAT_013b71e8 + 0xef6,0x14);
    for (local_34 = 0; local_34 < 0x14; local_34 = local_34 + 1) {
      if ((-1 < local_24[local_34]) && (local_24[local_34] < '`')) {
        local_24[local_34] = local_24[local_34] + *(char *)(DAT_013b71e8 + 0x700) * -0x18;
      }
    }
    FUN_0055f2dd(local_30,0x20);
    if ((DAT_005ccf98 != 0) && (iVar2 = FUN_00429a6d(0x1f), iVar2 != 0)) {
      uVar7 = 0;
      uVar6 = 0;
      FUN_00429a6d(0x1f);
      FUN_0042ad2b(uVar6,uVar7);
    }
  }
  ExceptionList = local_10;
  return;
}

