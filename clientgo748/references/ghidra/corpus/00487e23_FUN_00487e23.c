// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00487e23 | Name: FUN_00487e23


undefined4 __thiscall FUN_00487e23(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  int local_68;
  int local_5c;
  undefined4 local_54 [4];
  undefined4 local_44;
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
  undefined4 *local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_10 = param_2;
  local_c = *(int *)(param_1 + 0x28730);
  local_8 = DAT_013b71e8 + 0x6ec;
  if (((*(uint *)(*(int *)(param_1 + 0x2877c) + 0x404) == (uint)*(ushort *)(param_2 + 0xc)) ||
      (*(uint *)(*(int *)(param_1 + 0x28730) + 0x404) == (uint)*(ushort *)(param_2 + 0xc))) ||
     ((*(short *)(param_2 + 0xc) == 0 &&
      ((*(char *)(DAT_013b71e8 + 0x700) == '\x03' &&
       ((*(uint *)(DAT_013b71e8 + 0x9c8) & 0x1000) != 0)))))) {
    local_14 = (undefined4 *)0x0;
    if (*(short *)(param_2 + 0xe) == 0) {
      local_54[0] = *(undefined4 *)(param_1 + 0x2873c);
      local_54[1] = *(undefined4 *)(param_1 + 0x28740);
      local_54[2] = *(undefined4 *)(param_1 + 0x28744);
      local_54[3] = *(undefined4 *)(param_1 + 0x28748);
      local_44 = *(undefined4 *)(param_1 + 0x2874c);
      local_40 = *(undefined4 *)(param_1 + 0x28750);
      local_3c = *(undefined4 *)(param_1 + 0x28758);
      local_38 = *(undefined4 *)(param_1 + 0x28754);
      local_34 = *(undefined4 *)(param_1 + 0x28764);
      local_30 = *(undefined4 *)(param_1 + 0x28768);
      local_2c = *(undefined4 *)(param_1 + 0x2876c);
      local_28 = *(undefined4 *)(param_1 + 0x28770);
      local_24 = *(undefined4 *)(param_1 + 0x2875c);
      local_20 = *(undefined4 *)(param_1 + 0x28760);
      local_1c = *(undefined4 *)(param_1 + 0x28774);
      local_18 = *(undefined4 *)(param_1 + 0x28778);
      local_14 = (undefined4 *)
                 (**(code **)(*(int *)local_54[*(short *)(param_2 + 0x10)] + 0xa4))(0,0);
      local_5c = __ftol();
      iVar1 = DAT_013b71e8;
      if ((local_5c < 0x1389) || (10000 < local_5c)) {
        if (10000 < local_5c) {
          local_5c = local_5c / 2;
        }
      }
      else {
        local_5c = (local_5c << 1) / 3;
      }
      _memset((void *)(DAT_013b71e8 + 0x748 + *(short *)(local_10 + 0x10) * 8),0,8);
      *(int *)(iVar1 + 0x704) = *(int *)(iVar1 + 0x704) + local_5c;
    }
    else if (*(short *)(param_2 + 0xe) == 1) {
      local_14 = (undefined4 *)
                 (**(code **)(**(int **)(param_1 + 0x2873c) + 0xa8))
                           ((int)*(short *)(param_2 + 0x10) % 9,(int)*(short *)(param_2 + 0x10) / 9)
      ;
      local_68 = __ftol();
      iVar1 = DAT_013b71e8;
      if ((local_68 < 0x1389) || (10000 < local_68)) {
        if (10000 < local_68) {
          local_68 = local_68 / 2;
        }
      }
      else {
        local_68 = (local_68 << 1) / 3;
      }
      _memset((void *)(DAT_013b71e8 + 0x7c8 + *(short *)(local_10 + 0x10) * 8),0,8);
      *(int *)(iVar1 + 0x704) = *(int *)(iVar1 + 0x704) + local_68;
    }
    if ((DAT_005ccec0[0x7a] != 0) && ((undefined4 *)DAT_005ccec0[0x7a] == local_14)) {
      DAT_005ccec0[0x7a] = 0;
    }
    if (local_14 != (undefined4 *)0x0) {
      if (local_14 != (undefined4 *)0x0) {
        (**(code **)*local_14)(1);
      }
      local_14 = (undefined4 *)0x0;
    }
    FUN_004431e4(0);
    if ((DAT_005ccf98 != 0) && (iVar1 = FUN_00429a6d(0x1f), iVar1 != 0)) {
      uVar3 = 0;
      uVar2 = 0;
      FUN_00429a6d(0x1f);
      FUN_0042ad2b(uVar2,uVar3);
    }
    (**(code **)(*DAT_005ccec0 + 0x98))();
  }
  FUN_00480a83();
  return 1;
}

