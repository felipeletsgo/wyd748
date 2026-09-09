// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004a626e | Name: FUN_004a626e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_004a626e(int *param_1,undefined4 param_2,int param_3)

{
  undefined1 uVar1;
  int iVar2;
  undefined4 uVar3;
  void *pvVar4;
  undefined2 extraout_var;
  undefined2 extraout_var_00;
  char *_Str1;
  undefined2 extraout_var_01;
  undefined2 extraout_var_02;
  undefined4 uVar5;
  char *_Str2;
  int local_1c0;
  int local_1b0;
  undefined4 local_1ac;
  undefined1 local_13c [128];
  int *local_bc;
  int local_b8;
  int local_b4;
  int local_b0;
  undefined4 local_ac;
  undefined4 local_a8;
  undefined4 local_a4;
  undefined1 local_a0 [12];
  undefined1 local_94 [12];
  float local_88;
  int local_84;
  int local_80;
  int local_7c;
  int *local_78;
  int *local_74;
  int local_70;
  undefined4 local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined1 local_60 [12];
  undefined1 local_54 [12];
  float local_48;
  int local_44;
  int local_40;
  int local_3c;
  int *local_38;
  int *local_34;
  int local_30;
  undefined4 local_2c;
  int local_28;
  int local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a02cc;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar2 = FUN_0049889a(param_2,param_3);
  if (iVar2 == 1) {
    uVar3 = 1;
  }
  else {
    local_14 = param_3;
    if (param_3 == 0) {
      uVar3 = 0;
    }
    else if (*(short *)(param_3 + 4) == 0x3b4) {
      local_24 = param_3;
      local_28 = *(int *)(param_3 + 0xc);
      if ((local_28 < 0) || (3 < local_28)) {
        uVar3 = 1;
      }
      else {
        CSimpleArray<>();
        if (local_28 < 2) {
          uVar3 = FUN_00430f50(param_1[0x9bb9],0,
                               (float)local_28 * _DAT_005a4520 +
                               ((float)param_1[0x9bba] - _DAT_005a4524));
          FUN_004310a0(uVar3);
        }
        else {
          uVar3 = FUN_00430f50((float)param_1[0x9bb9] - (float)(local_28 + -1) * _DAT_005a4520,0,
                               param_1[0x9bba]);
          FUN_004310a0(uVar3);
        }
        if (param_1[0xb] != 0) {
          pvVar4 = operator_new(0x94);
          local_8 = 0;
          if (pvVar4 == (void *)0x0) {
            local_1ac = 0;
          }
          else {
            local_1ac = FUN_004e6590(local_20,local_1c,local_18,1,0x3dcccccd);
          }
          local_8 = 0xffffffff;
          local_2c = local_1ac;
          FUN_0054ac09(local_1ac);
        }
        uVar3 = 1;
      }
    }
    else if (*(short *)(param_3 + 4) == 0x110) {
      (**(code **)(*(int *)param_1[0x21] + 0x88))(0,1);
      local_30 = local_14;
      FUN_0058f220(DAT_013b71e8 + 1,local_14 + 0xc,0x2e8);
      local_34 = DAT_013b71e8 + 1;
      for (local_40 = 0; local_40 < 4; local_40 = local_40 + 1) {
        if ((local_34 != (int *)0x0) && ((char)local_34[local_40 * 4 + 4] != '\0')) {
          (**(code **)(*DAT_013b71e8 + 100))(param_1[local_40 + 0x9b8c]);
          pvVar4 = operator_new(0x894);
          local_8 = 1;
          if (pvVar4 == (void *)0x0) {
            local_1b0 = 0;
          }
          else {
            local_1b0 = FUN_004f7ea6(param_1);
          }
          local_8 = 0xffffffff;
          param_1[local_40 + 0x9b8c] = local_1b0;
          FUN_0058f078(param_1[local_40 + 0x9b8c] + 0x168,&DAT_005b9578,local_34 + local_40 * 4 + 4)
          ;
          FUN_0058f220(param_1[local_40 + 0x9b8c] + 0x444,local_34 + local_40 * 7 + 0x14,0x1c);
          *(undefined2 *)(param_1[local_40 + 0x9b8c] + 0x1d2) =
               *(undefined2 *)(&DAT_00d44990 + (short)local_34[local_40 * 0x20 + 0x30] * 0x8c);
          *(undefined2 *)(param_1[local_40 + 0x9b8c] + 0x1d4) =
               *(undefined2 *)(&DAT_00d44992 + (short)local_34[local_40 * 0x20 + 0x30] * 0x8c);
          *(undefined2 *)(param_1[local_40 + 0x9b8c] + 0x1d6) =
               *(undefined2 *)(&DAT_00d44990 + (short)local_34[local_40 * 0x20 + 0x32] * 0x8c);
          *(undefined2 *)(param_1[local_40 + 0x9b8c] + 0x1d8) =
               *(undefined2 *)(&DAT_00d44992 + (short)local_34[local_40 * 0x20 + 0x32] * 0x8c);
          *(undefined2 *)(param_1[local_40 + 0x9b8c] + 0x1da) =
               *(undefined2 *)(&DAT_00d44990 + (short)local_34[local_40 * 0x20 + 0x34] * 0x8c);
          *(undefined2 *)(param_1[local_40 + 0x9b8c] + 0x1dc) =
               *(undefined2 *)(&DAT_00d44992 + (short)local_34[local_40 * 0x20 + 0x34] * 0x8c);
          *(undefined2 *)(param_1[local_40 + 0x9b8c] + 0x1de) =
               *(undefined2 *)(&DAT_00d44990 + (short)local_34[local_40 * 0x20 + 0x36] * 0x8c);
          *(undefined2 *)(param_1[local_40 + 0x9b8c] + 0x1e0) =
               *(undefined2 *)(&DAT_00d44992 + (short)local_34[local_40 * 0x20 + 0x36] * 0x8c);
          *(undefined2 *)(param_1[local_40 + 0x9b8c] + 0x1e2) =
               *(undefined2 *)(&DAT_00d44990 + (short)local_34[local_40 * 0x20 + 0x38] * 0x8c);
          *(undefined2 *)(param_1[local_40 + 0x9b8c] + 0x1e4) =
               *(undefined2 *)(&DAT_00d44992 + (short)local_34[local_40 * 0x20 + 0x38] * 0x8c);
          *(undefined2 *)(param_1[local_40 + 0x9b8c] + 0x1e6) =
               *(undefined2 *)(&DAT_00d44990 + (short)local_34[local_40 * 0x20 + 0x3a] * 0x8c);
          *(undefined2 *)(param_1[local_40 + 0x9b8c] + 0x1e8) =
               *(undefined2 *)(&DAT_00d44992 + (short)local_34[local_40 * 0x20 + 0x3a] * 0x8c);
          *(undefined2 *)(param_1[local_40 + 0x9b8c] + 0x1ee) =
               *(undefined2 *)(&DAT_00d44990 + (short)local_34[local_40 * 0x20 + 0x3c] * 0x8c);
          *(undefined2 *)(param_1[local_40 + 0x9b8c] + 0x1f0) =
               *(undefined2 *)(&DAT_00d44992 + (short)local_34[local_40 * 0x20 + 0x3c] * 0x8c);
          *(undefined2 *)(param_1[local_40 + 0x9b8c] + 0x1ea) =
               *(undefined2 *)(&DAT_00d44990 + (short)local_34[local_40 * 0x20 + 0x3e] * 0x8c);
          *(undefined2 *)(param_1[local_40 + 0x9b8c] + 0x1ec) =
               *(undefined2 *)(&DAT_00d44992 + (short)local_34[local_40 * 0x20 + 0x3e] * 0x8c);
          *(short *)(param_1[local_40 + 0x9b8c] + 0x7ae) = (short)local_34[local_40 * 0x20 + 0x4a];
          *(undefined2 *)(param_1[local_40 + 0x9b8c] + 0x7aa) =
               *(undefined2 *)(&DAT_00d44992 + (short)local_34[local_40 * 0x20 + 0x4e] * 0x8c);
          uVar1 = FUN_0054e06c(local_34 + local_40 * 0x20 + 0x30);
          *(undefined1 *)(param_1[local_40 + 0x9b8c] + 0x1f2) = uVar1;
          uVar1 = FUN_0054e06c(local_34 + local_40 * 0x20 + 0x32);
          *(undefined1 *)(param_1[local_40 + 0x9b8c] + 499) = uVar1;
          uVar1 = FUN_0054e06c(local_34 + local_40 * 0x20 + 0x34);
          *(undefined1 *)(param_1[local_40 + 0x9b8c] + 500) = uVar1;
          uVar1 = FUN_0054e06c(local_34 + local_40 * 0x20 + 0x36);
          *(undefined1 *)(param_1[local_40 + 0x9b8c] + 0x1f5) = uVar1;
          uVar1 = FUN_0054e06c(local_34 + local_40 * 0x20 + 0x38);
          *(undefined1 *)(param_1[local_40 + 0x9b8c] + 0x1f6) = uVar1;
          uVar1 = FUN_0054e06c(local_34 + local_40 * 0x20 + 0x3a);
          *(undefined1 *)(param_1[local_40 + 0x9b8c] + 0x1f7) = uVar1;
          uVar1 = FUN_0054e06c(local_34 + local_40 * 0x20 + 0x3c);
          *(undefined1 *)(param_1[local_40 + 0x9b8c] + 0x1f9) = uVar1;
          uVar1 = FUN_0054e06c(local_34 + local_40 * 0x20 + 0x3e);
          *(undefined1 *)(param_1[local_40 + 0x9b8c] + 0x1f8) = uVar1;
          uVar1 = FUN_0054e06c(local_34 + local_40 * 0x20 + 0x4e);
          *(undefined1 *)(param_1[local_40 + 0x9b8c] + 0x7ac) = uVar1;
          *(undefined *)(param_1[local_40 + 0x9b8c] + 0x1fa) =
               (&DAT_00d449da)[(short)local_34[local_40 * 0x20 + 0x30] * 0x8c];
          *(undefined *)(param_1[local_40 + 0x9b8c] + 0x1fb) =
               (&DAT_00d449da)[(short)local_34[local_40 * 0x20 + 0x32] * 0x8c];
          *(undefined *)(param_1[local_40 + 0x9b8c] + 0x1fc) =
               (&DAT_00d449da)[(short)local_34[local_40 * 0x20 + 0x34] * 0x8c];
          *(undefined *)(param_1[local_40 + 0x9b8c] + 0x1fd) =
               (&DAT_00d449da)[(short)local_34[local_40 * 0x20 + 0x36] * 0x8c];
          *(undefined *)(param_1[local_40 + 0x9b8c] + 0x1fe) =
               (&DAT_00d449da)[(short)local_34[local_40 * 0x20 + 0x38] * 0x8c];
          *(undefined *)(param_1[local_40 + 0x9b8c] + 0x1ff) =
               (&DAT_00d449da)[(short)local_34[local_40 * 0x20 + 0x3a] * 0x8c];
          *(undefined *)(param_1[local_40 + 0x9b8c] + 0x201) =
               (&DAT_00d449da)[(short)local_34[local_40 * 0x20 + 0x3c] * 0x8c];
          *(undefined *)(param_1[local_40 + 0x9b8c] + 0x200) =
               (&DAT_00d449da)[(short)local_34[local_40 * 0x20 + 0x3e] * 0x8c];
          *(undefined *)(param_1[local_40 + 0x9b8c] + 0x7ad) =
               (&DAT_00d449da)[(short)local_34[local_40 * 0x20 + 0x4e] * 0x8c];
          uVar1 = FUN_0054df49(local_34 + local_40 * 0x20 + 0x30);
          *(undefined1 *)(param_1[local_40 + 0x9b8c] + 0x202) = uVar1;
          uVar1 = FUN_0054df49(local_34 + local_40 * 0x20 + 0x32);
          *(undefined1 *)(param_1[local_40 + 0x9b8c] + 0x203) = uVar1;
          uVar1 = FUN_0054df49(local_34 + local_40 * 0x20 + 0x34);
          *(undefined1 *)(param_1[local_40 + 0x9b8c] + 0x204) = uVar1;
          uVar1 = FUN_0054df49(local_34 + local_40 * 0x20 + 0x36);
          *(undefined1 *)(param_1[local_40 + 0x9b8c] + 0x205) = uVar1;
          uVar1 = FUN_0054df49(local_34 + local_40 * 0x20 + 0x38);
          *(undefined1 *)(param_1[local_40 + 0x9b8c] + 0x206) = uVar1;
          uVar1 = FUN_0054df49(local_34 + local_40 * 0x20 + 0x3a);
          *(undefined1 *)(param_1[local_40 + 0x9b8c] + 0x207) = uVar1;
          uVar1 = FUN_0054df49(local_34 + local_40 * 0x20 + 0x3c);
          *(undefined1 *)(param_1[local_40 + 0x9b8c] + 0x209) = uVar1;
          uVar1 = FUN_0054df49(local_34 + local_40 * 0x20 + 0x3e);
          *(undefined1 *)(param_1[local_40 + 0x9b8c] + 0x208) = uVar1;
          if ((short)local_34[local_40 * 0x20 + 0x4e] < 1) {
            *(undefined1 *)(param_1[local_40 + 0x9b8c] + 0x79b) = 0;
          }
          else {
            *(undefined1 *)(param_1[local_40 + 0x9b8c] + 0x79b) = 1;
          }
          local_48 = (float)(int)*(short *)(param_1[local_40 + 0x9b8c] + 0x45a);
          FUN_005277a7(local_48);
          FUN_004faf13(CONCAT22(extraout_var,(short)local_34[local_40 * 0x20 + 0x30]));
          *(undefined2 *)(param_1[local_40 + 0x9b8c] + 0x450) = 1;
          local_44 = FUN_0054cd07(local_34 + local_40 * 0x20 + 0x3c,0x15);
          if (local_44 == 0x29) {
            *(undefined2 *)(param_1[local_40 + 0x9b8c] + 0x1ea) =
                 *(undefined2 *)(param_1[local_40 + 0x9b8c] + 0x1ee);
            *(undefined2 *)(param_1[local_40 + 0x9b8c] + 0x1ec) =
                 *(undefined2 *)(param_1[local_40 + 0x9b8c] + 0x1f0);
          }
          (**(code **)(*(int *)param_1[local_40 + 0x9b8c] + 0x38))();
          FUN_0051bb41(CONCAT22(extraout_var_01,(short)local_34[local_40 * 0x20 + 0x3c]),
                       CONCAT22(extraout_var_01,(short)local_34[local_40 * 0x20 + 0x3e]));
          if (DAT_005b892c == 2) {
            (**(code **)(*(int *)param_1[local_40 + 0x9b8c] + 0x40))(0,0xc2340000,0);
            CSimpleArray<>();
            CSimpleArray<>();
            CSimpleArray<>();
            uVar3 = FUN_00430f50(0x45005000,0,0x45000333);
            FUN_004310a0(uVar3);
            uVar3 = FUN_00430f50(0x45000333,0,0x45005000);
            FUN_004310a0(uVar3);
            FUN_004a8940(&local_6c,local_54,local_60,(float)local_40 * _DAT_005a4528);
            (**(code **)(*(int *)param_1[local_40 + 0x9b8c] + 0x3c))(local_6c,local_68,local_64);
          }
          else {
            (**(code **)(*(int *)param_1[local_40 + 0x9b8c] + 0x40))
                      (0,((float)-local_40 * _DAT_005a4148) / _DAT_005a4104,0);
            if (local_40 < 2) {
              (**(code **)(*(int *)param_1[local_40 + 0x9b8c] + 0x3c))
                        (param_1[0x9bb9],0,
                         (float)local_40 * _DAT_005a4520 + ((float)param_1[0x9bba] - _DAT_005a4524))
              ;
            }
            else {
              (**(code **)(*(int *)param_1[local_40 + 0x9b8c] + 0x3c))
                        ((float)param_1[0x9bb9] - (float)(local_40 + -1) * _DAT_005a4520,0,
                         param_1[0x9bba]);
            }
          }
          FUN_0054ac09(param_1[local_40 + 0x9b8c]);
        }
      }
      if ((DAT_005ccf98 != 0) && (iVar2 = FUN_00429a6d(0x39), iVar2 != 0)) {
        uVar5 = 0;
        uVar3 = 0;
        FUN_00429a6d(0x39);
        FUN_0042ad2b(uVar3,uVar5);
      }
      (**(code **)(*param_1 + 0x68))(1);
      local_3c = 0;
      for (local_40 = 0; local_40 < 4; local_40 = local_40 + 1) {
        if ((char)local_34[local_40 * 4 + 4] == '\0') {
          local_3c = 1;
          break;
        }
      }
      local_38 = (int *)(**(code **)(*(int *)param_1[10] + 0x48))(0x1205);
      if (local_3c == 0) {
        (**(code **)(*local_38 + 100))(0);
      }
      else {
        (**(code **)(*local_38 + 100))(1);
      }
      uVar3 = 1;
    }
    else if (*(short *)(param_3 + 4) == 0x11a) {
      FUN_00403df2(&DAT_00a34af8,2000);
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
      uVar3 = 1;
    }
    else if (*(short *)(param_3 + 4) == 0x112) {
      (**(code **)(*(int *)param_1[0x21] + 0x88))(0,1);
      local_70 = local_14;
      FUN_0058f220(DAT_013b71e8 + 1,local_14 + 0xc,0x2e8);
      local_74 = DAT_013b71e8 + 1;
      if (DAT_005b892c == 2) {
        (**(code **)(*param_1 + 0x58))(0x1208,0);
      }
      for (local_80 = 0; local_80 < 4; local_80 = local_80 + 1) {
        if ((local_74 == (int *)0x0) || ((char)local_74[local_80 * 4 + 4] != '\0')) {
          if ((local_74 != (int *)0x0) && ((char)local_74[local_80 * 4 + 4] != '\0')) {
            (**(code **)(*DAT_013b71e8 + 100))(param_1[local_80 + 0x9b8c]);
            param_1[local_80 + 0x9b8c] = 0;
            pvVar4 = operator_new(0x894);
            local_8 = 2;
            if (pvVar4 == (void *)0x0) {
              local_1c0 = 0;
            }
            else {
              local_1c0 = FUN_004f7ea6(param_1);
            }
            local_8 = 0xffffffff;
            param_1[local_80 + 0x9b8c] = local_1c0;
            FUN_0058f078(param_1[local_80 + 0x9b8c] + 0x168,&DAT_005b957c,
                         local_74 + local_80 * 4 + 4);
            FUN_0058f220(param_1[local_80 + 0x9b8c] + 0x444,local_74 + local_80 * 7 + 0x14,0x1c);
            *(undefined2 *)(param_1[local_80 + 0x9b8c] + 0x1d2) =
                 *(undefined2 *)(&DAT_00d44990 + (short)local_74[local_80 * 0x20 + 0x30] * 0x8c);
            *(undefined2 *)(param_1[local_80 + 0x9b8c] + 0x1d4) =
                 *(undefined2 *)(&DAT_00d44992 + (short)local_74[local_80 * 0x20 + 0x30] * 0x8c);
            *(undefined2 *)(param_1[local_80 + 0x9b8c] + 0x1d6) =
                 *(undefined2 *)(&DAT_00d44990 + (short)local_74[local_80 * 0x20 + 0x32] * 0x8c);
            *(undefined2 *)(param_1[local_80 + 0x9b8c] + 0x1d8) =
                 *(undefined2 *)(&DAT_00d44992 + (short)local_74[local_80 * 0x20 + 0x32] * 0x8c);
            *(undefined2 *)(param_1[local_80 + 0x9b8c] + 0x1da) =
                 *(undefined2 *)(&DAT_00d44990 + (short)local_74[local_80 * 0x20 + 0x34] * 0x8c);
            *(undefined2 *)(param_1[local_80 + 0x9b8c] + 0x1dc) =
                 *(undefined2 *)(&DAT_00d44992 + (short)local_74[local_80 * 0x20 + 0x34] * 0x8c);
            *(undefined2 *)(param_1[local_80 + 0x9b8c] + 0x1de) =
                 *(undefined2 *)(&DAT_00d44990 + (short)local_74[local_80 * 0x20 + 0x36] * 0x8c);
            *(undefined2 *)(param_1[local_80 + 0x9b8c] + 0x1e0) =
                 *(undefined2 *)(&DAT_00d44992 + (short)local_74[local_80 * 0x20 + 0x36] * 0x8c);
            *(undefined2 *)(param_1[local_80 + 0x9b8c] + 0x1e2) =
                 *(undefined2 *)(&DAT_00d44990 + (short)local_74[local_80 * 0x20 + 0x38] * 0x8c);
            *(undefined2 *)(param_1[local_80 + 0x9b8c] + 0x1e4) =
                 *(undefined2 *)(&DAT_00d44992 + (short)local_74[local_80 * 0x20 + 0x38] * 0x8c);
            *(undefined2 *)(param_1[local_80 + 0x9b8c] + 0x1e6) =
                 *(undefined2 *)(&DAT_00d44990 + (short)local_74[local_80 * 0x20 + 0x3a] * 0x8c);
            *(undefined2 *)(param_1[local_80 + 0x9b8c] + 0x1e8) =
                 *(undefined2 *)(&DAT_00d44992 + (short)local_74[local_80 * 0x20 + 0x3a] * 0x8c);
            *(undefined2 *)(param_1[local_80 + 0x9b8c] + 0x1ee) =
                 *(undefined2 *)(&DAT_00d44990 + (short)local_74[local_80 * 0x20 + 0x3c] * 0x8c);
            *(undefined2 *)(param_1[local_80 + 0x9b8c] + 0x1f0) =
                 *(undefined2 *)(&DAT_00d44992 + (short)local_74[local_80 * 0x20 + 0x3c] * 0x8c);
            *(undefined2 *)(param_1[local_80 + 0x9b8c] + 0x1ea) =
                 *(undefined2 *)(&DAT_00d44990 + (short)local_74[local_80 * 0x20 + 0x3e] * 0x8c);
            *(undefined2 *)(param_1[local_80 + 0x9b8c] + 0x1ec) =
                 *(undefined2 *)(&DAT_00d44992 + (short)local_74[local_80 * 0x20 + 0x3e] * 0x8c);
            *(short *)(param_1[local_80 + 0x9b8c] + 0x7ae) = (short)local_74[local_80 * 0x20 + 0x4a]
            ;
            *(undefined2 *)(param_1[local_80 + 0x9b8c] + 0x7aa) =
                 *(undefined2 *)(&DAT_00d44992 + (short)local_74[local_80 * 0x20 + 0x4e] * 0x8c);
            uVar1 = FUN_0054e06c(local_74 + local_80 * 0x20 + 0x30);
            *(undefined1 *)(param_1[local_80 + 0x9b8c] + 0x1f2) = uVar1;
            uVar1 = FUN_0054e06c(local_74 + local_80 * 0x20 + 0x32);
            *(undefined1 *)(param_1[local_80 + 0x9b8c] + 499) = uVar1;
            uVar1 = FUN_0054e06c(local_74 + local_80 * 0x20 + 0x34);
            *(undefined1 *)(param_1[local_80 + 0x9b8c] + 500) = uVar1;
            uVar1 = FUN_0054e06c(local_74 + local_80 * 0x20 + 0x36);
            *(undefined1 *)(param_1[local_80 + 0x9b8c] + 0x1f5) = uVar1;
            uVar1 = FUN_0054e06c(local_74 + local_80 * 0x20 + 0x38);
            *(undefined1 *)(param_1[local_80 + 0x9b8c] + 0x1f6) = uVar1;
            uVar1 = FUN_0054e06c(local_74 + local_80 * 0x20 + 0x3a);
            *(undefined1 *)(param_1[local_80 + 0x9b8c] + 0x1f7) = uVar1;
            uVar1 = FUN_0054e06c(local_74 + local_80 * 0x20 + 0x3c);
            *(undefined1 *)(param_1[local_80 + 0x9b8c] + 0x1f9) = uVar1;
            uVar1 = FUN_0054e06c(local_74 + local_80 * 0x20 + 0x3e);
            *(undefined1 *)(param_1[local_80 + 0x9b8c] + 0x1f8) = uVar1;
            *(undefined *)(param_1[local_80 + 0x9b8c] + 0x1fa) =
                 (&DAT_00d449da)[(short)local_74[local_80 * 0x20 + 0x30] * 0x8c];
            *(undefined *)(param_1[local_80 + 0x9b8c] + 0x1fb) =
                 (&DAT_00d449da)[(short)local_74[local_80 * 0x20 + 0x32] * 0x8c];
            *(undefined *)(param_1[local_80 + 0x9b8c] + 0x1fc) =
                 (&DAT_00d449da)[(short)local_74[local_80 * 0x20 + 0x34] * 0x8c];
            *(undefined *)(param_1[local_80 + 0x9b8c] + 0x1fd) =
                 (&DAT_00d449da)[(short)local_74[local_80 * 0x20 + 0x36] * 0x8c];
            *(undefined *)(param_1[local_80 + 0x9b8c] + 0x1fe) =
                 (&DAT_00d449da)[(short)local_74[local_80 * 0x20 + 0x38] * 0x8c];
            *(undefined *)(param_1[local_80 + 0x9b8c] + 0x1ff) =
                 (&DAT_00d449da)[(short)local_74[local_80 * 0x20 + 0x3a] * 0x8c];
            *(undefined *)(param_1[local_80 + 0x9b8c] + 0x201) =
                 (&DAT_00d449da)[(short)local_74[local_80 * 0x20 + 0x3c] * 0x8c];
            *(undefined *)(param_1[local_80 + 0x9b8c] + 0x200) =
                 (&DAT_00d449da)[(short)local_74[local_80 * 0x20 + 0x3e] * 0x8c];
            uVar1 = FUN_0054df49(local_74 + local_80 * 0x20 + 0x30);
            *(undefined1 *)(param_1[local_80 + 0x9b8c] + 0x202) = uVar1;
            uVar1 = FUN_0054df49(local_74 + local_80 * 0x20 + 0x32);
            *(undefined1 *)(param_1[local_80 + 0x9b8c] + 0x203) = uVar1;
            uVar1 = FUN_0054df49(local_74 + local_80 * 0x20 + 0x34);
            *(undefined1 *)(param_1[local_80 + 0x9b8c] + 0x204) = uVar1;
            uVar1 = FUN_0054df49(local_74 + local_80 * 0x20 + 0x36);
            *(undefined1 *)(param_1[local_80 + 0x9b8c] + 0x205) = uVar1;
            uVar1 = FUN_0054df49(local_74 + local_80 * 0x20 + 0x38);
            *(undefined1 *)(param_1[local_80 + 0x9b8c] + 0x206) = uVar1;
            uVar1 = FUN_0054df49(local_74 + local_80 * 0x20 + 0x3a);
            *(undefined1 *)(param_1[local_80 + 0x9b8c] + 0x207) = uVar1;
            uVar1 = FUN_0054df49(local_74 + local_80 * 0x20 + 0x3c);
            *(undefined1 *)(param_1[local_80 + 0x9b8c] + 0x209) = uVar1;
            uVar1 = FUN_0054df49(local_74 + local_80 * 0x20 + 0x3e);
            *(undefined1 *)(param_1[local_80 + 0x9b8c] + 0x208) = uVar1;
            if ((short)local_74[local_80 * 0x20 + 0x4e] < 1) {
              *(undefined1 *)(param_1[local_80 + 0x9b8c] + 0x79b) = 0;
            }
            else {
              *(undefined1 *)(param_1[local_80 + 0x9b8c] + 0x79b) = 1;
            }
            local_88 = (float)(int)*(short *)(param_1[local_80 + 0x9b8c] + 0x45a);
            FUN_005277a7(local_88);
            FUN_004faf13(CONCAT22(extraout_var_00,(short)local_74[local_80 * 0x20 + 0x30]));
            *(undefined2 *)(param_1[local_80 + 0x9b8c] + 0x450) = 1;
            *(undefined4 *)(param_1[local_80 + 0x9b8c] + 0x220) = 0x40a00000;
            local_84 = FUN_0054cd07(local_74 + local_80 * 0x20 + 0x3c,0x15);
            if (local_84 == 0x29) {
              *(undefined2 *)(param_1[local_80 + 0x9b8c] + 0x1ea) =
                   *(undefined2 *)(param_1[local_80 + 0x9b8c] + 0x1ee);
              *(undefined2 *)(param_1[local_80 + 0x9b8c] + 0x1ec) =
                   *(undefined2 *)(param_1[local_80 + 0x9b8c] + 0x1f0);
            }
            (**(code **)(*(int *)param_1[local_80 + 0x9b8c] + 0x38))();
            FUN_0051bb41(CONCAT22(extraout_var_02,(short)local_74[local_80 * 0x20 + 0x3c]),
                         CONCAT22(extraout_var_02,(short)local_74[local_80 * 0x20 + 0x3e]));
            if (DAT_005b892c == 2) {
              (**(code **)(*(int *)param_1[local_80 + 0x9b8c] + 0x40))(0,0xc2340000,0);
              CSimpleArray<>();
              CSimpleArray<>();
              CSimpleArray<>();
              uVar3 = FUN_00430f50(0x45005000,0,0x45000333);
              FUN_004310a0(uVar3);
              uVar3 = FUN_00430f50(0x45000333,0,0x45005000);
              FUN_004310a0(uVar3);
              FUN_004a8940(&local_ac,local_94,local_a0,(float)local_80 * _DAT_005a4528);
              (**(code **)(*(int *)param_1[local_80 + 0x9b8c] + 0x3c))(local_ac,local_a8,local_a4);
            }
            else {
              (**(code **)(*(int *)param_1[local_80 + 0x9b8c] + 0x40))
                        (0,((float)-local_80 * _DAT_005a4148) / _DAT_005a4104,0);
              if (local_80 < 2) {
                (**(code **)(*(int *)param_1[local_80 + 0x9b8c] + 0x3c))
                          (param_1[0x9bb9],0,
                           (float)local_80 * _DAT_005a4520 +
                           ((float)param_1[0x9bba] - _DAT_005a4524));
              }
              else {
                (**(code **)(*(int *)param_1[local_80 + 0x9b8c] + 0x3c))
                          ((float)param_1[0x9bb9] - (float)(local_80 + -1) * _DAT_005a4520,0,
                           param_1[0x9bba]);
              }
            }
            FUN_0054ac09(param_1[local_80 + 0x9b8c]);
          }
        }
        else {
          (**(code **)(*DAT_013b71e8 + 100))(param_1[local_80 + 0x9b8c]);
          param_1[local_80 + 0x9b8c] = 0;
        }
      }
      local_7c = 0;
      for (local_80 = 0; local_80 < 4; local_80 = local_80 + 1) {
        if ((char)local_74[local_80 * 4 + 4] == '\0') {
          local_7c = 1;
          break;
        }
      }
      local_78 = (int *)(**(code **)(*(int *)param_1[10] + 0x48))(0x1205);
      if (local_7c == 0) {
        (**(code **)(*local_78 + 100))(0);
      }
      else {
        (**(code **)(*local_78 + 100))(1);
      }
      uVar3 = 1;
    }
    else if (*(short *)(param_3 + 4) == 0x11b) {
      FUN_00403df2(&DAT_00a34b78,2000);
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
      uVar3 = 1;
    }
    else if (*(short *)(param_3 + 4) == 0x114) {
      (**(code **)(*DAT_0092e654 + 4))(*(undefined4 *)(param_3 + 8));
      local_b4 = local_14;
      DAT_013b71e8[0x317] = (uint)*(ushort *)(local_14 + 0x306);
      FUN_0058f220(DAT_013b71e8 + 0x1bb,local_14 + 0x10,0x2f4);
      DAT_013b71e8[0x314] = *(int *)(local_b4 + 0x31c);
      *(undefined2 *)(DAT_013b71e8 + 0x1c3) = *(undefined2 *)(local_b4 + 0xc);
      *(undefined2 *)((int)DAT_013b71e8 + 0x70e) = *(undefined2 *)(local_b4 + 0xe);
      FUN_0058f220((int)DAT_013b71e8 + 0xef6,DAT_013b71e8 + 0x275,4);
      FUN_0058f220((int)DAT_013b71e8 + 0xefa,local_b4 + 0x30a,0x10);
      for (local_b0 = 0; local_b0 < 0x14; local_b0 = local_b0 + 1) {
        if ((-1 < *(char *)((int)DAT_013b71e8 + local_b0 + 0xef6)) &&
           (*(char *)((int)DAT_013b71e8 + local_b0 + 0xef6) < '\x18')) {
          *(char *)((int)DAT_013b71e8 + local_b0 + 0xef6) =
               *(char *)((int)DAT_013b71e8 + local_b0 + 0xef6) + (char)DAT_013b71e8[0x1c0] * '\x18';
        }
      }
      DAT_005d03e4 = (uint)*(ushort *)(local_b4 + 0x308);
      (**(code **)(*DAT_013b71e8 + 0x54))(0);
      uVar3 = 1;
    }
    else if (*(short *)(param_3 + 4) == 0x119) {
      FUN_00403df2(&DAT_00a34bf8,2000);
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
      uVar3 = 1;
    }
    else {
      if (*(short *)(param_3 + 4) == 0x7a9) {
        DAT_005d04e0 = 1;
      }
      else {
        if (*(short *)(param_3 + 4) == 0xfaa) {
          local_b8 = param_3;
          param_1[0x9b87] = 0;
          if (*(int *)(param_3 + 0x10) < 0) {
            ExceptionList = local_10;
            return 1;
          }
          if (*(int *)(param_3 + 0xc) == 0) {
            _Str2 = &DAT_005d04ec;
            _Str1 = (char *)(**(code **)(*(int *)param_1[0x9b9b] + 0x88))();
            iVar2 = _strcmp(_Str1,_Str2);
            if (iVar2 == 0) {
              FUN_0058f078(local_13c,&DAT_00a3a5f8,
                           param_1[*(int *)(local_b8 + 0x10) + 0x9b8c] + 0x168);
            }
            else {
              uVar3 = (**(code **)(*(int *)param_1[0x9b9b] + 0x88))();
              FUN_0058f078(local_13c,&DAT_00a3a5f8,uVar3);
              (**(code **)(*(int *)param_1[0x9b9b] + 0x90))(&DAT_005d04f0);
            }
            FUN_00403df2(local_13c,0xdac);
            (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
            *(undefined2 *)((int)DAT_013b71e8 + *(int *)(local_b8 + 0x10) * 2 + 4) = 0;
            *(undefined2 *)((int)DAT_013b71e8 + *(int *)(local_b8 + 0x10) * 2 + 0xc) = 0;
            *(undefined2 *)((int)DAT_013b71e8 + *(int *)(local_b8 + 0x10) * 2 + 0x2c4) = 0;
            DAT_013b71e8[*(int *)(local_b8 + 0x10) + 0xb3] = 0;
            DAT_013b71e8[*(int *)(local_b8 + 0x10) + 0xb7] = 0;
            _memset(DAT_013b71e8 + *(int *)(local_b8 + 0x10) * 4 + 5,0,0x10);
            _memset(DAT_013b71e8 + *(int *)(local_b8 + 0x10) * 0x20 + 0x31,0,0x80);
            _memset(DAT_013b71e8 + *(int *)(local_b8 + 0x10) * 7 + 0x15,0,0x1c);
            (**(code **)(*DAT_013b71e8 + 100))(param_1[*(int *)(local_b8 + 0x10) + 0x9b8c]);
            param_1[*(int *)(local_b8 + 0x10) + 0x9b8c] = 0;
            *(undefined1 *)(DAT_013b71e8 + 0x3bd) = 0xff;
            local_bc = (int *)(**(code **)(*(int *)param_1[10] + 0x48))(0x1205);
            (**(code **)(*local_bc + 100))(1);
          }
          else if (*(int *)(param_3 + 0xc) == 1) {
            FUN_00403df2(&DAT_00a3a678,0xdac);
            (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
            (**(code **)(*(int *)param_1[0x9b9a] + 0x60))(1);
          }
          else if (*(int *)(param_3 + 0xc) == 2) {
            FUN_00403df2(&DAT_00a3a6f8,0xdac);
            (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
          }
          else if (*(int *)(param_3 + 0xc) == 3) {
            FUN_00403df2(&DAT_00a3a7f8,0xdac);
            (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
          }
          else {
            FUN_00403df2(&DAT_00a3a778,0xdac);
            (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
          }
          ExceptionList = local_10;
          return 1;
        }
        if (*(short *)(param_3 + 4) == 0x1d6) {
          FUN_0058f220(DAT_013b71e8 + *(int *)(param_3 + 0xc) * 3 + 0x3c6,param_3 + 0x10,0xc);
        }
      }
      uVar3 = 0;
    }
  }
  ExceptionList = local_10;
  return uVar3;
}

