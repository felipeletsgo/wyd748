// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004cf90d | Name: FUN_004cf90d


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004cf90d(int *param_1)

{
  undefined4 uVar1;
  int iVar2;
  void *pvVar3;
  float10 fVar4;
  undefined4 uVar5;
  int local_d8;
  int local_cc;
  int *local_c4;
  int local_88;
  int local_84;
  float local_80;
  int local_7c;
  float local_78;
  float local_74;
  float local_70;
  int local_6c;
  float local_68;
  int local_64;
  undefined1 local_60 [8];
  float local_58;
  undefined4 local_54;
  int local_50;
  undefined4 local_4c;
  undefined4 local_48;
  float local_44;
  int *local_40;
  float local_3c;
  int local_38;
  undefined4 local_34;
  int local_30;
  float local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  int local_1c;
  float local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0908;
  local_10 = ExceptionList;
  if ((((param_1[6] == -1) || (param_1[6] == DAT_005bdef0)) || (param_1[6] == DAT_005bdef4)) ||
     ((param_1[6] == DAT_005bdef8 || (param_1[6] == DAT_005bdefc)))) {
    ExceptionList = &local_10;
    iVar2 = (**(code **)(*DAT_0092e654 + 8))();
    (**(code **)(*param_1 + 0x38))();
    local_14 = DAT_0067cf38;
    local_18 = (float)(uint)(iVar2 - param_1[0x21]) / (float)param_1[0x22];
    if (_DAT_005a3660 < local_18) {
      if (param_1[0x20] != 1) {
        local_84 = param_1[0x1d];
        local_7c = param_1[0x1f];
        local_80 = (float)param_1[0x1e] - _DAT_005a4154;
        for (local_88 = 0; local_88 < 3; local_88 = local_88 + 1) {
          iVar2 = _rand();
          iVar2 = iVar2 % 5;
          pvVar3 = operator_new(0x180);
          local_8 = 2;
          if (pvVar3 == (void *)0x0) {
            local_d8 = 0;
          }
          else {
            local_d8 = FUN_004c8f70(0x77,local_88 * 100 + 3000,
                                    (float)iVar2 * _DAT_005a4214 + (float)param_1[0x1c],
                                    (float)iVar2 * _DAT_005a40ec + (float)param_1[0x1c],
                                    (float)iVar2 * _DAT_005a4214 + (float)param_1[0x1c],0x38d1b717,1
                                    ,0x50);
          }
          local_8 = 0xffffffff;
          if (local_d8 != 0) {
            uVar1 = FUN_004310a0(&local_84);
            FUN_004310a0(uVar1);
            *(undefined4 *)(local_d8 + 0x100) = 1;
            FUN_0054ac09(local_d8);
          }
        }
      }
      (**(code **)(*DAT_013b71e8 + 100))(param_1);
    }
    else if (param_1[0x20] == 1) {
      for (local_1c = 0; local_1c < 4; local_1c = local_1c + 1) {
        param_1[local_1c * 3 + 0x28] =
             (int)(((float)param_1[0x1e] + _DAT_005a3794 + (float)(local_1c * 5)) -
                  local_18 * _DAT_005a4924);
        if (((float)param_1[local_1c * 3 + 0x28] < (float)param_1[0x1e]) &&
           (*(char *)((int)param_1 + local_1c + 0xcc) == '\0')) {
          *(undefined1 *)((int)param_1 + local_1c + 0xcc) = 1;
          local_34 = 0x38d1b717;
          local_28 = 0;
          local_2c = 0.15;
          local_3c = 0.0;
          local_30 = 0x77;
          FUN_00430f20(param_1[local_1c * 3 + 0x27],param_1[local_1c * 3 + 0x29]);
          iVar2 = FUN_0049d2b9(local_24,local_20,&local_28);
          if (iVar2 == 1) {
            local_30 = 0x97;
            local_2c = 0.1;
            local_34 = 0x3951b717;
            local_3c = 0.2;
          }
          if (local_30 == 0x97) {
            pvVar3 = operator_new(0x124);
            local_8 = 0;
            if (pvVar3 == (void *)0x0) {
              local_c4 = (int *)0x0;
            }
            else {
              local_c4 = (int *)FUN_004cb900(0x96,1000,0x3c23d70a,0x3c23d70a,0x3c23d70a,0x3ac49ba6,0
                                            );
            }
            local_8 = 0xffffffff;
            local_40 = local_c4;
            if (local_c4 != (int *)0x0) {
              local_c4[0x1b] = 1;
              (**(code **)(*local_c4 + 0x3c))(0xaaaaaaaa);
              local_40[0x1d] = param_1[local_1c * 3 + 0x27];
              local_40[0x1f] = param_1[local_1c * 3 + 0x29];
              local_54 = DAT_0067cf38;
              FUN_00430f20(local_40[0x1d],local_40[0x1f]);
              iVar2 = FUN_0049c70f(local_4c,local_48);
              local_44 = (float)iVar2 + _DAT_005a4150;
              if (local_44 == 127.0) {
                local_44 = 0.0;
              }
              local_40[0x1e] = (int)(local_44 * _DAT_005a4214 + _DAT_005a4150);
              FUN_0054ac09(local_40);
              local_50 = DAT_013b71e8[0x6c21];
              if (*(int *)(local_50 + 0x54) != 0) {
                fVar4 = (float10)FUN_004d0150(&local_4c);
                local_58 = (float)fVar4;
                if (((local_58 < _DAT_005a4104) && (DAT_005ccf98 != 0)) &&
                   (iVar2 = FUN_00429a6d(0xe), iVar2 != 0)) {
                  uVar5 = 0;
                  uVar1 = 0;
                  FUN_00429a6d(0xe);
                  FUN_0042ad2b(uVar1,uVar5);
                }
              }
            }
          }
          else {
            local_64 = DAT_013b71e8[0x6c21];
            FUN_00430f20(param_1[local_1c * 3 + 0x27],param_1[local_1c * 3 + 0x29]);
            if (*(int *)(local_64 + 0x54) != 0) {
              fVar4 = (float10)FUN_004d0150(local_60);
              local_68 = (float)fVar4;
              if (((local_68 < _DAT_005a4104) && (DAT_005ccf98 != 0)) &&
                 (iVar2 = FUN_00429a6d(0xd), iVar2 != 0)) {
                uVar5 = 0;
                uVar1 = 0;
                FUN_00429a6d(0xd);
                FUN_0042ad2b(uVar1,uVar5);
              }
            }
          }
          for (local_38 = 0; local_38 < 3; local_38 = local_38 + 1) {
            pvVar3 = operator_new(0x180);
            local_8 = 1;
            if (pvVar3 == (void *)0x0) {
              local_cc = 0;
            }
            else {
              local_cc = FUN_004c8f70(local_30,3000,0x3f000000,0x3fe66666,0x3f000000,local_34,1,0x50
                                     );
            }
            local_8 = 0xffffffff;
            local_6c = local_cc;
            if (local_cc != 0) {
              *(undefined4 *)(local_cc + 0x104) = 3;
              local_74 = (float)local_1c * local_3c + (float)param_1[0x1e];
              local_78 = (float)(local_38 + -1) * local_2c + (float)param_1[local_1c * 3 + 0x27];
              local_70 = (float)(local_38 + -1) * local_2c +
                         (float)(param_1 + local_1c * 3 + 0x27)[2];
              uVar1 = FUN_004310a0(&local_78);
              FUN_004310a0(uVar1);
              FUN_0054ac09(local_6c);
            }
          }
        }
        if ((float)param_1[local_1c * 3 + 0x28] < (float)param_1[0x1e]) {
          param_1[local_1c * 3 + 0x28] = param_1[0x1e];
        }
      }
    }
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  ExceptionList = local_10;
  return uVar1;
}

