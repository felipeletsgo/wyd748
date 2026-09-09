// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053bf5f | Name: FUN_0053bf5f


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_0053bf5f(int *param_1)

{
  uint uVar1;
  int iVar2;
  void *pvVar3;
  undefined4 uVar4;
  undefined4 *puVar5;
  float10 fVar6;
  float10 fVar7;
  float10 fVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  undefined4 uVar15;
  undefined4 uVar16;
  undefined4 uVar17;
  undefined4 uVar18;
  undefined4 uVar19;
  int local_138;
  int *local_134;
  int *local_130;
  int *local_12c;
  int local_ac;
  uint local_a0;
  undefined4 local_98;
  undefined4 local_94;
  int local_84;
  float local_74;
  float local_70;
  float local_6c;
  int local_68;
  float local_64;
  int local_60;
  float local_5c;
  float local_58;
  float local_54;
  float local_50;
  float local_4c;
  int local_48;
  int *local_44;
  int local_40;
  float local_3c [6];
  int local_24;
  float local_20;
  int local_1c;
  float local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a263b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  (**(code **)(*param_1 + 0x4c))();
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x5a] = (int)(((float)(uVar1 % 20000) / _DAT_005a420c) * _DAT_005a4148);
  local_14 = (int *)(**(code **)(**(int **)(DAT_013b71e8 + 0x1b084) + 0x4c))();
  if (local_14 != (int *)0x0) {
    iVar2 = (**(code **)(*local_14 + 0x5c))();
    if (iVar2 == 1) {
      *(undefined1 *)(param_1 + 0x58) = 1;
    }
    else {
      *(undefined1 *)(param_1 + 0x58) = 0;
    }
    if (((*(char *)((int)param_1 + 0x161) == '\x02') && (param_1[0x12] == 1)) &&
       (200 < uVar1 - param_1[0x5b])) {
      local_3c[0] = -1.3463969;
      local_3c[1] = 0.0;
      local_3c[2] = 1.5707964;
      local_3c[3] = 3.1415927;
      local_3c[4] = 4.712389;
      local_3c[5] = 5.105088;
      local_24 = 1;
      local_1c = 6;
      local_20 = 1.8;
      local_18 = 0.0;
      if (param_1[9] == 0x112) {
        local_24 = 0;
        local_1c = 1;
        local_20 = 1.0;
      }
      else if (param_1[9] == 0xc3) {
        local_24 = 0;
        local_1c = 4;
        local_20 = 0.8;
      }
      else if (param_1[9] == 0x2b9) {
        local_24 = 0;
        local_1c = 1;
        local_20 = -1.0;
      }
      else if (param_1[9] == 699) {
        local_24 = 0;
        local_1c = 4;
        local_20 = 1.0;
      }
      else if (((param_1[9] == 0x5f0) || (param_1[9] == 0x5ff)) || (param_1[9] == 0x69f)) {
        local_24 = 0;
        local_1c = 0;
      }
      else if (param_1[9] == 0x7c9) {
        local_24 = 0;
        local_1c = 4;
        local_20 = 1.5;
        local_18 = 0.7853982;
      }
      if (DAT_0092e660 == 0) {
        for (local_40 = local_24; local_40 < local_1c; local_40 = local_40 + 1) {
          local_48 = _rand();
          local_48 = local_48 % 5;
          pvVar3 = operator_new(0x180);
          local_8 = 0;
          if (pvVar3 == (void *)0x0) {
            local_12c = (int *)0x0;
          }
          else {
            local_12c = (int *)FUN_004c8f70(0x97,0x5dc,
                                            (float)local_48 * _DAT_005a4214 + _DAT_005a4214,
                                            (float)local_48 * _DAT_005a442c + _DAT_005a4be4,
                                            (float)local_48 * _DAT_005a4214 + _DAT_005a4214,
                                            0x3a03126f,1,0x50);
          }
          local_8 = 0xffffffff;
          local_44 = local_12c;
          if (local_12c != (int *)0x0) {
            fVar6 = (float10)FUN_00493bf0(local_18 + (float)param_1[0xd] + local_3c[local_40]);
            local_4c = (float)(-fVar6 * (float10)local_20);
            fVar6 = (float10)FUN_00423b00(local_18 + (float)param_1[0xd] + local_3c[local_40]);
            local_50 = (float)(fVar6 * (float10)local_20);
            uVar4 = FUN_00430f50((local_4c + (float)param_1[10]) - (float)local_48 * _DAT_005a439c,
                                 (float)param_1[0xc] + _DAT_005a40ec,
                                 (local_50 + (float)param_1[0xb]) - (float)local_48 * _DAT_005a439c)
            ;
            FUN_004310a0(uVar4);
            local_44[0x1b] = 0;
            local_44[0x40] = 1;
            (**(code **)(*local_44 + 0x3c))(0xffffffff);
            FUN_0054ac09(local_44);
          }
        }
      }
      param_1[0x5b] = uVar1;
    }
    else if ((*(char *)((int)param_1 + 0x161) == '\x03') && (param_1[0x12] == 1)) {
      if ((param_1[9] != 0x681) &&
         ((((param_1[9] != 0x5f6 && (param_1[9] != 0x7d5)) || (299 < uVar1 - param_1[0x5b])) &&
          (99 < uVar1 - param_1[0x5b])))) {
        local_68 = _rand();
        local_68 = local_68 % 5;
        local_54 = 0.1;
        local_64 = 3.2;
        local_58 = 0.4;
        local_60 = 2;
        local_6c = 0.0;
        local_5c = -1.9;
        if ((param_1[9] == 0x5f6) || (param_1[9] == 0x7d5)) {
          local_64 = 4.0;
          local_60 = 4;
          local_58 = 0.8;
          local_54 = 0.2;
          local_6c = 1.0;
          if (param_1[9] == 0x7d5) {
            local_64 = 2.5;
            local_6c = 6.4;
          }
          if (((*(int *)(DAT_0067cf38 + 0x40) != 0) &&
              (*(int *)(*(int *)(DAT_0067cf38 + 0x40) + 0x20a20) == 9)) &&
             (*(int *)(*(int *)(DAT_0067cf38 + 0x40) + 0x20a24) == 0x1c)) {
            local_5c = 0.0;
          }
          FUN_00430f10();
          fVar6 = (float10)FUN_00493bf0(local_6c * _DAT_005a40ec + (float)param_1[0xd] +
                                        _DAT_005a4380);
          local_74 = (float)(-fVar6 * (float10)local_64 + (float10)(float)param_1[10]);
          fVar6 = (float10)FUN_00423b00(local_6c * _DAT_005a40ec + (float)param_1[0xd] +
                                        _DAT_005a4380);
          local_70 = (float)(fVar6 * (float10)local_64 + (float10)(float)param_1[0xb]);
          fVar6 = (float10)FUN_004d0150(&local_74);
          if ((((float)fVar6 < _DAT_005a3664) && (_DAT_0092fce0 = uVar1, DAT_005ccf98 != 0)) &&
             (iVar2 = FUN_00429a6d(6), iVar2 != 0)) {
            FUN_00429a6d(6);
            iVar2 = FUN_0042afd0();
            if (iVar2 == 0) {
              uVar18 = 1;
              uVar4 = 0;
              FUN_00429a6d(6);
              FUN_0042ad2b(uVar4,uVar18);
            }
          }
        }
        if (DAT_0092e660 == 0) {
          for (local_84 = 0; local_84 < local_60; local_84 = local_84 + 1) {
            pvVar3 = operator_new(0x180);
            local_8 = 1;
            if (pvVar3 == (void *)0x0) {
              local_130 = (int *)0x0;
            }
            else {
              local_130 = (int *)FUN_004c8f70(0x97,0x5dc,
                                              (float)local_84 * _DAT_005a40ec +
                                              (float)local_68 * _DAT_005a442c + local_58,
                                              (float)local_84 * _DAT_005a40ec +
                                              (float)local_68 * _DAT_005a442c + local_58,
                                              (float)local_84 * _DAT_005a40ec +
                                              (float)local_68 * _DAT_005a442c + local_58,0x3a03126f,
                                              1,0x50);
            }
            local_8 = 0xffffffff;
            if (local_130 != (int *)0x0) {
              fVar7 = (float10)FUN_00493bf0(((local_6c * _DAT_005a40ec + (float)param_1[0xd]) -
                                            (float)local_84 * local_54) + _DAT_005a4380);
              fVar6 = (float10)local_64;
              fVar8 = (float10)FUN_00423b00(((local_6c * _DAT_005a40ec + (float)param_1[0xd]) -
                                            (float)local_84 * local_54) + _DAT_005a4380);
              if (param_1[9] == 0x5f6) {
                uVar4 = FUN_00430f50(((float)(-fVar7 * fVar6) + (float)param_1[10]) -
                                     (float)local_68 * _DAT_005a442c,local_5c + (float)param_1[0xc],
                                     ((float)(fVar8 * (float10)local_64) + (float)param_1[0xb]) -
                                     (float)local_68 * _DAT_005a442c);
                FUN_004310a0(uVar4);
              }
              else {
                uVar4 = FUN_00430f50(((float)(-fVar7 * fVar6) + (float)param_1[10]) -
                                     (float)local_68 * _DAT_005a439c,
                                     (float)param_1[0xc] + _DAT_005a40ec,
                                     ((float)(fVar8 * (float10)local_64) + (float)param_1[0xb]) -
                                     (float)local_68 * _DAT_005a439c);
                FUN_004310a0(uVar4);
              }
              local_130[0x1b] = 0;
              local_130[0x41] = 3;
              local_130[0x40] = 1;
              (**(code **)(*local_130 + 0x3c))(0xffffffff);
              FUN_0054ac09(local_130);
            }
          }
        }
        param_1[0x5b] = uVar1;
      }
    }
    else if ((*(char *)((int)param_1 + 0x161) == '\x04') && (200 < uVar1 - param_1[0x5b])) {
      local_98 = 0;
      local_94 = 0xff00aaff;
      if (DAT_005ccfa0 == 2) {
        local_98 = 0x97;
        local_94 = 0xffffffff;
      }
      local_a0 = (uint)(DAT_005ccfa0 != 2);
      if (DAT_0092e660 == 0) {
        for (local_ac = 0; local_ac < 5; local_ac = local_ac + 1) {
          iVar2 = _rand();
          iVar2 = iVar2 % 10 + -5;
          pvVar3 = operator_new(0x180);
          local_8 = 2;
          if (pvVar3 == (void *)0x0) {
            local_134 = (int *)0x0;
          }
          else {
            local_134 = (int *)FUN_004c8f70(local_98,iVar2 * 100 + 0x5dc,
                                            (float)iVar2 * _DAT_005a4214 + _DAT_005a40ec,
                                            (float)iVar2 * _DAT_005a414c + _DAT_005a430c,
                                            (float)iVar2 * _DAT_005a4214 + _DAT_005a40ec,0x38d1b717,
                                            1,0x50);
          }
          local_8 = 0xffffffff;
          if (local_134 != (int *)0x0) {
            uVar4 = FUN_00430f50((float)iVar2 * _DAT_005a439c + (float)param_1[10],
                                 (float)param_1[0xc] + _DAT_005a3660,
                                 (float)iVar2 * _DAT_005a439c + (float)param_1[0xb]);
            uVar4 = FUN_004310a0(uVar4);
            FUN_004310a0(uVar4);
            local_134[0x1b] = local_a0;
            local_134[0x40] = 0;
            local_134[0x52] = 1;
            local_134[0x50] = 0x3fc00000;
            (**(code **)(*local_134 + 0x3c))(local_94);
            FUN_0054ac09(local_134);
          }
        }
        pvVar3 = operator_new(0x90);
        local_8 = 3;
        if (pvVar3 == (void *)0x0) {
          local_138 = 0;
        }
        else {
          uVar19 = 1000;
          puVar5 = (undefined4 *)FUN_00430f50(0,0,0);
          uVar4 = *puVar5;
          uVar18 = puVar5[1];
          uVar17 = puVar5[2];
          uVar16 = 1;
          uVar15 = 0x3f800000;
          uVar14 = 0x38;
          uVar13 = 0;
          uVar12 = 0xff005588;
          uVar11 = 0x3d4ccccd;
          uVar10 = 3;
          uVar9 = 0;
          puVar5 = (undefined4 *)
                   FUN_00430f50(param_1[10],(float)param_1[0xc] + _DAT_005a3660,param_1[0xb]);
          local_138 = FUN_004d4de0(*puVar5,puVar5[1],puVar5[2],uVar9,uVar10,uVar11,uVar12,uVar13,
                                   uVar14,uVar15,uVar16,uVar4,uVar18,uVar17,uVar19);
        }
        local_8 = 0xffffffff;
        if (local_138 != 0) {
          FUN_0054ac09(local_138);
        }
      }
      fVar6 = (float10)FUN_004d0150(param_1 + 10);
      if ((((float)fVar6 < _DAT_005a40f0) && (DAT_005ccf98 != 0)) &&
         (iVar2 = FUN_00429a6d(0x27), iVar2 != 0)) {
        FUN_00429a6d(0x27);
        iVar2 = FUN_0042afd0();
        if (iVar2 == 0) {
          uVar18 = 0;
          uVar4 = 0;
          FUN_00429a6d(0x27);
          FUN_0042ad2b(uVar4,uVar18);
        }
      }
      param_1[0x5b] = uVar1;
    }
  }
  ExceptionList = local_10;
  return 1;
}

