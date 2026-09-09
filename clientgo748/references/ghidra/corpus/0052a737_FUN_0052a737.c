// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052a737 | Name: FUN_0052a737


undefined4 __thiscall FUN_0052a737(int *param_1,int param_2)

{
  void *pvVar1;
  int iVar2;
  undefined2 extraout_var;
  undefined2 extraout_var_00;
  int local_110;
  int local_10c;
  int local_104;
  undefined1 local_c4 [32];
  undefined4 local_a4;
  int local_a0;
  void *local_9c;
  int local_98;
  undefined4 local_94;
  int local_90;
  int *local_8c;
  void *local_88;
  int local_84;
  undefined4 *local_80;
  int local_7c;
  int *local_78;
  void *local_74;
  int local_70;
  undefined4 *local_6c;
  int local_68;
  int local_64;
  int local_60;
  int local_5c [4];
  undefined4 local_4c;
  undefined4 local_48;
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
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a235c;
  local_10 = ExceptionList;
  local_14 = param_2;
  local_1c = DAT_013b71e8 + 0x6ec;
  ExceptionList = &local_10;
  if (param_1 == *(int **)(DAT_0067cf38 + 0x4c)) {
    if (*(short *)(param_2 + 0xc) == 0) {
      ExceptionList = &local_10;
      if ((*(short *)(param_2 + 0xe) == 6) &&
         (ExceptionList = &local_10, local_68 = FUN_0054cd07(DAT_013b71e8 + 0x778,0x15),
         local_68 == 0x29)) {
        *(undefined2 *)((int)param_1 + 0x1ea) = 0;
        *(undefined2 *)(param_1 + 0x7b) = 0;
      }
      FUN_0058f220(local_1c + 0x5c + *(short *)(local_14 + 0xe) * 8,local_14 + 0x10,8);
      if (*(short *)(local_14 + 0xe) != 0) {
        FUN_0058f220(DAT_013b71e8 + 0xc4 + *(char *)(DAT_013b71e8 + 0xef4) * 0x80 +
                     *(short *)(local_14 + 0xe) * 8,local_14 + 0x10,8);
      }
      local_60 = DAT_0067cf38;
      local_64 = 0;
      _memset(local_5c,0,0x40);
      if (*(int *)(local_60 + 0x24) == 30000) {
        local_64 = local_60;
      }
      if (local_64 != 0) {
        local_5c[1] = *(undefined4 *)(local_64 + 0x28740);
        local_5c[2] = *(undefined4 *)(local_64 + 0x28744);
        local_5c[3] = *(undefined4 *)(local_64 + 0x28748);
        local_4c = *(undefined4 *)(local_64 + 0x2874c);
        local_48 = *(undefined4 *)(local_64 + 0x28750);
        local_44 = *(undefined4 *)(local_64 + 0x28758);
        local_40 = *(undefined4 *)(local_64 + 0x28754);
        local_3c = *(undefined4 *)(local_64 + 0x28764);
        local_38 = *(undefined4 *)(local_64 + 0x28768);
        local_34 = *(undefined4 *)(local_64 + 0x2876c);
        local_30 = *(undefined4 *)(local_64 + 0x28770);
        local_2c = *(undefined4 *)(local_64 + 0x2875c);
        local_28 = *(undefined4 *)(local_64 + 0x28760);
        local_24 = *(undefined4 *)(local_64 + 0x28774);
        local_20 = *(undefined4 *)(local_64 + 0x28778);
      }
      if ((0 < *(short *)(local_14 + 0xe)) && (*(short *)(local_14 + 0xe) < 0x10)) {
        if (local_5c[*(short *)(local_14 + 0xe)] != 0) {
          local_6c = (undefined4 *)
                     (**(code **)(*(int *)local_5c[*(short *)(local_14 + 0xe)] + 0xa4))(0,0);
          if ((DAT_005ccec0[0x7a] != 0) && ((undefined4 *)DAT_005ccec0[0x7a] == local_6c)) {
            DAT_005ccec0[0x7a] = 0;
          }
          if (local_6c != (undefined4 *)0x0) {
            if (local_6c != (undefined4 *)0x0) {
              (**(code **)*local_6c)(1);
            }
            local_6c = (undefined4 *)0x0;
          }
        }
        if (0 < *(short *)(local_14 + 0x10)) {
          local_70 = 0;
          local_74 = operator_new(8);
          if (local_74 != (void *)0x0) {
            _memset(local_74,0,8);
            FUN_0058f220(local_74,local_1c + 0x5c + *(short *)(local_14 + 0xe) * 8,8);
            pvVar1 = operator_new(0x678);
            local_8 = 0;
            if (pvVar1 == (void *)0x0) {
              local_104 = 0;
            }
            else {
              local_104 = FUN_0040d13e(0,local_74,0,0);
            }
            local_8 = 0xffffffff;
            local_70 = local_104;
            if ((local_104 != 0) && (local_5c[*(short *)(local_14 + 0xe)] != 0)) {
              (**(code **)(*(int *)local_5c[*(short *)(local_14 + 0xe)] + 0x8c))(local_104,0,0);
            }
          }
        }
      }
    }
    else if (*(short *)(param_2 + 0xc) == 1) {
      ExceptionList = &local_10;
      FUN_0058f220(DAT_013b71e8 + 0x7c8 + *(short *)(param_2 + 0xe) * 8,param_2 + 0x10,8);
      local_7c = DAT_0067cf38;
      local_78 = (int *)(**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x48))(0x151);
      local_80 = (undefined4 *)
                 (**(code **)(*local_78 + 0xa8))
                           ((int)*(short *)(local_14 + 0xe) % 9,(int)*(short *)(local_14 + 0xe) / 9)
      ;
      if ((DAT_005ccec0[0x7a] != 0) && ((undefined4 *)DAT_005ccec0[0x7a] == local_80)) {
        DAT_005ccec0[0x7a] = 0;
      }
      if (local_80 != (undefined4 *)0x0) {
        if (local_80 != (undefined4 *)0x0) {
          (**(code **)*local_80)(1);
        }
        local_80 = (undefined4 *)0x0;
      }
      if (0 < *(short *)(local_14 + 0x10)) {
        local_84 = 0;
        local_88 = operator_new(8);
        if (local_88 != (void *)0x0) {
          FUN_0058f220(local_88,local_14 + 0x10,8);
          pvVar1 = operator_new(0x678);
          local_8 = 1;
          if (pvVar1 == (void *)0x0) {
            local_10c = 0;
          }
          else {
            local_10c = FUN_0040d13e(0,local_88,0,0);
          }
          local_8 = 0xffffffff;
          local_84 = local_10c;
          if (local_10c != 0) {
            (**(code **)(*local_78 + 0x8c))
                      (local_10c,(int)*(short *)(local_14 + 0xe) % 9,
                       (int)*(short *)(local_14 + 0xe) / 9);
          }
        }
      }
    }
    else {
      ExceptionList = &local_10;
      if (*(short *)(param_2 + 0xc) == 2) {
        ExceptionList = &local_10;
        FUN_0058f220(DAT_013b71e8 + 0x2ec + *(short *)(param_2 + 0xe) * 8,param_2 + 0x10,8);
        local_90 = DAT_0067cf38;
        local_8c = (int *)(**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x48))(0x726);
        local_94 = (**(code **)(*local_8c + 0xa8))
                             ((int)*(short *)(local_14 + 0xe) % 9,
                              (int)*(short *)(local_14 + 0xe) / 9);
        if (0 < *(short *)(local_14 + 0x10)) {
          local_98 = 0;
          local_9c = operator_new(8);
          if (local_9c != (void *)0x0) {
            FUN_0058f220(local_9c,local_14 + 0x10,8);
            pvVar1 = operator_new(0x678);
            local_8 = 2;
            if (pvVar1 == (void *)0x0) {
              local_110 = 0;
            }
            else {
              local_110 = FUN_0040d13e(0,local_9c,0,0);
            }
            local_8 = 0xffffffff;
            local_98 = local_110;
            if (local_110 != 0) {
              (**(code **)(*local_8c + 0x8c))
                        (local_110,(int)*(short *)(local_14 + 0xe) % 9,
                         (int)*(short *)(local_14 + 0xe) / 9);
            }
          }
        }
      }
    }
    if (DAT_005ccec0[0x7a] != 0) {
      (**(code **)(*DAT_005ccec0 + 0x98))();
    }
  }
  FUN_00524ded(local_1c);
  FUN_005277a7((float)(int)*(short *)((int)param_1 + 0x45a));
  FUN_004faf13(CONCAT22(extraout_var,*(undefined2 *)(local_1c + 0x5c)));
  if (param_1[0x58] == 0x29) {
    *(undefined2 *)((int)param_1 + 0x1ea) = *(undefined2 *)((int)param_1 + 0x1ee);
    *(short *)(param_1 + 0x7b) = (short)param_1[0x7c];
    *(undefined1 *)(param_1 + 0x7e) = *(undefined1 *)((int)param_1 + 0x1f9);
    *(undefined1 *)(param_1 + 0x80) = *(undefined1 *)((int)param_1 + 0x201);
  }
  (**(code **)(*param_1 + 0x38))();
  FUN_0051bb41(*(undefined2 *)(local_1c + 0x8c),
               CONCAT22(extraout_var_00,*(undefined2 *)(local_1c + 0x94)));
  (**(code **)(*param_1 + 0x40))(0,param_1[0xd],0);
  if ((char)param_1[0x1e7] == '\x01') {
    local_a4 = FUN_0054cd07(local_1c + 0xcc,0x50);
    (**(code **)(*(int *)param_1[0x10e] + 0x88))(local_a4);
    local_a0 = DAT_0067cf38;
    if ((DAT_005b892c == 2) && (DAT_0067cf38 != 0)) {
      (**(code **)(**(int **)(DAT_0067cf38 + 0x27ad4) + 0x88))(local_a4);
      FUN_0058f078(local_c4,&DAT_005bb7fc,local_a4);
      (**(code **)(**(int **)(local_a0 + 0x27ac0) + 0x80))(local_c4,0);
    }
  }
  local_18 = 0;
  iVar2 = FUN_00494dcf();
  if (iVar2 == 30000) {
    local_18 = DAT_0067cf38;
  }
  if (local_18 != 0) {
    FUN_004431e4(0);
  }
  DAT_005b12bc = 0xffff;
  ExceptionList = local_10;
  return 1;
}

