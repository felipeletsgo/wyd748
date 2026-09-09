// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00486009 | Name: FUN_00486009


undefined4 __thiscall FUN_00486009(int param_1,int param_2)

{
  byte bVar1;
  void *pvVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  int local_80;
  undefined1 local_68 [64];
  int local_28;
  byte local_24;
  undefined3 uStack_23;
  int local_20;
  int *local_1c;
  int local_18;
  void *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059faf0;
  local_10 = ExceptionList;
  local_1c = *(int **)(param_1 + 0x2873c);
  local_20 = param_2;
  ExceptionList = &local_10;
  local_14 = operator_new(8);
  if (local_14 != (void *)0x0) {
    FUN_0058f220(local_14,local_20 + 0x14,8);
    local_18 = FUN_0054cd07(local_14,0x26);
    if (local_18 == 2) {
      bVar1 = FUN_0054cd07(local_14,0x24);
      local_28 = (uint)bVar1 << 8;
      local_24 = bVar1;
      bVar1 = FUN_0054cd07(local_14,0x25);
      _local_24 = CONCAT31(uStack_23,bVar1);
      local_28 = local_28 + (uint)bVar1;
      *(int *)(DAT_013b71e8 + 0x704) = *(int *)(DAT_013b71e8 + 0x704) + local_28;
      FUN_0058f078(local_68,&DAT_005b888c,*(undefined4 *)(DAT_013b71e8 + 0x704));
      *(undefined1 *)(*(int *)(param_1 + 0x27920) + 0xcdd) = 1;
      (**(code **)(**(int **)(param_1 + 0x27920) + 0x80))(local_68,0);
      *(undefined1 *)(*(int *)(param_1 + 0x27924) + 0xcdd) = 1;
      (**(code **)(**(int **)(param_1 + 0x27924) + 0x80))(local_68,0);
      FUN_0058f078(local_68,&DAT_005b8894,
                   *(int *)(DAT_013b71e8 + 0x704) - *(int *)(param_1 + 0x276b0));
      *(undefined1 *)(*(int *)(param_1 + 0x27928) + 0xcdd) = 1;
      (**(code **)(**(int **)(param_1 + 0x27928) + 0x80))(local_68,0);
    }
    else {
      pvVar2 = operator_new(0x678);
      local_8 = 0;
      if (pvVar2 == (void *)0x0) {
        local_80 = 0;
      }
      else {
        local_80 = FUN_0040d13e(0,local_14,0,0);
      }
      local_8 = 0xffffffff;
      if (local_80 != 0) {
        (**(code **)(*local_1c + 0x8c))
                  (local_80,*(int *)(local_20 + 0x10) % 9,*(int *)(local_20 + 0x10) / 9);
      }
      FUN_0058f220(DAT_013b71e8 + 0x7c8 + *(int *)(local_20 + 0x10) * 8,local_14,8);
    }
    if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x2d), iVar3 != 0)) {
      uVar5 = 0;
      uVar4 = 0;
      FUN_00429a6d(0x2d);
      FUN_0042ad2b(uVar4,uVar5);
    }
    FUN_004431e4(0);
  }
  ExceptionList = local_10;
  return 1;
}

