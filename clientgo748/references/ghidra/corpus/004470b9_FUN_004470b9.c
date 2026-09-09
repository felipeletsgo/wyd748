// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004470b9 | Name: FUN_004470b9


void __fastcall FUN_004470b9(int param_1)

{
  int *piVar1;
  undefined4 *puVar2;
  short *psVar3;
  void *pvVar4;
  int local_70;
  int local_68;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f5da;
  local_10 = ExceptionList;
  piVar1 = *(int **)(param_1 + 0x28718);
  ExceptionList = &local_10;
  for (local_14 = 0; local_14 < 10; local_14 = local_14 + 1) {
    puVar2 = (undefined4 *)(**(code **)(*piVar1 + 0xa4))(local_14,0);
    if ((-1 < *(char *)(DAT_013b71e8 + local_14 + 0xef6)) &&
       (*(char *)(DAT_013b71e8 + local_14 + 0xef6) < 'h')) {
      psVar3 = operator_new(8);
      _memset(psVar3,0,8);
      *psVar3 = *(char *)(DAT_013b71e8 + local_14 + 0xef6) + 5000;
      pvVar4 = operator_new(0x678);
      local_8 = 0;
      if (pvVar4 == (void *)0x0) {
        local_68 = 0;
      }
      else {
        local_68 = FUN_0040d13e(0,psVar3,0,0);
      }
      local_8 = 0xffffffff;
      if (local_68 != 0) {
        (**(code **)(*piVar1 + 0x8c))(local_68,local_14,0);
      }
      if ((local_14 == *(char *)(DAT_013b71e8 + 0xef5)) &&
         (*(undefined4 *)(local_68 + 0xa0) = 2, DAT_005b892c == 2)) {
        *(undefined4 *)(local_68 + 0xa0) = 200;
      }
    }
    if ((*(int *)(DAT_005ccec0 + 0x1e8) != 0) && (*(undefined4 **)(DAT_005ccec0 + 0x1e8) == puVar2))
    {
      *(undefined4 *)(DAT_005ccec0 + 0x1e8) = 0;
    }
    if ((puVar2 != (undefined4 *)0x0) && (puVar2 != (undefined4 *)0x0)) {
      (**(code **)*puVar2)(1);
    }
  }
  piVar1 = *(int **)(param_1 + 0x2871c);
  for (local_14 = 0; local_14 < 10; local_14 = local_14 + 1) {
    puVar2 = (undefined4 *)(**(code **)(*piVar1 + 0xa4))(local_14,0);
    if ((-1 < *(char *)(DAT_013b71e8 + local_14 + 0xf00)) &&
       (*(char *)(DAT_013b71e8 + local_14 + 0xf00) < 'h')) {
      psVar3 = operator_new(8);
      _memset(psVar3,0,8);
      *psVar3 = *(char *)(DAT_013b71e8 + local_14 + 0xf00) + 5000;
      pvVar4 = operator_new(0x678);
      local_8 = 1;
      if (pvVar4 == (void *)0x0) {
        local_70 = 0;
      }
      else {
        local_70 = FUN_0040d13e(0,psVar3,0,0);
      }
      local_8 = 0xffffffff;
      if (local_70 != 0) {
        (**(code **)(*piVar1 + 0x8c))(local_70,local_14,0);
      }
      if ((local_14 + 10 == (int)*(char *)(DAT_013b71e8 + 0xef5)) &&
         (*(undefined4 *)(local_70 + 0xa0) = 2, DAT_005b892c == 2)) {
        *(undefined4 *)(local_70 + 0xa0) = 200;
      }
    }
    if ((*(int *)(DAT_005ccec0 + 0x1e8) != 0) && (*(undefined4 **)(DAT_005ccec0 + 0x1e8) == puVar2))
    {
      *(undefined4 *)(DAT_005ccec0 + 0x1e8) = 0;
    }
    if ((puVar2 != (undefined4 *)0x0) && (puVar2 != (undefined4 *)0x0)) {
      (**(code **)*puVar2)(1);
    }
  }
  ExceptionList = local_10;
  return;
}

