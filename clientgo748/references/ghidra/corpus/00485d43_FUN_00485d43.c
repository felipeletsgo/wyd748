// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00485d43 | Name: FUN_00485d43


undefined4 __thiscall FUN_00485d43(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  int local_50 [4];
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  int local_10;
  undefined4 *local_c;
  int local_8;
  
  local_c = (undefined4 *)0x0;
  local_8 = param_2;
  if (*(int *)(param_2 + 0xc) == 0) {
    local_50[0] = 0;
    local_50[1] = *(undefined4 *)(param_1 + 0x28740);
    local_50[2] = *(undefined4 *)(param_1 + 0x28744);
    local_50[3] = *(undefined4 *)(param_1 + 0x28748);
    local_40 = *(undefined4 *)(param_1 + 0x2874c);
    local_3c = *(undefined4 *)(param_1 + 0x28750);
    local_38 = *(undefined4 *)(param_1 + 0x28758);
    local_34 = *(undefined4 *)(param_1 + 0x28754);
    local_30 = *(undefined4 *)(param_1 + 0x28764);
    local_2c = *(undefined4 *)(param_1 + 0x28768);
    local_28 = *(undefined4 *)(param_1 + 0x2876c);
    local_24 = *(undefined4 *)(param_1 + 0x28770);
    local_20 = *(undefined4 *)(param_1 + 0x2875c);
    local_1c = *(undefined4 *)(param_1 + 0x28760);
    local_18 = *(undefined4 *)(param_1 + 0x28774);
    local_14 = *(undefined4 *)(param_1 + 0x28778);
    if (local_50[*(int *)(param_2 + 0x10)] != 0) {
      local_c = (undefined4 *)(**(code **)(*(int *)local_50[*(int *)(param_2 + 0x10)] + 0xa4))(0,0);
    }
    _memset((void *)(DAT_013b71e8 + 0x748 + *(int *)(local_8 + 0x10) * 8),0,8);
  }
  else if (*(int *)(param_2 + 0xc) == 1) {
    local_c = (undefined4 *)
              (**(code **)(**(int **)(param_1 + 0x2873c) + 0xa8))
                        (*(int *)(param_2 + 0x10) % 9,*(int *)(param_2 + 0x10) / 9);
    _memset((void *)(DAT_013b71e8 + 0x7c8 + *(int *)(local_8 + 0x10) * 8),0,8);
  }
  else if (*(int *)(param_2 + 0xc) == 2) {
    piVar1 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x726);
    local_c = (undefined4 *)
              (**(code **)(*piVar1 + 0xa8))
                        (*(int *)(local_8 + 0x10) % 9,*(int *)(local_8 + 0x10) / 9);
    _memset((void *)(DAT_013b71e8 + 0x2ec + *(int *)(local_8 + 0x10) * 8),0,8);
  }
  (**(code **)(*DAT_005ccec0 + 0x98))();
  if (local_c != (undefined4 *)0x0) {
    if (local_c != (undefined4 *)0x0) {
      (**(code **)*local_c)(1);
    }
    local_c = (undefined4 *)0x0;
  }
  local_10 = DAT_013b71e8 + 0x6ec;
  *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x7ae) = *(undefined2 *)(DAT_013b71e8 + 0x7b0);
  if ((DAT_005ccf98 != 0) && (iVar2 = FUN_00429a6d(0x2d), iVar2 != 0)) {
    uVar4 = 0;
    uVar3 = 0;
    FUN_00429a6d(0x2d);
    FUN_0042ad2b(uVar3,uVar4);
  }
  FUN_004431e4(0);
  FUN_00480a83();
  return 1;
}

