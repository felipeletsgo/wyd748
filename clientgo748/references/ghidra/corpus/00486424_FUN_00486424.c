// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00486424 | Name: FUN_00486424


undefined4 __thiscall FUN_00486424(int param_1,int param_2)

{
  void *pvVar1;
  void *pvVar2;
  int local_dc;
  undefined1 local_b4 [128];
  int *local_34;
  int *local_30;
  undefined4 *local_2c;
  int *local_28;
  int local_24;
  int *local_20;
  int local_1c;
  int *local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059fb08;
  local_10 = ExceptionList;
  local_14 = param_2;
  ExceptionList = &local_10;
  local_28 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x299);
  local_18 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x29a);
  *(undefined1 *)(local_14 + 0x23) = 0;
  *(undefined1 *)(local_14 + 0x22) = 0;
  if (local_28 != (int *)0x0) {
    (**(code **)(*local_28 + 0x80))(local_14 + 0xc,0);
  }
  local_1c = (**(code **)(*DAT_013b71e8 + 0x34))(*(undefined2 *)(local_14 + 0xc2));
  FUN_0058f220(param_1 + 0x274ec,local_14,0xc4);
  if (local_1c != 0) {
    (**(code **)(*local_18 + 0x80))(local_1c + 0x168,1);
  }
  for (local_24 = 0; local_24 < 0xc; local_24 = local_24 + 1) {
    FUN_0058f078(local_b4,&DAT_005b889c,*(undefined4 *)(local_14 + 0x90 + local_24 * 4));
    local_34 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(local_24 + 800);
    *(undefined1 *)((int)local_34 + 0xcdd) = 1;
    (**(code **)(*local_34 + 0x80))(local_b4,0);
    local_30 = *(int **)(param_1 + 0x27b3c + local_24 * 4);
    local_30[0x163] = *(int *)(local_14 + 0x90 + local_24 * 4);
    local_2c = (undefined4 *)(**(code **)(*local_30 + 0xa8))(0,0);
    if ((*(int *)(DAT_005ccec0 + 0x1e8) != 0) &&
       (*(undefined4 **)(DAT_005ccec0 + 0x1e8) == local_2c)) {
      *(undefined4 *)(DAT_005ccec0 + 0x1e8) = 0;
    }
    if (local_2c != (undefined4 *)0x0) {
      if (local_2c != (undefined4 *)0x0) {
        (**(code **)*local_2c)(1);
      }
      local_2c = (undefined4 *)0x0;
    }
    if ((0 < *(short *)(local_14 + 0x24 + local_24 * 8)) &&
       (pvVar1 = operator_new(8), pvVar1 != (void *)0x0)) {
      FUN_0058f220(pvVar1,local_14 + 0x24 + local_24 * 8,8);
      pvVar2 = operator_new(0x678);
      local_8 = 0;
      if (pvVar2 == (void *)0x0) {
        local_dc = 0;
      }
      else {
        local_dc = FUN_0040d13e(local_30,pvVar1,0,0);
      }
      local_8 = 0xffffffff;
      if (local_dc != 0) {
        (**(code **)(*local_30 + 0x8c))(local_dc,0,0);
      }
    }
  }
  local_20 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x29b);
  if (local_20 != (int *)0x0) {
    (**(code **)(*local_20 + 0x60))(0);
  }
  if ((uint)*(ushort *)(local_14 + 0xc2) == *(uint *)(*(int *)(param_1 + 0x4c) + 0x20)) {
    FUN_0058f078(*(int *)(param_1 + 0x4c) + 0x724,local_14 + 0xc);
    (**(code **)(**(int **)(*(int *)(param_1 + 0x4c) + 0x424) + 0x80))
              (*(int *)(param_1 + 0x4c) + 0x724,0);
  }
  else {
    FUN_0044ae38(1,0);
  }
  ExceptionList = local_10;
  return 1;
}

