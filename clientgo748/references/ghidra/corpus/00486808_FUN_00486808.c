// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00486808 | Name: FUN_00486808


undefined4 __thiscall FUN_00486808(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 local_94 [4];
  undefined4 local_84;
  undefined4 local_80;
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  int *local_54;
  undefined4 *local_50;
  undefined4 local_4c [4];
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
  undefined4 local_10;
  int *local_c;
  undefined4 *local_8;
  
  local_c = (int *)0x0;
  local_54 = (int *)0x0;
  local_50 = (undefined4 *)0x0;
  local_8 = (undefined4 *)0x0;
  if (*(char *)(param_2 + 0xc) == '\0') {
    local_94[0] = *(undefined4 *)(param_1 + 0x2873c);
    local_94[1] = *(undefined4 *)(param_1 + 0x28740);
    local_94[2] = *(undefined4 *)(param_1 + 0x28744);
    local_94[3] = *(undefined4 *)(param_1 + 0x28748);
    local_84 = *(undefined4 *)(param_1 + 0x2874c);
    local_80 = *(undefined4 *)(param_1 + 0x28750);
    local_7c = *(undefined4 *)(param_1 + 0x28758);
    local_78 = *(undefined4 *)(param_1 + 0x28754);
    local_74 = *(undefined4 *)(param_1 + 0x28764);
    local_70 = *(undefined4 *)(param_1 + 0x28768);
    local_6c = *(undefined4 *)(param_1 + 0x2876c);
    local_68 = *(undefined4 *)(param_1 + 0x28770);
    local_64 = *(undefined4 *)(param_1 + 0x2875c);
    local_60 = *(undefined4 *)(param_1 + 0x28760);
    local_5c = *(undefined4 *)(param_1 + 0x28774);
    local_58 = *(undefined4 *)(param_1 + 0x28778);
    local_c = (int *)local_94[*(byte *)(param_2 + 0xd)];
    local_50 = (undefined4 *)(**(code **)(*local_c + 0xa4))(0,0);
    _memset(DAT_013b71e8 + (uint)*(byte *)(param_2 + 0xd) * 2 + 0x1d2,0,8);
  }
  else if (*(char *)(param_2 + 0xc) == '\x01') {
    local_c = *(int **)(param_1 + 0x2873c);
    local_50 = (undefined4 *)
               (**(code **)(*local_c + 0xa8))
                         ((uint)*(byte *)(param_2 + 0xd) % 9,*(byte *)(param_2 + 0xd) / 9);
    _memset(DAT_013b71e8 + (uint)*(byte *)(param_2 + 0xd) * 2 + 0x1f2,0,8);
  }
  else if (*(char *)(param_2 + 0xc) == '\x02') {
    local_c = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x726);
    local_50 = (undefined4 *)
               (**(code **)(*local_c + 0xa8))
                         ((uint)*(byte *)(param_2 + 0xd) % 9,*(byte *)(param_2 + 0xd) / 9);
    _memset(DAT_013b71e8 + (uint)*(byte *)(param_2 + 0xd) * 2 + 0xbb,0,8);
  }
  if (*(char *)(param_2 + 0xe) == '\0') {
    local_4c[0] = *(undefined4 *)(param_1 + 0x2873c);
    local_4c[1] = *(undefined4 *)(param_1 + 0x28740);
    local_4c[2] = *(undefined4 *)(param_1 + 0x28744);
    local_4c[3] = *(undefined4 *)(param_1 + 0x28748);
    local_3c = *(undefined4 *)(param_1 + 0x2874c);
    local_38 = *(undefined4 *)(param_1 + 0x28750);
    local_34 = *(undefined4 *)(param_1 + 0x28758);
    local_30 = *(undefined4 *)(param_1 + 0x28754);
    local_2c = *(undefined4 *)(param_1 + 0x28764);
    local_28 = *(undefined4 *)(param_1 + 0x28768);
    local_24 = *(undefined4 *)(param_1 + 0x2876c);
    local_20 = *(undefined4 *)(param_1 + 0x28770);
    local_1c = *(undefined4 *)(param_1 + 0x2875c);
    local_18 = *(undefined4 *)(param_1 + 0x28760);
    local_14 = *(undefined4 *)(param_1 + 0x28774);
    local_10 = *(undefined4 *)(param_1 + 0x28778);
    local_54 = (int *)local_4c[*(byte *)(param_2 + 0xf)];
    local_8 = (undefined4 *)(**(code **)(*local_54 + 0xa4))(0,0);
    _memset(DAT_013b71e8 + (uint)*(byte *)(param_2 + 0xf) * 2 + 0x1d2,0,8);
  }
  else if (*(char *)(param_2 + 0xe) == '\x01') {
    local_54 = *(int **)(param_1 + 0x2873c);
    local_8 = (undefined4 *)
              (**(code **)(*local_54 + 0xa8))
                        ((uint)*(byte *)(param_2 + 0xf) % 9,*(byte *)(param_2 + 0xf) / 9);
    _memset(DAT_013b71e8 + (uint)*(byte *)(param_2 + 0xf) * 2 + 0x1f2,0,8);
  }
  else if (*(char *)(param_2 + 0xe) == '\x02') {
    local_54 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x726);
    local_8 = (undefined4 *)
              (**(code **)(*local_54 + 0xa8))
                        ((uint)*(byte *)(param_2 + 0xf) % 9,*(byte *)(param_2 + 0xf) / 9);
    _memset(DAT_013b71e8 + (uint)*(byte *)(param_2 + 0xf) * 2 + 0xbb,0,8);
  }
  if (local_8 != (undefined4 *)0x0) {
    if (*(char *)(param_2 + 0xc) == '\0') {
      if (*(short *)local_8[0x19c] < 0x29) {
        if ((DAT_005ccec0[0x7a] != 0) && ((undefined4 *)DAT_005ccec0[0x7a] == local_8)) {
          DAT_005ccec0[0x7a] = 0;
        }
        if (local_8 != (undefined4 *)0x0) {
          if (local_8 != (undefined4 *)0x0) {
            (**(code **)*local_8)(1);
          }
          local_8 = (undefined4 *)0x0;
        }
      }
      else {
        (**(code **)(*local_c + 0x8c))(local_8,0,0);
        if (local_8 == (undefined4 *)0x0) {
          _memset(DAT_013b71e8 + (uint)*(byte *)(param_2 + 0xd) * 2 + 0x1d2,0,8);
        }
        else {
          FUN_0058f220(DAT_013b71e8 + (uint)*(byte *)(param_2 + 0xd) * 2 + 0x1d2,local_8[0x19c],8);
        }
      }
    }
    else if (*(char *)(param_2 + 0xc) == '\x01') {
      if (*(short *)local_8[0x19c] < 0x29) {
        if ((DAT_005ccec0[0x7a] != 0) && ((undefined4 *)DAT_005ccec0[0x7a] == local_8)) {
          DAT_005ccec0[0x7a] = 0;
        }
        if (local_8 != (undefined4 *)0x0) {
          if (local_8 != (undefined4 *)0x0) {
            (**(code **)*local_8)(1);
          }
          local_8 = (undefined4 *)0x0;
        }
      }
      else {
        (**(code **)(*local_c + 0x8c))
                  (local_8,(uint)*(byte *)(param_2 + 0xd) % 9,*(byte *)(param_2 + 0xd) / 9);
        if (local_8 == (undefined4 *)0x0) {
          _memset(DAT_013b71e8 + (uint)*(byte *)(param_2 + 0xd) * 2 + 0x1f2,0,8);
        }
        else {
          FUN_0058f220(DAT_013b71e8 + (uint)*(byte *)(param_2 + 0xd) * 2 + 0x1f2,local_8[0x19c],8);
        }
      }
    }
    else if (*(char *)(param_2 + 0xc) == '\x02') {
      if (*(short *)local_8[0x19c] < 0x29) {
        if ((DAT_005ccec0[0x7a] != 0) && ((undefined4 *)DAT_005ccec0[0x7a] == local_8)) {
          DAT_005ccec0[0x7a] = 0;
        }
        if (local_8 != (undefined4 *)0x0) {
          if (local_8 != (undefined4 *)0x0) {
            (**(code **)*local_8)(1);
          }
          local_8 = (undefined4 *)0x0;
        }
      }
      else {
        (**(code **)(*local_c + 0x8c))
                  (local_8,(uint)*(byte *)(param_2 + 0xd) % 9,*(byte *)(param_2 + 0xd) / 9);
        if (local_8 == (undefined4 *)0x0) {
          _memset(DAT_013b71e8 + (uint)*(byte *)(param_2 + 0xd) * 2 + 0xbb,0,8);
        }
        else {
          FUN_0058f220(DAT_013b71e8 + (uint)*(byte *)(param_2 + 0xd) * 2 + 0xbb,local_8[0x19c],8);
        }
      }
    }
  }
  if (local_50 != (undefined4 *)0x0) {
    if (*(char *)(param_2 + 0xe) == '\0') {
      if (*(short *)local_50[0x19c] < 0x29) {
        if ((DAT_005ccec0[0x7a] != 0) && ((undefined4 *)DAT_005ccec0[0x7a] == local_50)) {
          DAT_005ccec0[0x7a] = 0;
        }
        if (local_50 != (undefined4 *)0x0) {
          if (local_50 != (undefined4 *)0x0) {
            (**(code **)*local_50)(1);
          }
          local_50 = (undefined4 *)0x0;
        }
      }
      else {
        (**(code **)(*local_54 + 0x8c))(local_50,0,0);
        if (local_50 == (undefined4 *)0x0) {
          _memset(DAT_013b71e8 + (uint)*(byte *)(param_2 + 0xf) * 2 + 0x1d2,0,8);
        }
        else {
          FUN_0058f220(DAT_013b71e8 + (uint)*(byte *)(param_2 + 0xf) * 2 + 0x1d2,local_50[0x19c],8);
        }
      }
    }
    else if (*(char *)(param_2 + 0xe) == '\x01') {
      if (*(short *)local_50[0x19c] < 0x29) {
        if ((DAT_005ccec0[0x7a] != 0) && ((undefined4 *)DAT_005ccec0[0x7a] == local_50)) {
          DAT_005ccec0[0x7a] = 0;
        }
        if (local_50 != (undefined4 *)0x0) {
          if (local_50 != (undefined4 *)0x0) {
            (**(code **)*local_50)(1);
          }
          local_50 = (undefined4 *)0x0;
        }
      }
      else {
        (**(code **)(*local_54 + 0x8c))
                  (local_50,(uint)*(byte *)(param_2 + 0xf) % 9,*(byte *)(param_2 + 0xf) / 9);
        if (local_50 == (undefined4 *)0x0) {
          _memset(DAT_013b71e8 + (uint)*(byte *)(param_2 + 0xf) * 2 + 0x1f2,0,8);
        }
        else {
          FUN_0058f220(DAT_013b71e8 + (uint)*(byte *)(param_2 + 0xf) * 2 + 0x1f2,local_50[0x19c],8);
        }
      }
    }
    else if (*(char *)(param_2 + 0xe) == '\x02') {
      if (*(short *)local_50[0x19c] < 0x29) {
        if ((DAT_005ccec0[0x7a] != 0) && ((undefined4 *)DAT_005ccec0[0x7a] == local_50)) {
          DAT_005ccec0[0x7a] = 0;
        }
        if (local_50 != (undefined4 *)0x0) {
          if (local_50 != (undefined4 *)0x0) {
            (**(code **)*local_50)(1);
          }
          local_50 = (undefined4 *)0x0;
        }
      }
      else {
        (**(code **)(*local_54 + 0x8c))
                  (local_50,(uint)*(byte *)(param_2 + 0xf) % 9,*(byte *)(param_2 + 0xf) / 9);
        if (local_50 == (undefined4 *)0x0) {
          _memset(DAT_013b71e8 + (uint)*(byte *)(param_2 + 0xf) * 2 + 0xbb,0,8);
        }
        else {
          FUN_0058f220(DAT_013b71e8 + (uint)*(byte *)(param_2 + 0xf) * 2 + 0xbb,local_50[0x19c],8);
        }
      }
    }
  }
  piVar1 = DAT_013b71e8;
  if ((*(short *)(*(int *)(param_1 + 0x4c) + 0x7ae) != (short)DAT_013b71e8[0x1ec]) &&
     (*(int *)(*(int *)(param_1 + 0x4c) + 0x560) != 0)) {
    (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x560));
    *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x560) = 0;
  }
  *(short *)(*(int *)(param_1 + 0x4c) + 0x7ae) = (short)piVar1[0x1ec];
  if ((short)piVar1[0x1ea] == 0) {
    *(undefined2 *)(DAT_013b71e8 + 0x3c4) = 0xffff;
  }
  if (((*(char *)(param_2 + 0xd) == '\x0f') && (*(char *)(param_2 + 0xf) != '\x0f')) &&
     (*(int *)(param_1 + 0x4c) != 0)) {
    *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x1a0) = 0;
  }
  if ((DAT_005ccf98 != 0) && (iVar2 = FUN_00429a6d(0x1f), iVar2 != 0)) {
    uVar4 = 0;
    uVar3 = 0;
    FUN_00429a6d(0x1f);
    FUN_0042ad2b(uVar3,uVar4);
  }
  (**(code **)(*DAT_005ccec0 + 0x98))();
  DAT_005ccf08 = 0;
  FUN_004431e4(0);
  FUN_00480a83();
  return 1;
}

