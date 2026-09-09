// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004776c3 | Name: FUN_004776c3


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004776c3(int *param_1)

{
  short sVar1;
  POINT pt;
  POINT pt_00;
  POINT pt_01;
  float fVar2;
  bool bVar3;
  byte bVar4;
  undefined1 uVar5;
  undefined2 uVar6;
  uint uVar7;
  int iVar8;
  uint uVar9;
  size_t sVar10;
  void *pvVar11;
  BOOL BVar12;
  undefined4 uVar13;
  undefined4 *puVar14;
  float10 extraout_ST0;
  float10 extraout_ST0_00;
  float10 fVar15;
  undefined4 uVar16;
  int local_8ac;
  int local_8a8;
  int local_894;
  int local_880;
  int local_874;
  int local_864;
  int *local_85c;
  int *local_858;
  int *local_850;
  undefined4 local_830;
  undefined4 local_828;
  undefined4 local_81c;
  undefined4 local_814;
  undefined4 local_80c;
  undefined4 local_800;
  undefined4 local_7f4;
  undefined4 local_7e8;
  undefined4 local_7dc;
  undefined4 local_7d4;
  int local_7c4;
  int local_7a4;
  undefined1 local_698 [256];
  undefined1 local_598 [256];
  int local_498;
  int local_494;
  int local_490;
  int local_48c;
  int local_488;
  byte local_484;
  undefined3 uStack_483;
  int local_480;
  uint local_47c;
  int local_478;
  uint local_474;
  uint local_470;
  undefined1 local_46c [128];
  int local_3ec;
  uint local_3e8;
  uint local_3e4;
  int local_3e0;
  int local_3dc;
  undefined1 local_3d8 [128];
  int local_358;
  float local_354;
  int local_350;
  uint local_34c;
  char local_348;
  int local_344;
  undefined4 local_340;
  float local_33c;
  int local_338;
  byte local_334;
  undefined3 uStack_333;
  int local_330;
  int local_32c;
  int local_328;
  undefined4 local_324;
  undefined4 local_320;
  undefined4 local_31c;
  undefined4 local_318;
  undefined4 local_314;
  undefined4 local_310;
  int local_30c;
  int local_308;
  undefined4 local_304;
  float local_300;
  int local_2fc;
  int local_2f8;
  int *local_2f4;
  int *local_2f0;
  int local_2ec;
  int local_2e8;
  undefined4 local_2e4;
  int local_2e0;
  float local_2dc;
  uint local_2d8;
  float local_2d4;
  uint local_2d0;
  float local_2cc;
  int local_2c8;
  int local_2c4;
  undefined1 local_2c0 [4];
  undefined2 local_2bc;
  undefined2 uStack_2ba;
  short local_2b4;
  short local_2b2;
  undefined4 local_2b0;
  undefined4 local_2ac;
  ushort local_2a8;
  ushort uStack_2a6;
  undefined1 auStack_2a4 [24];
  int local_28c;
  undefined1 local_288 [4];
  undefined2 local_284;
  undefined2 uStack_282;
  short local_27c;
  short local_27a;
  undefined4 local_278;
  undefined4 local_274;
  ushort local_270;
  ushort uStack_26e;
  undefined1 auStack_26c [24];
  int local_254;
  int local_250;
  float local_24c;
  float local_248;
  float local_244;
  int local_240;
  int local_23c;
  undefined1 local_238 [128];
  int local_1b8;
  undefined1 local_1b4;
  undefined4 local_1b3;
  undefined1 local_174 [64];
  LONG local_134;
  LONG local_130;
  int local_12c;
  int local_128;
  int local_124;
  int local_120;
  int local_11c;
  undefined4 local_118;
  int local_114;
  int local_110;
  int local_10c;
  int local_108;
  undefined1 local_104 [4];
  undefined2 local_100;
  undefined2 local_fe;
  int local_f8;
  int local_f4;
  LONG local_f0;
  LONG local_ec;
  int local_e8;
  int local_e4;
  undefined1 local_e0 [4];
  undefined2 local_dc;
  undefined2 local_da;
  int local_d4;
  int local_d0;
  LONG local_cc;
  LONG local_c8;
  int local_c4;
  int local_c0;
  int local_bc;
  int local_b8;
  float local_b4;
  int local_b0;
  float local_ac;
  float local_a8;
  float local_a4;
  undefined4 local_a0;
  undefined4 local_9c;
  int local_98;
  undefined4 local_94;
  undefined4 local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined4 local_80;
  int local_7c;
  int local_78;
  int local_74;
  undefined1 local_70 [4];
  undefined2 local_6c;
  undefined2 local_6a;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  int local_48;
  int local_44;
  int local_40;
  int local_3c;
  int local_38;
  uint local_34;
  int local_30;
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f8ff;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  uVar7 = (**(code **)(*DAT_0092e654 + 8))();
  FUN_0049a73a(uVar7);
  iVar8 = FUN_00476006(uVar7);
  if (iVar8 != 1) {
    local_3c = 0;
    if ((param_1[0xa3a0] != 0) && (param_1[0xa3a0] + 1000U < uVar7)) {
      iVar8 = FUN_0055af20(&DAT_00a3c7f8);
      if (iVar8 == 1) {
        param_1[0xa3a0] = 0;
      }
      else {
        param_1[0xa3a0] = uVar7;
      }
    }
    FUN_0047ef1d(uVar7);
    FUN_004770ad(uVar7);
    if ((DAT_005b892c == 2) && (param_1[0x9ec5] != 0)) {
      (**(code **)(*(int *)param_1[0x9ec5] + 0x60))(0);
      local_64 = (**(code **)(*(int *)param_1[10] + 0x48))(0x125);
      local_60 = (**(code **)(*(int *)param_1[10] + 0x48))(0x126);
      local_5c = (**(code **)(*(int *)param_1[10] + 0x48))(0x127);
      local_58 = (**(code **)(*(int *)param_1[10] + 0x48))(0x13b);
      local_54 = (**(code **)(*(int *)param_1[10] + 0x48))(0x139);
      local_50 = (**(code **)(*(int *)param_1[10] + 0x48))(0x13a);
      local_4c = (**(code **)(*(int *)param_1[10] + 0x48))(0x130);
      for (local_48 = 0; local_48 < 7; local_48 = local_48 + 1) {
        iVar8 = FUN_00493cd0();
        if (iVar8 != 0) {
          *(float *)(param_1[0x9ec5] + 0x50) = (float)local_48 * _DAT_005a40fc + _DAT_005a40f0;
          (**(code **)(*(int *)param_1[0x9ec5] + 0x60))(1);
        }
      }
    }
    if ((param_1[0x802b] == 0) && (param_1[0x802a] + 3000U < uVar7)) {
      param_1[0x802b] = 1;
    }
    if ((param_1[0xa396] == 1) && (param_1[0xa397] + 500U < uVar7)) {
      param_1[0xa396] = 0;
      param_1[0xa397] = 0;
    }
    FUN_0049362c();
    if (((param_1[0x9d15] != 0) && (180000 < uVar7 - param_1[0x9d15])) &&
       (*(char *)(param_1[0x13] + 0x245) == '\x01')) {
      _memset(local_70,0,0xc);
      local_6a = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
      local_6c = 0x289;
      FUN_0055f2dd(local_70,0xc);
      param_1[0x9d15] = 0;
    }
    if (((5000 < uVar7 - param_1[0xa183]) && (param_1[0xa17d] != 0)) &&
       (*(int *)(param_1[0xa17d] + 0x28) != 0)) {
      (**(code **)(*(int *)param_1[0xa17d] + 0x60))(0);
    }
    local_14 = (**(code **)(*(int *)param_1[0x13] + 0x50))();
    if (DAT_005b892c == 2) {
      local_34 = 180000;
    }
    else {
      local_34 = 30000;
    }
    if (local_34 < uVar7 - param_1[0xa265]) {
      param_1[0xa266] = param_1[0xa266] + 1;
      uVar9 = param_1[0xa266] & 0x80000003;
      if ((int)uVar9 < 0) {
        uVar9 = (uVar9 - 1 | 0xfffffffc) + 1;
      }
      param_1[0xa266] = uVar9;
      param_1[0xa265] = uVar7;
      if (DAT_005b892c == 2) {
        local_74 = param_1[0x9eba];
        for (local_78 = 3; -1 < local_78; local_78 = local_78 + -1) {
          if ((local_74 != 0) &&
             (sVar10 = _strlen((char *)(param_1 + local_78 * 0x18 + 0xa205)), sVar10 != 0)) {
            pvVar11 = operator_new(0xe50);
            local_8 = 0;
            if (pvVar11 == (void *)0x0) {
              local_7a4 = 0;
            }
            else {
              local_7a4 = FUN_00407203(param_1 + local_78 * 0x18 + 0xa205,0xfffff6a6,0,0,0x43960000,
                                       0x41800000,0,0x77777777,1,0);
            }
            local_8 = 0xffffffff;
            local_7c = local_7a4;
            if (local_7a4 != 0) {
              FUN_00408d33(local_7a4);
            }
          }
        }
      }
      else if (param_1[0xa203] != 0) {
        (**(code **)(*(int *)param_1[0xa203] + 0x80))(param_1 + param_1[0xa266] * 0x18 + 0xa205,0);
      }
    }
    local_18 = param_1[0x9f1f];
    if ((local_18 != 0) && (iVar8 = FUN_0040c0f0(), iVar8 == 1)) {
      local_8c = (**(code **)(*(int *)param_1[10] + 0x48))(0x431);
      local_94 = (**(code **)(*(int *)param_1[10] + 0x48))(0x434);
      local_88 = (**(code **)(*(int *)param_1[10] + 0x48))(0x437);
      local_9c = (**(code **)(*(int *)param_1[10] + 0x48))(0x440);
      local_84 = (**(code **)(*(int *)param_1[10] + 0x48))(0x473);
      local_a0 = (**(code **)(*(int *)param_1[10] + 0x48))(0x476);
      local_90 = (**(code **)(*(int *)param_1[10] + 0x48))(0x479);
      local_80 = (**(code **)(*(int *)param_1[10] + 0x48))(0x482);
      local_98 = 0;
      iVar8 = FUN_00493cd0();
      if (iVar8 == 1) {
        local_98 = 0x431;
      }
      iVar8 = FUN_00493cd0();
      if (iVar8 == 1) {
        local_98 = 0x434;
      }
      iVar8 = FUN_00493cd0();
      if (iVar8 == 1) {
        local_98 = 0x437;
      }
      iVar8 = FUN_00493cd0();
      if (iVar8 == 1) {
        local_98 = 0x440;
      }
      iVar8 = FUN_00493cd0();
      if (iVar8 == 1) {
        local_98 = 0x473;
      }
      iVar8 = FUN_00493cd0();
      if (iVar8 == 1) {
        local_98 = 0x476;
      }
      iVar8 = FUN_00493cd0();
      if (iVar8 == 1) {
        local_98 = 0x479;
      }
      iVar8 = FUN_00493cd0();
      if (iVar8 == 1) {
        local_98 = 0x482;
      }
      if ((local_98 == 0) || (*(int *)(DAT_005d0504 + 0x1c) == 0)) {
        param_1[0x9dae] = 0;
        param_1[0x9dad] = 0;
      }
      else if (param_1[0x9dae] == 0) {
        if (param_1[0x9dad] == 0) {
          param_1[0x9dad] = uVar7;
        }
        else if (param_1[0x9dad] + 0x5dcU < uVar7) {
          param_1[0x9dad] = uVar7;
          param_1[0x9dae] = 1;
        }
      }
      else if (param_1[0x9dad] + 100U < uVar7) {
        (**(code **)(*param_1 + 0x58))(local_98,0);
        param_1[0x9dad] = uVar7;
      }
    }
    if (((1000 < uVar7 - _DAT_0092fce0) && (DAT_005ccf98 != 0)) &&
       (iVar8 = FUN_00429a6d(6), iVar8 != 0)) {
      FUN_00429a6d(6);
      iVar8 = FUN_0042afd0();
      if (iVar8 != 0) {
        FUN_00429a6d(6);
        FUN_0042aef6();
      }
    }
    if ((param_1[0x15] == 1) && (param_1[0x21] != 0)) {
      FUN_00403df2(s_Critical_Data_Error_In_Client_005b86b4,0);
      (**(code **)(*(int *)param_1[0x21] + 0x88))(1,0);
    }
    else {
      if (((param_1[0x21] != 0) &&
          ((*(int *)(param_1[0x23] + 0x1ec) == 0xb && (iVar8 = FUN_0040c0f0(), iVar8 == 1)))) &&
         (0 < *(short *)(DAT_013b71e8 + 0x738))) {
        (**(code **)(*(int *)param_1[0x23] + 0x60))(0);
      }
      if (param_1[0x9f65] != 0) {
        fVar2 = *(float *)(param_1[0x13] + 0x34) - _DAT_005a43dc;
        iVar8 = (**(code **)(*(int *)param_1[0x9f65] + 0x84))();
        *(float *)(iVar8 + 0x18) = fVar2;
      }
      if ((param_1[0x9f66] != 0) && (*(int *)(param_1[0x9f66] + 0x28) != 0)) {
        local_a4 = 0.0;
        local_a8 = 0.0;
        local_ac = 0.0;
        local_b4 = 0.0;
        local_b0 = *(int *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x54);
        if (local_b0 != 0) {
          local_b8 = 400;
          if (_DAT_005bde1c < _DAT_005a3660) {
            local_b8 = 0x9b;
          }
          local_c0 = 0x3c;
          local_c4 = 10;
          for (local_bc = 0; local_bc < 0x100; local_bc = local_bc + 1) {
            local_a4 = *(float *)(local_b0 + 0x28) -
                       (float)*(int *)(&DAT_005ccfb8 + local_bc * 0x34);
            local_a8 = *(float *)(local_b0 + 0x2c) -
                       (float)*(int *)(&DAT_005ccfbc + local_bc * 0x34);
            iVar8 = __ftol();
            local_ac = (float)((float10)iVar8 * (float10)_DAT_005a3aa4 + extraout_ST0);
            iVar8 = __ftol();
            local_b4 = (float)((float10)iVar8 * (float10)_DAT_005a3aa4 + extraout_ST0_00);
            if ((((local_ac < _DAT_005a3534) || ((float)local_b8 < local_ac)) ||
                (local_b4 < _DAT_005a3534)) || ((float)local_b8 < local_b4)) {
              (**(code **)(*(int *)param_1[local_bc + 0xa075] + 0x60))(0);
            }
            else {
              (**(code **)(*(int *)param_1[local_bc + 0xa075] + 0x60))(1);
            }
            if ((((float)*(int *)(&DAT_005ccfc0 + local_bc * 0x34) + local_ac < _DAT_005a3534) ||
                ((float)(local_b8 - local_c0) <
                 (float)*(int *)(&DAT_005ccfc0 + local_bc * 0x34) + local_ac)) ||
               (((float)*(int *)(&DAT_005ccfc4 + local_bc * 0x34) + local_b4 < _DAT_005a3534 ||
                ((float)(local_b8 - local_c4) <
                 (float)*(int *)(&DAT_005ccfc4 + local_bc * 0x34) + local_b4)))) {
              (**(code **)(*(int *)param_1[local_bc + 0x9f75] + 0x60))(0);
            }
            else if (_DAT_005a3660 <= _DAT_005bde1c) {
              (**(code **)(*(int *)param_1[local_bc + 0x9f75] + 0x60))(1);
            }
            else {
              (**(code **)(*(int *)param_1[local_bc + 0x9f75] + 0x60))(0);
            }
            (**(code **)(*(int *)param_1[local_bc + 0xa075] + 0x70))(local_ac,local_b4);
            if (_DAT_005a3660 <= _DAT_005bde1c) {
              (**(code **)(*(int *)param_1[local_bc + 0x9f75] + 0x70))
                        ((float)*(int *)(&DAT_005ccfc0 + local_bc * 0x34) + local_ac,
                         (float)*(int *)(&DAT_005ccfc4 + local_bc * 0x34) + local_b4);
            }
          }
        }
      }
      if (((param_1[0x9ba1] != 0) && (param_1[0x9ba2] != 0)) && (param_1[0x9ba3] != 0)) {
        if (*(int *)(param_1[0x9ba1] + 0x24) == 1) {
          *(float *)(param_1[0x9ba1] + 0x8c) =
               ((float)(uVar7 % 1000) * _DAT_005a4148) / _DAT_005a43cc;
          *(undefined4 *)(param_1[0x9ba2] + 0x8c) = *(undefined4 *)(param_1[0x9ba1] + 0x8c);
          fVar15 = (float10)FUN_00423b00(*(float *)(param_1[0x9ba1] + 0x8c) +
                                         *(float *)(param_1[0x9ba1] + 0x8c));
          *(float *)(param_1[0x9ba3] + 0xe0) =
               (float)(fVar15 * (float10)_DAT_005a4214 + (float10)_DAT_005a3660);
          *(undefined4 *)(param_1[0x9ba3] + 0xe4) = *(undefined4 *)(param_1[0x9ba3] + 0xe0);
          *(undefined4 *)(param_1[0x9ba3] + 0xe8) = *(undefined4 *)(param_1[0x9ba3] + 0xe0);
        }
        if ((((*(int *)(param_1[0x13] + 0x740) == 2) || (*(int *)(param_1[0x13] + 0x740) == 3)) &&
            (param_1[0x8028] == 0)) &&
           (((*(int *)(param_1[0x13] + 0x768) == 0 && (*(int *)(param_1[0x13] + 0x76c) == 0)) &&
            (param_1[0x9ba6] == 0)))) {
          *(undefined4 *)(param_1[0x9ba1] + 0x24) = 1;
          *(undefined4 *)(param_1[0x9ba2] + 0x24) = 1;
          *(undefined4 *)(param_1[0x9ba3] + 0x24) = 1;
        }
        else {
          *(undefined4 *)(param_1[0x9ba1] + 0x24) = 0;
          *(undefined4 *)(param_1[0x9ba2] + 0x24) = 0;
          *(undefined4 *)(param_1[0x9ba3] + 0x24) = 0;
        }
      }
      local_40 = 0;
      for (local_20 = 0; local_20 < 7; local_20 = local_20 + 1) {
        local_cc = __ftol();
        local_c8 = __ftol();
        pt.y = local_c8;
        pt.x = local_cc;
        BVar12 = PtInRect((RECT *)(&DAT_005b4cd0 + local_20 * 0x10),pt);
        if (BVar12 == 1) {
          local_40 = 1;
          break;
        }
      }
      if ((param_1[0x9d39] == 0) && (local_40 == 1)) {
        param_1[0x9d39] = 1;
        FUN_00403df2(&DAT_00a35678,2000);
        (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
      }
      else if (local_40 == 0) {
        param_1[0x9d39] = 0;
      }
      iVar8 = __ftol();
      if ((((iVar8 >> 7 < 8) || (iVar8 = __ftol(), 0xc < iVar8 >> 7)) ||
          (iVar8 = __ftol(), iVar8 >> 7 < 0xb)) || (iVar8 = __ftol(), 0xe < iVar8 >> 7)) {
        local_7c4 = 0;
      }
      else {
        local_7c4 = 1;
      }
      local_1c = local_7c4;
      if ((DAT_005ccfa0 == 0) && (local_7c4 == 1)) {
        DAT_005ccfa0 = 3;
      }
      else if ((DAT_005ccfa0 == 3) && (local_7c4 == 0)) {
        DAT_005ccfa0 = 0;
      }
      if (DAT_005ccfa0 == 4) {
        if (900000 < uVar7 - param_1[0x9e44]) {
          (**(code **)(*(int *)param_1[0x9e37] + 0x60))(0);
        }
      }
      else if ((DAT_005ccfa0 != 5) && (6000 < uVar7 - param_1[0x9e44])) {
        (**(code **)(*(int *)param_1[0x9e37] + 0x60))(0);
      }
      if (DAT_005ccfa0 == 5) {
        FUN_00465e9a();
      }
      else {
        iVar8 = FUN_0040c0f0();
        if (iVar8 == 1) {
          FUN_00465e42(0);
        }
      }
      iVar8 = __ftol();
      if ((iVar8 >> 7 == 6) && (iVar8 = __ftol(), iVar8 >> 7 == 0x1c)) {
        bVar3 = true;
      }
      else {
        bVar3 = false;
      }
      if ((bVar3) && (param_1[0x9ec7] != 0)) {
        (**(code **)(*(int *)param_1[0x9ec7] + 0x60))(0);
      }
      iVar8 = __ftol();
      if ((iVar8 >> 7 == 0x1f) && (iVar8 = __ftol(), iVar8 >> 7 == 0x1f)) {
        bVar3 = true;
      }
      else {
        bVar3 = false;
      }
      if ((!bVar3) && (*(int *)(param_1[0x9ef9] + 0x28) == 1)) {
        FUN_0044a745(0,0);
      }
      if (DAT_005d03f8 == 0) {
        local_e4 = DAT_013b71e8 + 0x6ec;
        if (local_14 == 1) {
          local_f0 = __ftol();
          local_ec = __ftol();
          local_e8 = 0;
          pt_00.y = local_ec;
          pt_00.x = local_f0;
          BVar12 = PtInRect((RECT *)&DAT_005b4d40,pt_00);
          if (BVar12 == 1) {
            local_e8 = 8;
          }
          else {
            uVar13 = __ftol();
            uVar13 = __ftol(uVar13);
            local_f4 = FUN_00552f42(uVar13);
            if (local_f4 == 2) {
              local_f4 = 0;
            }
            local_e8 = local_f4 * 2 + 1;
          }
          if (DAT_005b2980 != local_e8) {
            param_1[0x9b93] = uVar7;
            if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
              if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                  FUN_00493cf0(1);
                }
                *(undefined4 *)(DAT_013b71e0 + 0xec) = 0;
              }
              pvVar11 = operator_new(0x18);
              local_8 = 2;
              if (pvVar11 == (void *)0x0) {
                local_7dc = 0;
              }
              else {
                local_7dc = FUN_004287c2(1,*(int *)(DAT_013b71e0 + 0x108) * 0x1e + -3000);
              }
              local_8 = 0xffffffff;
              *(undefined4 *)(DAT_013b71e0 + 0xec) = local_7dc;
              FUN_004294a0(0,**(undefined4 **)(DAT_013b71e0 + 0xec));
              FUN_00428ba0(local_e8);
            }
            DAT_005b2980 = local_e8;
            _memset(local_104,0,0x10);
            local_fe = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
            local_100 = 0x291;
            uVar13 = __ftol();
            uVar13 = __ftol(uVar13);
            local_f8 = FUN_00552f42(uVar13);
            if (local_f8 < 4) {
              FUN_0055f2dd(local_104,0x10);
              uVar6 = __ftol();
              *(undefined2 *)(DAT_013b71e8 + 4 + *(char *)(DAT_013b71e8 + 0xef4) * 2) = uVar6;
              uVar6 = __ftol();
              *(undefined2 *)(DAT_013b71e8 + 0xc + *(char *)(DAT_013b71e8 + 0xef4) * 2) = uVar6;
            }
          }
        }
        else {
          iVar8 = __ftol();
          if (iVar8 >> 7 < 0x1a) {
            iVar8 = __ftol();
            if ((((iVar8 < 0x681) || (iVar8 = __ftol(), 0x6ff < iVar8)) ||
                (iVar8 = __ftol(), iVar8 < 0x601)) || (iVar8 = __ftol(), 0x77f < iVar8)) {
              if (DAT_005ccfa0 == 3) {
                local_120 = 9;
                if (DAT_005b2980 != 9) {
                  param_1[0x9b94] = uVar7;
                  if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                    if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                      if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                        FUN_00493cf0(1);
                      }
                      *(undefined4 *)(DAT_013b71e0 + 0xec) = 0;
                    }
                    pvVar11 = operator_new(0x18);
                    local_8 = 8;
                    if (pvVar11 == (void *)0x0) {
                      local_81c = 0;
                    }
                    else {
                      local_81c = FUN_004287c2(1,*(int *)(DAT_013b71e0 + 0x108) * 0x1e + -3000);
                    }
                    local_8 = 0xffffffff;
                    *(undefined4 *)(DAT_013b71e0 + 0xec) = local_81c;
                    FUN_004294a0(0,**(undefined4 **)(DAT_013b71e0 + 0xec));
                    FUN_00428ba0(local_120);
                  }
                  DAT_005b2980 = local_120;
                }
                if ((param_1[0x9ece] != 0) && (iVar8 = FUN_0040c0f0(), iVar8 == 1)) {
                  FUN_0044ae38(0,0);
                }
                if (((*(char *)(local_e4 + 0x14) != '\x03') ||
                    ((*(uint *)(local_e4 + 0x2dc) & 0x2000) == 0)) &&
                   ((param_1[0xa176] != 0 && (iVar8 = FUN_0040c0f0(), iVar8 == 1)))) {
                  FUN_004484f3(0);
                }
              }
              else {
                iVar8 = __ftol();
                if ((((iVar8 >> 7 < 0x1b) || (iVar8 = __ftol(), 0x1e < iVar8 >> 7)) ||
                    (iVar8 = __ftol(), iVar8 >> 7 < 0x15)) || (iVar8 = __ftol(), 0x18 < iVar8 >> 7))
                {
                  bVar3 = false;
                }
                else {
                  bVar3 = true;
                }
                if (bVar3) {
                  local_124 = 9;
                  if (DAT_005b2980 != 9) {
                    param_1[0x9b94] = uVar7;
                    if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                      if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                        if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                          FUN_00493cf0(1);
                        }
                        *(undefined4 *)(DAT_013b71e0 + 0xec) = 0;
                      }
                      pvVar11 = operator_new(0x18);
                      local_8 = 9;
                      if (pvVar11 == (void *)0x0) {
                        local_828 = 0;
                      }
                      else {
                        local_828 = FUN_004287c2(1,*(int *)(DAT_013b71e0 + 0x108) * 0x1e + -3000);
                      }
                      local_8 = 0xffffffff;
                      *(undefined4 *)(DAT_013b71e0 + 0xec) = local_828;
                      FUN_004294a0(0,**(undefined4 **)(DAT_013b71e0 + 0xec));
                      FUN_00428ba0(local_124);
                    }
                    DAT_005b2980 = local_124;
                  }
                  if ((param_1[0x9ece] != 0) && (iVar8 = FUN_0040c0f0(), iVar8 == 1)) {
                    FUN_0044ae38(0,0);
                  }
                  if (((*(char *)(local_e4 + 0x14) != '\x03') ||
                      ((*(uint *)(local_e4 + 0x2dc) & 0x2000) == 0)) &&
                     ((param_1[0xa176] != 0 && (iVar8 = FUN_0040c0f0(), iVar8 == 1)))) {
                    FUN_004484f3(0);
                  }
                }
                else {
                  local_134 = __ftol();
                  local_130 = __ftol();
                  local_12c = 0;
                  while ((local_12c < 2 &&
                         (pt_01.y = local_130, pt_01.x = local_134,
                         BVar12 = PtInRect((RECT *)(&DAT_005b4d80 + local_12c * 0x10),pt_01),
                         BVar12 != 1))) {
                    local_12c = local_12c + 1;
                  }
                  if (local_12c == 2) {
                    local_12c = 1;
                  }
                  local_128 = local_12c * 2 + 2;
                  if ((DAT_005ccfa0 == 4) && (*(int *)(param_1[0x9e37] + 0x28) == 1)) {
                    local_128 = 5;
                  }
                  if (DAT_005b2980 != local_128) {
                    param_1[0x9b94] = uVar7;
                    if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                      if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                        if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                          FUN_00493cf0(1);
                        }
                        *(undefined4 *)(DAT_013b71e0 + 0xec) = 0;
                      }
                      pvVar11 = operator_new(0x18);
                      local_8 = 10;
                      if (pvVar11 == (void *)0x0) {
                        local_830 = 0;
                      }
                      else {
                        local_830 = FUN_004287c2(1,*(int *)(DAT_013b71e0 + 0x108) * 0x1e + -3000);
                      }
                      local_8 = 0xffffffff;
                      *(undefined4 *)(DAT_013b71e0 + 0xec) = local_830;
                      FUN_004294a0(0,**(undefined4 **)(DAT_013b71e0 + 0xec));
                      FUN_00428ba0(local_128);
                    }
                    DAT_005b2980 = local_128;
                  }
                  if ((param_1[0x9ece] != 0) && (iVar8 = FUN_0040c0f0(), iVar8 == 1)) {
                    FUN_0044ae38(0,0);
                  }
                  if ((((*(char *)(local_e4 + 0x14) != '\x03') ||
                       ((*(uint *)(local_e4 + 0x2dc) & 0x2000) == 0)) && (param_1[0xa176] != 0)) &&
                     (iVar8 = FUN_0040c0f0(), iVar8 == 1)) {
                    FUN_004484f3(0);
                  }
                }
              }
            }
            else {
              local_11c = 6;
              if (DAT_005b2980 != 6) {
                param_1[0x9b93] = uVar7;
                if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                  if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                    if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                      FUN_00493cf0(1);
                    }
                    *(undefined4 *)(DAT_013b71e0 + 0xec) = 0;
                  }
                  pvVar11 = operator_new(0x18);
                  local_8 = 7;
                  if (pvVar11 == (void *)0x0) {
                    local_814 = 0;
                  }
                  else {
                    local_814 = FUN_004287c2(1,*(int *)(DAT_013b71e0 + 0x108) * 0x1e + -3000);
                  }
                  local_8 = 0xffffffff;
                  *(undefined4 *)(DAT_013b71e0 + 0xec) = local_814;
                  FUN_004294a0(0,**(undefined4 **)(DAT_013b71e0 + 0xec));
                  FUN_00428ba0(local_11c);
                }
                DAT_005b2980 = local_11c;
              }
              if ((param_1[0x9ece] != 0) && (iVar8 = FUN_0040c0f0(), iVar8 == 1)) {
                FUN_0044ae38(0,0);
              }
              if ((((*(char *)(local_e4 + 0x14) != '\x03') ||
                   ((*(uint *)(local_e4 + 0x2dc) & 0x2000) == 0)) && (param_1[0xa176] != 0)) &&
                 (iVar8 = FUN_0040c0f0(), iVar8 == 1)) {
                FUN_004484f3(0);
              }
            }
          }
          else {
            iVar8 = __ftol();
            if (((iVar8 >> 7 < 0x10) && (iVar8 = __ftol(), 8 < iVar8 >> 7)) &&
               (iVar8 = __ftol(), 0x19 < iVar8 >> 7)) {
              bVar3 = true;
            }
            else {
              bVar3 = false;
            }
            if (bVar3) {
              local_108 = 7;
              if (DAT_005b2980 != 7) {
                param_1[0x9b94] = uVar7;
                if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                  if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                    if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                      FUN_00493cf0(1);
                    }
                    *(undefined4 *)(DAT_013b71e0 + 0xec) = 0;
                  }
                  pvVar11 = operator_new(0x18);
                  local_8 = 3;
                  if (pvVar11 == (void *)0x0) {
                    local_7e8 = 0;
                  }
                  else {
                    local_7e8 = FUN_004287c2(1,*(int *)(DAT_013b71e0 + 0x108) * 0x1e + -3000);
                  }
                  local_8 = 0xffffffff;
                  *(undefined4 *)(DAT_013b71e0 + 0xec) = local_7e8;
                  FUN_004294a0(0,**(undefined4 **)(DAT_013b71e0 + 0xec));
                  FUN_00428ba0(local_108);
                }
                DAT_005b2980 = local_108;
              }
            }
            else {
              iVar8 = __ftol();
              if ((iVar8 >> 7 == 0x12) && (iVar8 = __ftol(), iVar8 >> 7 == 0x1e)) {
                bVar3 = true;
              }
              else {
                bVar3 = false;
              }
              if (bVar3) {
                local_10c = 0xc;
                if (DAT_005b2980 != 0xc) {
                  param_1[0x9b94] = uVar7;
                  if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                    if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                      if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                        FUN_00493cf0(1);
                      }
                      *(undefined4 *)(DAT_013b71e0 + 0xec) = 0;
                    }
                    pvVar11 = operator_new(0x18);
                    local_8 = 4;
                    if (pvVar11 == (void *)0x0) {
                      local_7f4 = 0;
                    }
                    else {
                      local_7f4 = FUN_004287c2(1,*(int *)(DAT_013b71e0 + 0x108) * 0x1e + -3000);
                    }
                    local_8 = 0xffffffff;
                    *(undefined4 *)(DAT_013b71e0 + 0xec) = local_7f4;
                    FUN_004294a0(0,**(undefined4 **)(DAT_013b71e0 + 0xec));
                    FUN_00428ba0(local_10c);
                  }
                  DAT_005b2980 = local_10c;
                }
                if ((param_1[0x13] == 0) || (*(float *)(param_1[0x13] + 0x2c) < _DAT_005a43c8)) {
                  FUN_00481bd5(0);
                }
                else {
                  FUN_00481bd5(param_1[0x9daf] == 0);
                }
                FUN_0047e49d(uVar7);
              }
              else {
                iVar8 = __ftol();
                if (((iVar8 >> 7 < 0x11) || (iVar8 = __ftol(), 0x13 < iVar8 >> 7)) ||
                   (iVar8 = __ftol(), iVar8 >> 7 < 0x1e)) {
                  bVar3 = false;
                }
                else {
                  bVar3 = true;
                }
                if (bVar3) {
                  local_110 = 0xb;
                  if (DAT_005b2980 != 0xb) {
                    param_1[0x9b94] = uVar7;
                    if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                      if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                        if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                          FUN_00493cf0(1);
                        }
                        *(undefined4 *)(DAT_013b71e0 + 0xec) = 0;
                      }
                      pvVar11 = operator_new(0x18);
                      local_8 = 5;
                      if (pvVar11 == (void *)0x0) {
                        local_800 = 0;
                      }
                      else {
                        local_800 = FUN_004287c2(1,*(int *)(DAT_013b71e0 + 0x108) * 0x1e + -3000);
                      }
                      local_8 = 0xffffffff;
                      *(undefined4 *)(DAT_013b71e0 + 0xec) = local_800;
                      FUN_004294a0(0,**(undefined4 **)(DAT_013b71e0 + 0xec));
                      FUN_00428ba0(local_110);
                    }
                    DAT_005b2980 = local_110;
                  }
                }
                else {
                  local_114 = 5;
                  iVar8 = __ftol();
                  if ((iVar8 >> 7 == 0x1f) && (iVar8 = __ftol(), iVar8 >> 7 == 0x1f)) {
                    bVar3 = true;
                  }
                  else {
                    bVar3 = false;
                  }
                  if (bVar3) {
                    local_118 = 6;
                  }
                  else if (*(int *)(param_1[0x9ef9] + 0x28) == 1) {
                    FUN_0044a745(0,0);
                  }
                  if (DAT_005b2980 != local_114) {
                    param_1[0x9b94] = uVar7;
                    if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                      if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                        if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
                          FUN_00493cf0(1);
                        }
                        *(undefined4 *)(DAT_013b71e0 + 0xec) = 0;
                      }
                      pvVar11 = operator_new(0x18);
                      local_8 = 6;
                      if (pvVar11 == (void *)0x0) {
                        local_80c = 0;
                      }
                      else {
                        local_80c = FUN_004287c2(1,*(int *)(DAT_013b71e0 + 0x108) * 0x1e + -3000);
                      }
                      local_8 = 0xffffffff;
                      *(undefined4 *)(DAT_013b71e0 + 0xec) = local_80c;
                      FUN_004294a0(0,**(undefined4 **)(DAT_013b71e0 + 0xec));
                      FUN_00428ba0(local_114);
                    }
                    DAT_005b2980 = local_114;
                  }
                }
              }
            }
            if ((param_1[0x9ece] != 0) && (iVar8 = FUN_0040c0f0(), iVar8 == 1)) {
              FUN_0044ae38(0,0);
            }
            if ((((*(char *)(local_e4 + 0x14) != '\x03') ||
                 ((*(uint *)(local_e4 + 0x2dc) & 0x2000) == 0)) && (param_1[0xa176] != 0)) &&
               (iVar8 = FUN_0040c0f0(), iVar8 == 1)) {
              FUN_004484f3(0);
            }
          }
        }
      }
      else {
        if (local_14 == 1) {
          uVar13 = __ftol();
          uVar13 = __ftol(uVar13);
          local_d0 = FUN_00552f42(uVar13);
          if (param_1[0x9d6e] != local_d0) {
            _memset(local_e0,0,0x10);
            local_da = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
            local_dc = 0x291;
            uVar13 = __ftol();
            uVar13 = __ftol(uVar13);
            local_d4 = FUN_00552f42(uVar13);
            param_1[0x9d6e] = local_d0;
            if (local_d4 < 4) {
              FUN_0055f2dd(local_e0,0x10);
              uVar6 = __ftol();
              *(undefined2 *)(DAT_013b71e8 + 4 + *(char *)(DAT_013b71e8 + 0xef4) * 2) = uVar6;
              uVar6 = __ftol();
              *(undefined2 *)(DAT_013b71e8 + 0xc + *(char *)(DAT_013b71e8 + 0xef4) * 2) = uVar6;
            }
          }
        }
        else {
          param_1[0x9d6e] = -1;
        }
        if ((DAT_005b2984 != 10) && (*(int *)(DAT_013b71e0 + 0xec) != 0)) {
          if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
            if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
              FUN_00493cf0(1);
            }
            *(undefined4 *)(DAT_013b71e0 + 0xec) = 0;
          }
          pvVar11 = operator_new(0x18);
          local_8 = 1;
          if (pvVar11 == (void *)0x0) {
            local_7d4 = 0;
          }
          else {
            local_7d4 = FUN_004287c2(1,*(int *)(DAT_013b71e0 + 0x108) * 0x1e + -3000);
          }
          local_8 = 0xffffffff;
          *(undefined4 *)(DAT_013b71e0 + 0xec) = local_7d4;
          FUN_004294a0(0,**(undefined4 **)(DAT_013b71e0 + 0xec));
          FUN_00428c6e(10);
        }
      }
      if ((param_1[0x9ec7] != 0) && (iVar8 = FUN_0040c0f0(), iVar8 == 1)) {
        local_1b4 = 0;
        puVar14 = &local_1b3;
        for (iVar8 = 0xf; iVar8 != 0; iVar8 = iVar8 + -1) {
          *puVar14 = 0;
          puVar14 = puVar14 + 1;
        }
        *(undefined2 *)puVar14 = 0;
        *(undefined1 *)((int)puVar14 + 2) = 0;
        local_1b8 = *(int *)(DAT_013b71e8 + 0x1af4c);
        if (DAT_005b892c == 2) {
          uVar13 = __ftol();
          uVar13 = __ftol(uVar13);
          FUN_0058f078(local_174,s_X___4d_Y___4d_005b86d4,uVar13);
        }
        else {
          uVar13 = __ftol();
          uVar13 = __ftol(uVar13);
          FUN_0058f078(local_174,s__s__d__4d__4d_005b86e4,&DAT_013b714c + local_1b8 * 9,
                       *(undefined4 *)(DAT_013b71e8 + 0x1af50),uVar13);
        }
        (**(code **)(*(int *)param_1[0x9ec7] + 0x80))(local_174,0);
        if ((DAT_005b892c == 2) &&
           (FUN_0058f078(&local_1b4,s__s__d_005b86f4,&DAT_013b714c + local_1b8 * 9,
                         *(undefined4 *)(DAT_013b71e8 + 0x1af50)), param_1[0x9f74] != 0)) {
          (**(code **)(*(int *)param_1[0x9f74] + 0x80))(&local_1b4,0);
        }
      }
      for (local_3c = 0; local_3c < 0x10; local_3c = local_3c + 1) {
        if (*(char *)(param_1[0x13] + 0x6a4 + local_3c * 8) == '\0') {
          if ((DAT_005b892c != 2) && (local_3c < 4)) {
            (**(code **)(*(int *)param_1[local_3c + 0xa18e] + 0x80))(&DAT_005d04a4,0);
          }
          if (param_1[local_3c + 0xa1a4] != 0) {
            (**(code **)(*(int *)param_1[local_3c + 0xa194] + 0x80))(&DAT_005d04a8,0);
            (**(code **)(*(int *)param_1[local_3c + 0xa194] + 0x60))(0);
            (**(code **)(*(int *)param_1[local_3c + 0xa1a4] + 0x60))(0);
          }
          if (DAT_005b892c == 2) {
            (**(code **)(*(int *)param_1[local_3c + 0xa1b4] + 0x60))(0);
            param_1[local_3c + 0xa3a1] = 0;
          }
        }
        else {
          local_23c = (*(int *)(param_1[0x13] + 0x6a8 + local_3c * 8) * 8 + -4) -
                      (uVar7 - param_1[local_3c + 0x9d20]) / 1000;
          if (local_23c < 1) {
            if (param_1[local_3c + 0xa1a4] != 0) {
              if (local_3c == 0) {
                (**(code **)(*(int *)param_1[0xa194] + 0x84))(0xff000000);
              }
              if (DAT_005b892c == 2) {
                (**(code **)(*(int *)param_1[local_3c + 0xa194] + 0x80))(s_0_005b86fc,0);
              }
              else {
                if (local_3c < 4) {
                  (**(code **)(*(int *)param_1[local_3c + 0xa18e] + 0x80))(&DAT_005b8704,0);
                }
                (**(code **)(*(int *)param_1[local_3c + 0xa194] + 0x80))(&DAT_005b8708,0);
              }
              (**(code **)(*(int *)param_1[local_3c + 0xa194] + 0x60))(1);
              (**(code **)(*(int *)param_1[local_3c + 0xa1a4] + 0x60))(1);
              if (DAT_005b892c == 2) {
                (**(code **)(*(int *)param_1[local_3c + 0xa194] + 0x60))(0);
                (**(code **)(*(int *)param_1[local_3c + 0xa1a4] + 0x60))(0);
              }
            }
          }
          else {
            FUN_00477513(local_238,local_23c,*(undefined4 *)(param_1[0x13] + 0x6a8 + local_3c * 8),
                         local_3c);
            if ((DAT_005b892c != 2) && (local_3c < 4)) {
              (**(code **)(*(int *)param_1[local_3c + 0xa18e] + 0x80))(local_238,0);
            }
            if (param_1[local_3c + 0xa1a4] != 0) {
              if (local_3c == 0) {
                if (local_23c < 0x1ccf) {
                  for (local_240 = 0; local_240 < 7; local_240 = local_240 + 1) {
                    if (local_23c < *(short *)((int)&PTR_DAT_005b4d6c + local_240 * 2)) {
                      (**(code **)(*(int *)param_1[0xa194] + 0x84))
                                (*(undefined4 *)(&DAT_005b4d50 + local_240 * 4));
                      break;
                    }
                  }
                }
                else {
                  (**(code **)(*(int *)param_1[0xa194] + 0x84))(0xffffffff);
                }
              }
              (**(code **)(*(int *)param_1[local_3c + 0xa194] + 0x80))(local_238,0);
              (**(code **)(*(int *)param_1[local_3c + 0xa194] + 0x60))(1);
              (**(code **)(*(int *)param_1[local_3c + 0xa1a4] + 0x60))(1);
            }
            if (DAT_005b892c == 2) {
              if (param_1[local_3c + 0xa1a4] != 0) {
                (**(code **)(*(int *)param_1[local_3c + 0xa194] + 0x60))(0);
                (**(code **)(*(int *)param_1[local_3c + 0xa1a4] + 0x60))(0);
              }
              if (param_1[0xa1ff] != 0) {
                if (*(byte *)(param_1[0x13] + 0x6a4 + local_3c * 8) < 0x28) {
                  *(undefined4 *)(param_1[local_3c + 0xa1b4] + 0xa4) =
                       *(undefined4 *)
                        (&DAT_005b77c0 + (uint)*(byte *)(param_1[0x13] + 0x6a4 + local_3c * 8) * 4);
                }
                else {
                  *(undefined4 *)(param_1[local_3c + 0xa1b4] + 0xa4) = 0;
                }
                *(undefined4 *)(param_1[local_3c + 0xa1b4] + 0x1ac) = 0x1d;
              }
            }
          }
        }
      }
      if (((0 < *(short *)(DAT_013b71e8 + 0x738)) && (param_1[0x8028] != 0)) &&
         (*(int *)(DAT_005ccec0 + 0x1e8) == 0)) {
        local_24c = 1.0;
        if (param_1[0x8029] != 0) {
          local_24c = -1.0;
        }
        local_248 = local_24c * *(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x58) *
                    *(float *)(param_1[0x13] + 0x220) * _DAT_005a430c +
                    *(float *)(param_1[0x13] + 0x28);
        local_244 = local_24c * *(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x60) *
                    *(float *)(param_1[0x13] + 0x220) * _DAT_005a430c +
                    *(float *)(param_1[0x13] + 0x2c);
        uVar13 = __ftol();
        uVar13 = __ftol(uVar13);
        iVar8 = FUN_004b3962(uVar13);
        if (iVar8 == 0) {
          FUN_0046218b(local_248,local_244,uVar7);
        }
      }
      if (*(char *)(param_1[0x13] + 0x247) == '\0') {
        if (_DAT_005a4104 <= *(float *)(param_1[0x13] + 0x220)) {
          if ((_DAT_005a415c <= *(float *)(param_1[0x13] + 0x230)) ||
             (*(float *)(param_1[0x13] + 0x230) == _DAT_005a3534)) {
            iVar8 = __ftol();
            if (((iVar8 == param_1[0x9d6c]) && (iVar8 = __ftol(), iVar8 == param_1[0x9d6d])) &&
               ((((*(int *)(param_1[0x13] + 0x484) != param_1[0x9d6c] ||
                  (*(int *)(param_1[0x13] + 0x488) != param_1[0x9d6d])) &&
                 (1000 < uVar7 - *(int *)(param_1[0x13] + 0x5ec))) &&
                (*(char *)(param_1[0x13] + 0x245) == '\0')))) {
              FUN_0040bd30(param_1 + 0x9d6c);
              _memset(local_2c0,0,0x34);
              local_2b4 = (short)param_1[0xa1e5];
              local_2b2 = (short)param_1[0xa1e6];
              local_2ac = 0;
              _local_2bc = CONCAT22(*(undefined2 *)(param_1[0x13] + 0x20),0x366);
              local_2b0 = DAT_005bb418;
              local_2a8 = *(ushort *)(param_1[0x13] + 0x484);
              uStack_2a6 = *(ushort *)(param_1[0x13] + 0x488);
              for (local_2c4 = 0; local_2c4 < 0x17; local_2c4 = local_2c4 + 1) {
                auStack_2a4[local_2c4] = 0;
              }
              DAT_005d03fc = 0x366;
              param_1[0xa1e3] = (int)local_2b4;
              param_1[0xa1e4] = (int)local_2b2;
              param_1[0xa1e5] = (uint)local_2a8;
              param_1[0xa1e6] = (uint)uStack_2a6;
              FUN_0055f2dd(local_2c0,0x34);
              uVar13 = (**(code **)(*DAT_0092e654 + 8))();
              *(undefined4 *)(param_1[0x13] + 0x5ec) = uVar13;
            }
            else {
              iVar8 = __ftol();
              if ((param_1[0x9d6c] != iVar8) || (iVar8 = __ftol(), param_1[0x9d6d] != iVar8)) {
                FUN_00520216(param_1[0x9d6c],param_1[0x9d6d],0,(int)*(char *)(param_1[0x13] + 0x247)
                            );
              }
            }
          }
        }
        else if ((_DAT_005a40f4 <= *(float *)(param_1[0x13] + 0x230)) ||
                (*(float *)(param_1[0x13] + 0x230) == _DAT_005a3534)) {
          local_250 = __ftol();
          local_254 = __ftol();
          if (((local_250 == param_1[0x9d6c]) && (local_254 == param_1[0x9d6d])) &&
             ((((*(int *)(param_1[0x13] + 0x484) != param_1[0x9d6c] ||
                (*(int *)(param_1[0x13] + 0x488) != param_1[0x9d6d])) &&
               (1000 < uVar7 - *(int *)(param_1[0x13] + 0x5ec))) &&
              (*(char *)(param_1[0x13] + 0x245) == '\0')))) {
            FUN_0040bd30(param_1 + 0x9d6c);
            _memset(local_288,0,0x34);
            local_27c = *(short *)(param_1[0x13] + 0x484);
            local_27a = *(short *)(param_1[0x13] + 0x488);
            local_274 = 0;
            _local_284 = CONCAT22(*(undefined2 *)(param_1[0x13] + 0x20),0x366);
            local_278 = DAT_005bb418;
            local_270 = *(ushort *)(param_1[0x13] + 0x484);
            uStack_26e = *(ushort *)(param_1[0x13] + 0x488);
            for (local_28c = 0; local_28c < 0x17; local_28c = local_28c + 1) {
              auStack_26c[local_28c] = 0;
            }
            DAT_005d03fc = 0x366;
            param_1[0xa1e3] = (int)local_27c;
            param_1[0xa1e4] = (int)local_27a;
            param_1[0xa1e5] = (uint)local_270;
            param_1[0xa1e6] = (uint)uStack_26e;
            FUN_0055f2dd(local_288,0x34);
            uVar13 = (**(code **)(*DAT_0092e654 + 8))();
            *(undefined4 *)(param_1[0x13] + 0x5ec) = uVar13;
          }
          else {
            iVar8 = __ftol();
            if ((param_1[0x9d6c] != iVar8) || (iVar8 = __ftol(), param_1[0x9d6d] != iVar8)) {
              FUN_00520216(param_1[0x9d6c],param_1[0x9d6d],0,(int)*(char *)(param_1[0x13] + 0x247));
            }
          }
        }
      }
      if (param_1[0x9ba6] != 0) {
        FUN_0051a56c(param_1[0x9ba6]);
      }
      if (param_1[0x9bb4] != 0) {
        if ((float)param_1[0x9bb5] <= _DAT_005a3534) {
          param_1[0x9bb5] = 0x3f800000;
        }
        local_2cc = (float)(uVar7 - param_1[0x9bb4]) / (float)param_1[0x9bb5];
        local_2c8 = param_1[0x9ec8];
        if (_DAT_005a4214 <= local_2cc) {
          if (_DAT_005a40f4 <= local_2cc) {
            if (_DAT_005a3660 <= local_2cc) {
              *(undefined4 *)(local_2c8 + 0x94) = 0;
              param_1[0x9bb4] = 0;
            }
            else {
              fVar15 = (float10)FUN_00423b00(((local_2cc - _DAT_005a40f4) * _DAT_005a4104 +
                                             _DAT_005a34a0) * _DAT_005a4148);
              local_2dc = (float)fVar15;
              local_2d8 = __ftol();
              *(uint *)(local_2c8 + 0x94) =
                   local_2d8 << 0x18 | local_2d8 << 0x10 | local_2d8 << 8 | local_2d8;
            }
          }
          else {
            *(undefined4 *)(local_2c8 + 0x94) = 0xffffffff;
          }
        }
        else {
          fVar15 = (float10)FUN_00423b00(local_2cc * _DAT_005a4104 * _DAT_005a4148);
          local_2d4 = (float)fVar15;
          local_2d0 = __ftol();
          *(uint *)(local_2c8 + 0x94) =
               local_2d0 << 0x18 | local_2d0 << 0x10 | local_2d0 << 8 | local_2d0;
        }
        param_1[0x9ba4] = 0;
      }
      if (((*(int *)(param_1[0x10] + 0xc4) == 0) || (*(int *)(param_1[0x10] + 0xc4) == 3)) ||
         (*(int *)(param_1[0x10] + 0xc4) == 4)) {
        if ((DAT_005ccf98 != 0) && (iVar8 = FUN_00429a6d(0x155), iVar8 != 0)) {
          FUN_00429a6d(0x155);
          iVar8 = FUN_0042afd0();
          if (iVar8 != 0) {
            FUN_00429a6d(0x155);
            FUN_0042aef6();
          }
        }
      }
      else {
        if ((DAT_005ccf98 != 0) && (iVar8 = FUN_00429a6d(0x6b), iVar8 != 0)) {
          FUN_00429a6d(0x6b);
          iVar8 = FUN_0042afd0();
          if (iVar8 != 0) {
            FUN_00429a6d(0x6b);
            FUN_0042aef6();
          }
        }
        iVar8 = __ftol();
        if ((iVar8 >> 7 == 0x12) && (iVar8 = __ftol(), iVar8 >> 7 == 0x1e)) {
          bVar3 = true;
        }
        else {
          bVar3 = false;
        }
        if (bVar3) {
          if ((DAT_005ccf98 != 0) && (iVar8 = FUN_00429a6d(0x155), iVar8 != 0)) {
            FUN_00429a6d(0x155);
            iVar8 = FUN_0042afd0();
            if (iVar8 == 0) {
              uVar16 = 1;
              uVar13 = 0;
              FUN_00429a6d(0x155);
              FUN_0042ad2b(uVar13,uVar16);
            }
          }
        }
        else if ((DAT_005ccf98 != 0) && (iVar8 = FUN_00429a6d(0x155), iVar8 != 0)) {
          FUN_00429a6d(0x155);
          iVar8 = FUN_0042afd0();
          if (iVar8 != 0) {
            FUN_00429a6d(0x155);
            FUN_0042aef6();
          }
        }
      }
      if ((DAT_005d03e4 == 2) || (DAT_005d03e4 == 3)) {
        if (((*(int *)(param_1[0x10] + 0xc4) == 3) || (*(int *)(param_1[0x10] + 0xc4) == 4)) &&
           (*(int *)(param_1[0x9ba8] + 0x28) == 1)) {
          *(undefined4 *)(param_1[0x9ba8] + 0x28) = 0;
          *(undefined4 *)(param_1[0x9ba9] + 0x28) = 0;
          *(undefined4 *)(param_1[0x9ba7] + 0x28) = 1;
        }
        else if ((*(int *)(param_1[0x10] + 0xc4) == 0) && (*(int *)(param_1[0x9ba8] + 0x28) == 0)) {
          *(undefined4 *)(param_1[0x9ba8] + 0x28) = 1;
          if (DAT_005d03e4 == 3) {
            *(undefined4 *)(param_1[0x9ba9] + 0x28) = 1;
          }
          *(undefined4 *)(param_1[0x9ba7] + 0x28) = 0;
        }
      }
      if ((DAT_005d03e4 == 0) &&
         (((*(int *)(param_1[0x10] + 0xc4) == 0 || (*(int *)(param_1[0x10] + 0xc4) == 3)) ||
          (*(int *)(param_1[0x10] + 0xc4) == 4)))) {
        local_2e0 = (int)(((ulonglong)uVar7 / 600000) % 0xc);
        if (param_1[0x9bb2] != 0) {
          local_2e0 = param_1[0x9bb2];
        }
        if (*(int *)(param_1[0x10] + 0xc4) == 4) {
          DAT_005d03e4 = 0;
          local_2e0 = 0xb;
        }
        if (((*(int *)(param_1[0x10] + 0xc4) == 3) || (*(int *)(param_1[0x10] + 0xc4) == 4)) &&
           (*(int *)(param_1[0x9ba8] + 0x28) == 1)) {
          *(undefined4 *)(param_1[0x9ba8] + 0x28) = 0;
          *(undefined4 *)(param_1[0x9ba9] + 0x28) = 0;
          *(undefined4 *)(param_1[0x9ba7] + 0x28) = 1;
        }
        if (local_2e0 < 8) {
          iVar8 = __ftol();
          if ((((iVar8 >> 7 < 0x1b) || (iVar8 = __ftol(), 0x1e < iVar8 >> 7)) ||
              (iVar8 = __ftol(), iVar8 >> 7 < 0x15)) || (iVar8 = __ftol(), 0x18 < iVar8 >> 7)) {
            bVar3 = false;
          }
          else {
            bVar3 = true;
          }
          if (bVar3) {
            if ((*(int *)(param_1[0x11] + 0x164) != 1) && (*(int *)(param_1[0x11] + 0x164) != 0xb))
            {
              FUN_00549bbf(0xb);
              *(undefined4 *)(param_1[0x9ba7] + 0x28) = 0;
              *(undefined4 *)(param_1[0x9ba8] + 0x28) = 1;
              *(undefined4 *)(param_1[0x9ba9] + 0x28) = 0;
            }
          }
          else if ((*(int *)(param_1[0x11] + 0x164) != 0) && (*(int *)(param_1[0x11] + 0x164) != 10)
                  ) {
            FUN_00549bbf(10);
            *(undefined4 *)(param_1[0x9ba7] + 0x28) = 0;
            *(undefined4 *)(param_1[0x9ba8] + 0x28) = 0;
            *(undefined4 *)(param_1[0x9ba9] + 0x28) = 0;
          }
        }
        else if ((local_2e0 < 8) || (9 < local_2e0)) {
          if ((local_2e0 < 10) || (0xb < local_2e0)) {
            if (local_2e0 == 0xc) {
              if (*(int *)(param_1[0x11] + 0x164) != 1) {
                DAT_005d03e4 = 0xc;
                FUN_00549bbf(0xb);
              }
              if (*(int *)(param_1[0x9ba7] + 0x28) == 0) {
                *(undefined4 *)(param_1[0x9ba7] + 0x28) = 1;
              }
              if (*(int *)(param_1[0x9ba8] + 0x28) == 1) {
                *(undefined4 *)(param_1[0x9ba8] + 0x28) = 0;
              }
              if (*(int *)(param_1[0x9ba9] + 0x28) == 1) {
                *(undefined4 *)(param_1[0x9ba9] + 0x28) = 0;
              }
            }
            else if (local_2e0 == 0xd) {
              if (*(int *)(param_1[0x11] + 0x164) != 1) {
                DAT_005d03e4 = 0xd;
                FUN_00549bbf(0xb);
              }
              if (*(int *)(param_1[0x9ba7] + 0x28) == 1) {
                *(undefined4 *)(param_1[0x9ba7] + 0x28) = 0;
              }
              if (*(int *)(param_1[0x9ba8] + 0x28) == 0) {
                *(undefined4 *)(param_1[0x9ba8] + 0x28) = 1;
              }
              if (*(int *)(param_1[0x9ba9] + 0x28) == 1) {
                *(undefined4 *)(param_1[0x9ba9] + 0x28) = 0;
              }
            }
            else if (local_2e0 == 0xe) {
              if (*(int *)(param_1[0x11] + 0x164) != 1) {
                DAT_005d03e4 = 0xe;
                FUN_00549bbf(0xb);
              }
              if (*(int *)(param_1[0x9ba7] + 0x28) == 1) {
                *(undefined4 *)(param_1[0x9ba7] + 0x28) = 0;
              }
              if (*(int *)(param_1[0x9ba8] + 0x28) == 0) {
                *(undefined4 *)(param_1[0x9ba8] + 0x28) = 1;
              }
              if (*(int *)(param_1[0x9ba9] + 0x28) == 0) {
                *(undefined4 *)(param_1[0x9ba9] + 0x28) = 1;
              }
            }
          }
          else if ((*(int *)(param_1[0x11] + 0x164) != 3) &&
                  (*(int *)(param_1[0x11] + 0x164) != 0xd)) {
            FUN_00549bbf(0xd);
            *(undefined4 *)(param_1[0x9ba7] + 0x28) = 0;
            *(undefined4 *)(param_1[0x9ba8] + 0x28) = 0;
            *(undefined4 *)(param_1[0x9ba9] + 0x28) = 0;
          }
        }
        else if ((*(int *)(param_1[0x11] + 0x164) != 2) && (*(int *)(param_1[0x11] + 0x164) != 0xc))
        {
          FUN_00549bbf(0xc);
          *(undefined4 *)(param_1[0x9ba7] + 0x28) = 0;
          *(undefined4 *)(param_1[0x9ba8] + 0x28) = 0;
          *(undefined4 *)(param_1[0x9ba9] + 0x28) = 0;
        }
        if (1000 < uVar7 - param_1[0x9d1a]) {
          if (*(int *)(param_1[0x10] + 0xc4) == 0) {
            local_2e8 = _rand();
            local_2e8 = local_2e8 % 7;
            local_2e4 = 0x5f;
            iVar8 = __ftol();
            if (((iVar8 >> 7 == 0x1d) || (iVar8 = __ftol(), iVar8 >> 7 == 0x1e)) &&
               (iVar8 = __ftol(), iVar8 >> 7 == 0x16)) {
              bVar3 = true;
            }
            else {
              bVar3 = false;
            }
            if (bVar3) {
              if ((DAT_005ccf98 != 0) && (iVar8 = FUN_00429a6d(0x72), iVar8 != 0)) {
                FUN_00429a6d(0x72);
                iVar8 = FUN_0042afd0();
                if (iVar8 == 0) {
                  uVar16 = 1;
                  uVar13 = 0;
                  FUN_00429a6d(0x72);
                  FUN_0042ad2b(uVar13,uVar16);
                }
              }
              local_2e4 = 2;
            }
            else if ((DAT_005ccf98 != 0) && (iVar8 = FUN_00429a6d(0x72), iVar8 != 0)) {
              FUN_00429a6d(0x72);
              iVar8 = FUN_0042afd0();
              if (iVar8 != 0) {
                FUN_00429a6d(0x72);
                FUN_0042aef6();
              }
            }
            iVar8 = __ftol();
            if (((iVar8 >> 7 == 0x1c) && (iVar8 = __ftol(), iVar8 >> 7 == 0x18)) ||
               ((iVar8 = __ftol(), iVar8 >> 7 == 0x13 && (iVar8 = __ftol(), iVar8 >> 7 == 0xc)))) {
              local_2ec = _rand();
              local_2ec = local_2ec % 7;
              if (local_2ec == 0) {
                if ((DAT_005ccf98 != 0) && (iVar8 = FUN_00429a6d(0x16f), iVar8 != 0)) {
                  FUN_00429a6d(0x16f);
                  iVar8 = FUN_0042afd0();
                  if (iVar8 == 0) {
                    uVar16 = 0;
                    uVar13 = 0;
                    FUN_00429a6d(0x16f);
                    FUN_0042ad2b(uVar13,uVar16);
                  }
                }
              }
              else if (local_2ec == 1) {
                if ((DAT_005ccf98 != 0) && (iVar8 = FUN_00429a6d(0x170), iVar8 != 0)) {
                  FUN_00429a6d(0x170);
                  iVar8 = FUN_0042afd0();
                  if (iVar8 == 0) {
                    uVar16 = 0;
                    uVar13 = 0;
                    FUN_00429a6d(0x170);
                    FUN_0042ad2b(uVar13,uVar16);
                  }
                }
              }
              else if (((local_2ec == 2) && (DAT_005ccf98 != 0)) &&
                      (iVar8 = FUN_00429a6d(0x171), iVar8 != 0)) {
                FUN_00429a6d(0x171);
                iVar8 = FUN_0042afd0();
                if (iVar8 == 0) {
                  uVar16 = 0;
                  uVar13 = 0;
                  FUN_00429a6d(0x171);
                  FUN_0042ad2b(uVar13,uVar16);
                }
              }
            }
            if (DAT_0092e660 == 0) {
              pvVar11 = operator_new(0x180);
              local_8 = 0xb;
              if (pvVar11 == (void *)0x0) {
                local_850 = (int *)0x0;
              }
              else {
                local_850 = (int *)FUN_004c8f70(local_2e4,20000,0x3e4ccccd,0x3e4ccccd,0x3e4ccccd,0,1
                                                ,0x50);
              }
              local_8 = 0xffffffff;
              local_2f0 = local_850;
              if (local_850 != (int *)0x0) {
                uVar13 = FUN_00430f50((*(float *)(param_1[0x13] + 0x28) + _DAT_005a365c) -
                                      (float)local_2e8,
                                      (float)local_2e8 * _DAT_005a4158 +
                                      *(float *)(param_1[0x13] + 0x30),
                                      (*(float *)(param_1[0x13] + 0x2c) + _DAT_005a365c) -
                                      (float)local_2e8);
                FUN_004310a0(uVar13);
                local_2f0[0x1e] = (int)((float)local_2f0[0x1e] + _DAT_005a3660);
                FUN_004310a0(local_2f0 + 0x1d);
                local_2f0[0x51] = (int)((float)local_2e8 * _DAT_005a4214 + _DAT_005a3660);
                local_2e8 = _rand();
                local_2e8 = local_2e8 % 7;
                local_2f0[0x4f] = (int)((float)local_2e8 * _DAT_005a34a0 + _DAT_005a3664);
                local_2e8 = _rand();
                local_2e8 = local_2e8 % 7;
                local_2f0[0x50] = (int)((float)local_2e8 * _DAT_005a4214 + _DAT_005a3660);
                local_2f0[0x1b] = 1;
                local_2f0[0x52] = local_2e8 % 3 + 3;
                (**(code **)(*local_2f0 + 0x3c))(0xaaaaaaaa);
                if (param_1[0xb] != 0) {
                  FUN_0054ac09(local_2f0);
                }
              }
            }
          }
          if ((*(int *)(param_1[0x11] + 0x164) == 3) || (*(int *)(param_1[0x11] + 0x164) == 0xd)) {
            if ((DAT_005ccf98 != 0) && (iVar8 = FUN_00429a6d(0x6b), iVar8 != 0)) {
              FUN_00429a6d(0x6b);
              iVar8 = FUN_0042afd0();
              if (iVar8 == 0) {
                uVar16 = 1;
                uVar13 = 0;
                FUN_00429a6d(0x6b);
                FUN_0042ad2b(uVar13,uVar16);
              }
            }
            iVar8 = __ftol();
            if ((((iVar8 >> 7 < 0x1b) || (iVar8 = __ftol(), 0x1e < iVar8 >> 7)) ||
                (iVar8 = __ftol(), iVar8 >> 7 < 0x15)) || (iVar8 = __ftol(), 0x18 < iVar8 >> 7)) {
              bVar3 = false;
            }
            else {
              bVar3 = true;
            }
            if ((!bVar3) && (DAT_0092e660 == 0)) {
              local_2f8 = _rand();
              local_2f8 = local_2f8 % 7;
              pvVar11 = operator_new(0x180);
              local_8 = 0xc;
              if (pvVar11 == (void *)0x0) {
                local_858 = (int *)0x0;
              }
              else {
                local_858 = (int *)FUN_004c8f70(0x38,20000,0x3e4ccccd,0x3e4ccccd,0x3e4ccccd,0,1,0x50
                                               );
              }
              local_8 = 0xffffffff;
              local_2f4 = local_858;
              if (local_858 != (int *)0x0) {
                uVar13 = FUN_00430f50((*(float *)(param_1[0x13] + 0x28) + _DAT_005a365c) -
                                      (float)local_2f8,
                                      (float)local_2f8 * _DAT_005a4158 +
                                      *(float *)(param_1[0x13] + 0x30) + _DAT_005a430c,
                                      (*(float *)(param_1[0x13] + 0x2c) + _DAT_005a365c) -
                                      (float)local_2f8);
                FUN_004310a0(uVar13);
                FUN_004310a0(local_2f4 + 0x1d);
                local_2f4[0x51] = (int)((float)local_2f8 * _DAT_005a4214 + _DAT_005a430c);
                local_2f4[0x4f] = (int)((float)local_2f8 * _DAT_005a34a0 + _DAT_005a4104);
                local_2f4[0x50] = (int)((float)local_2f8 * _DAT_005a439c + _DAT_005a4158);
                local_2f4[0x52] = local_2f8 % 3 + 6;
                local_2f4[0x1b] = 1;
                (**(code **)(*local_2f4 + 0x3c))(0xffffff00);
                if (param_1[0xb] != 0) {
                  FUN_0054ac09(local_2f4);
                }
              }
              pvVar11 = operator_new(0x180);
              local_8 = 0xd;
              if (pvVar11 == (void *)0x0) {
                local_85c = (int *)0x0;
              }
              else {
                local_85c = (int *)FUN_004c8f70(0x3c,20000,0x3d8f5c29,0x3d8f5c29,0x3d8f5c29,0,1,0x50
                                               );
              }
              local_8 = 0xffffffff;
              local_2f4 = local_85c;
              if (local_85c != (int *)0x0) {
                uVar13 = FUN_00430f50((*(float *)(param_1[0x13] + 0x28) + _DAT_005a365c) -
                                      (float)local_2f8,
                                      (float)local_2f8 * _DAT_005a4158 +
                                      *(float *)(param_1[0x13] + 0x30) + _DAT_005a430c,
                                      (*(float *)(param_1[0x13] + 0x2c) + _DAT_005a365c) -
                                      (float)local_2f8);
                FUN_004310a0(uVar13);
                FUN_004310a0(local_2f4 + 0x1d);
                local_2f4[0x51] = (int)((float)local_2f8 * _DAT_005a4214 + _DAT_005a430c);
                local_2f4[0x4f] = (int)((float)local_2f8 * _DAT_005a34a0 + _DAT_005a4104);
                local_2f4[0x50] = (int)((float)local_2f8 * _DAT_005a439c + _DAT_005a4158);
                local_2f4[0x52] = local_2f8 % 3 + 6;
                local_2f4[0x1b] = 1;
                (**(code **)(*local_2f4 + 0x3c))(0xffffff00);
                if (param_1[0xb] != 0) {
                  FUN_0054ac09(local_2f4);
                }
              }
            }
            if (((DAT_005ccfa0 == 3) || (DAT_005ccfa0 == 4)) && (25000 < uVar7 - param_1[0x9bb1])) {
              if ((DAT_005ccf98 != 0) && (iVar8 = FUN_00429a6d(0x6f), iVar8 != 0)) {
                uVar16 = 0;
                uVar13 = 0;
                FUN_00429a6d(0x6f);
                FUN_0042ad2b(uVar13,uVar16);
              }
              param_1[0x9bb1] = uVar7;
            }
          }
          else if ((DAT_005ccf98 != 0) && (iVar8 = FUN_00429a6d(0x6b), iVar8 != 0)) {
            FUN_00429a6d(0x6b);
            iVar8 = FUN_0042afd0();
            if (iVar8 != 0) {
              FUN_00429a6d(0x6b);
              FUN_0042aef6();
            }
          }
          if (DAT_005ccfa0 == 3) {
            if (((((uint)param_1[0x9c4a] < uVar7 - param_1[0x9c49]) && (param_1[0x9bb3] == 1)) &&
                (DAT_005d03f8 == 0)) && (*(int *)(param_1[0x11] + 0x164) == 0)) {
              if ((DAT_005ccf98 != 0) && (iVar8 = FUN_00429a6d(0x130), iVar8 != 0)) {
                uVar16 = 0;
                uVar13 = 0;
                FUN_00429a6d(0x130);
                FUN_0042ad2b(uVar13,uVar16);
              }
              if (DAT_0092e660 == 0) {
                for (local_2fc = 0; local_2fc < 5; local_2fc = local_2fc + 1) {
                  iVar8 = _rand();
                  local_300 = (float)((iVar8 % 5) * 0x32) + _DAT_005a43c4;
                  local_304 = *(undefined4 *)(DAT_013b71e8 + 0x1b084);
                  pvVar11 = operator_new(0x140);
                  local_8 = 0xe;
                  if (pvVar11 == (void *)0x0) {
                    local_864 = 0;
                  }
                  else {
                    local_864 = FUN_004cd800(0xc1,local_2fc * 800 + 3000,local_300,local_300,
                                             0x3f800000,1,0x50);
                  }
                  local_8 = 0xffffffff;
                  local_308 = local_864;
                  if (local_864 != 0) {
                    FUN_004cda99(local_2fc * 100 + -900,local_2fc * 100);
                    FUN_004cdad4(0x44c80000,0);
                    FUN_0054ac09(local_308);
                  }
                }
              }
              param_1[0x9c4b] = param_1[0x9c4b] + 1;
              param_1[0x9c4b] = param_1[0x9c4b] % 10;
              if (param_1[0x9c4b] == 0) {
                iVar8 = _rand();
                param_1[0x9c4a] = (iVar8 % 10 + 5) * 3000;
              }
              else {
                param_1[0x9c4a] = 1000;
              }
              param_1[0x9c49] = uVar7;
            }
            if (2000 < uVar7 - param_1[0x9c48]) {
              for (local_30c = 0; local_30c < 8; local_30c = local_30c + 1) {
                local_32c = _rand();
                local_32c = local_32c % 5;
                local_330 = _rand();
                local_330 = local_330 % 5;
                FUN_00430f50((*(float *)(param_1[0x13] + 0x28) - _DAT_005a3668) + (float)local_30c +
                             (float)(local_32c << 2),
                             *(float *)(param_1[0x13] + 0x30) + _DAT_005a366c,
                             (*(float *)(param_1[0x13] + 0x2c) - _DAT_005a3e9c) + (float)local_30c +
                             (float)(local_330 << 1));
                FUN_00430f50(*(float *)(param_1[0x13] + 0x28) + _DAT_005a3834 + (float)local_30c +
                             (float)(local_32c << 1),
                             *(float *)(param_1[0x13] + 0x30) + _DAT_005a34a0,
                             *(float *)(param_1[0x13] + 0x2c) + _DAT_005a3834 + (float)local_30c +
                             (float)local_330);
                pvVar11 = operator_new(0xc0);
                local_8 = 0xf;
                if (pvVar11 == (void *)0x0) {
                  local_874 = 0;
                }
                else {
                  local_874 = FUN_004e7610(local_318,local_314,local_310,local_324,local_320,
                                           local_31c,3,0);
                }
                local_8 = 0xffffffff;
                local_328 = local_874;
                if (local_874 != 0) {
                  FUN_0054ac09(local_874);
                }
              }
              param_1[0x9c48] = uVar7;
            }
          }
          iVar8 = (**(code **)(*DAT_0092e654 + 8))();
          param_1[0x9d1a] = iVar8;
        }
      }
      iVar8 = __ftol();
      if (((iVar8 >> 7 < 0x1b) || (iVar8 = __ftol(), 0x1e < iVar8 >> 7)) ||
         ((iVar8 = __ftol(), iVar8 >> 7 < 0x15 || (iVar8 = __ftol(), 0x18 < iVar8 >> 7)))) {
        bVar3 = false;
      }
      else {
        bVar3 = true;
      }
      if (bVar3) {
        uVar13 = __ftol();
        uVar13 = __ftol(uVar13);
        bVar4 = FUN_00433a60(uVar13);
        _local_334 = CONCAT31(uStack_333,bVar4);
        if (((((bVar4 & 8) == 0) && ((uint)param_1[0x9c4a] < uVar7 - param_1[0x9c49])) &&
            (param_1[0x9bb3] == 1)) &&
           ((DAT_005d03f8 == 0 && (*(int *)(param_1[0x11] + 0x164) == 1)))) {
          if ((DAT_005ccf98 != 0) && (iVar8 = FUN_00429a6d(0x130), iVar8 != 0)) {
            uVar16 = 0;
            uVar13 = 0;
            FUN_00429a6d(0x130);
            FUN_0042ad2b(uVar13,uVar16);
          }
          for (local_338 = 0; local_338 < 5; local_338 = local_338 + 1) {
            iVar8 = _rand();
            local_33c = (float)((iVar8 % 5) * 0x32) + _DAT_005a43c4;
            local_340 = *(undefined4 *)(DAT_013b71e8 + 0x1b084);
            pvVar11 = operator_new(0x140);
            local_8 = 0x10;
            if (pvVar11 == (void *)0x0) {
              local_880 = 0;
            }
            else {
              local_880 = FUN_004cd800(0x1a6,local_338 * 800 + 3000,local_33c,local_33c,0x3f800000,1
                                       ,0x50);
            }
            local_8 = 0xffffffff;
            local_344 = local_880;
            if (local_880 != 0) {
              FUN_004cda99(local_338 * 100 + -900,local_338 * 100);
              FUN_004cdad4(0x44c80000,0);
              FUN_0054ac09(local_344);
            }
          }
          param_1[0x9c4b] = param_1[0x9c4b] + 1;
          param_1[0x9c4b] = param_1[0x9c4b] % 10;
          if (param_1[0x9c4b] == 0) {
            iVar8 = _rand();
            param_1[0x9c4a] = (iVar8 % 10 + 5) * 3000;
          }
          else {
            param_1[0x9c4a] = 1000;
          }
          param_1[0x9c49] = uVar7;
        }
      }
      local_2c = param_1[0xa1c6];
      local_24 = param_1[0xa1c7];
      local_30 = -1;
      local_28 = -1;
      for (local_38 = 0; local_38 < 0x14; local_38 = local_38 + 1) {
        local_348 = *(char *)(DAT_013b71e8 + local_38 + 0xef6);
        local_34c = uVar7 - *(int *)(&DAT_0092eaf8 + local_348 * 4);
        if (-1 < local_348) {
          if (local_38 < 10) {
            local_30 = local_30 + 1;
          }
          else {
            local_28 = local_28 + 1;
          }
        }
        if (local_34c != 0) {
          local_354 = 0.0;
          local_350 = *(int *)(&DAT_0092ff04 + local_348 * 0x60);
          if ((8 < param_1[0x9b9e]) && (1 < local_350)) {
            local_350 = local_350 + -1;
          }
          iVar8 = FUN_004ff388();
          if (((iVar8 == 2) && (local_348 == 'f')) && (0x4e < *(short *)(DAT_013b71e8 + 0x72c))) {
            local_350 = 1000;
          }
          if (local_350 < 1) {
            local_354 = (float)local_34c / _DAT_005a43c0;
          }
          else {
            local_354 = (float)local_34c / ((float)local_350 * _DAT_005a43c0);
          }
          if ((_DAT_005a3534 < local_354) && (local_354 < _DAT_005a40f4)) {
            if (local_38 < 10) {
              local_358 = *(int *)(local_2c + 0x200 + local_30 * 4);
            }
            else {
              local_358 = *(int *)(local_24 + 0x200 + local_28 * 4);
            }
            if (local_358 == 0) {
              ExceptionList = local_10;
              return 1;
            }
            if ((int)**(short **)(local_358 + 0x670) == local_348 + 5000) {
              *(float *)(local_358 + 0x674) = local_354;
            }
          }
        }
      }
      if (10000 < uVar7 - param_1[0x9d19]) {
        local_3dc = param_1[0x9eba];
        FUN_0058f078(local_3d8,&DAT_005d04ac);
        pvVar11 = operator_new(0xe50);
        local_8 = 0x11;
        if (pvVar11 == (void *)0x0) {
          local_894 = 0;
        }
        else {
          local_894 = FUN_00407203(local_3d8,0xffaaffaa,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
        }
        local_8 = 0xffffffff;
        local_3e0 = local_894;
        if (local_894 != 0) {
          FUN_00408d33(local_894);
          param_1[0x9d19] = uVar7;
        }
      }
      if ((param_1[0x9e35] != 0) && (iVar8 = FUN_0040c0f0(), iVar8 != 0)) {
        if (((((((*(int *)(param_1[0x10] + 0x20a20) == 1) &&
                (*(int *)(param_1[0x10] + 0x20a24) == 0x1f)) ||
               ((*(int *)(param_1[0x10] + 0x20a20) == 6 &&
                (*(int *)(param_1[0x10] + 0x20a24) == 0x1c)))) ||
              ((*(int *)(param_1[0x10] + 0x20a20) == 9 &&
               (*(int *)(param_1[0x10] + 0x20a24) == 0x1c)))) ||
             (((((*(int *)(param_1[0x10] + 0x20a20) == 8 &&
                 (*(int *)(param_1[0x10] + 0x20a24) == 0x1b)) ||
                ((*(int *)(param_1[0x10] + 0x20a20) == 10 &&
                 (*(int *)(param_1[0x10] + 0x20a24) == 0x1b)))) ||
               ((*(int *)(param_1[0x10] + 0x20a20) == 10 &&
                (*(int *)(param_1[0x10] + 0x20a24) == 0xb)))) ||
              ((*(int *)(param_1[0x10] + 0x20a20) == 0xd &&
               (*(int *)(param_1[0x10] + 0x20a24) == 0x1f)))))) ||
            (((((*(int *)(param_1[0x10] + 0x20a20) == 0xe &&
                (*(int *)(param_1[0x10] + 0x20a24) == 0x1e)) ||
               ((*(int *)(param_1[0x10] + 0x20a20) == 0xf &&
                (*(int *)(param_1[0x10] + 0x20a24) == 0x1f)))) ||
              (((*(int *)(param_1[0x10] + 0x20a20) == 8 && (*(int *)(param_1[0x10] + 0x20a24) == 2))
               || ((*(int *)(param_1[0x10] + 0x20a20) == 9 &&
                   (*(int *)(param_1[0x10] + 0x20a24) == 1)))))) ||
             (((*(int *)(param_1[0x10] + 0x20a20) == 10 && (*(int *)(param_1[0x10] + 0x20a24) == 2))
              || ((*(int *)(param_1[0x10] + 0x20a20) == 0xd &&
                  (*(int *)(param_1[0x10] + 0x20a24) == 0x1c)))))))) ||
           (((*(int *)(param_1[0x10] + 0x20a20) == 0xe &&
             (*(int *)(param_1[0x10] + 0x20a24) == 0x1c)) ||
            ((*(int *)(param_1[0x10] + 0x20a20) == 1 && (*(int *)(param_1[0x10] + 0x20a24) == 1)))))
           ) {
          local_3e4 = uVar7 - param_1[0x9d72];
          local_3e8 = local_3e4 / 1000;
          local_3ec = param_1[0x9d71] - local_3e8;
          if (local_3ec < 1) {
            (**(code **)(*(int *)param_1[0x9e35] + 0x60))(0);
          }
          else {
            _memset(local_46c,0,0x80);
            FUN_0058f078(local_46c,s__02d____02d_005b870c,local_3ec / 0x3c,local_3ec % 0x3c);
            (**(code **)(*(int *)param_1[0x9e35] + 0x80))(local_46c,0);
          }
        }
        else {
          (**(code **)(*(int *)param_1[0x9e35] + 0x60))(0);
        }
      }
      if (((((char)param_1[0x9bad] == '\x01') && (param_1[0x9ba4] != 0)) &&
          (*(char *)(param_1[0x9ba4] + 0x245) == '\0')) && (200 < uVar7 - param_1[0x9bae])) {
        if (((*(short *)(param_1[0x13] + 0x680) != *(short *)(param_1[0x9ba4] + 0x680)) &&
            (*(short *)(DAT_013b71e8 + 0xf12) != *(short *)(param_1[0x9ba4] + 0x680))) ||
           ((*(short *)(param_1[0x13] + 0x680) == 0 || (*(short *)(param_1[0x9ba4] + 0x680) == 0))))
        {
          FUN_0051a939(param_1[0x9ba4]);
          param_1[0x9bae] = uVar7;
        }
      }
      else if ((((char)param_1[0x9bad] == '\x01') && (param_1[0x9ba4] != 0)) &&
              (*(char *)(param_1[0x9ba4] + 0x245) == '\x01')) {
        param_1[0x9ba4] = 0;
      }
      if (param_1[0x13] != 0) {
        uVar13 = __ftol();
        uVar13 = __ftol(uVar13);
        uVar5 = FUN_00433a60(uVar13);
        local_470 = CONCAT31(local_470._1_3_,uVar5);
        local_480 = -1;
        local_474 = __ftol();
        local_474 = local_474 & 0xfffc;
        local_47c = __ftol();
        local_47c = local_47c & 0xfffc;
        for (local_478 = 0; local_478 < 0x22; local_478 = local_478 + 1) {
          if ((*(uint *)(&DAT_005b4da0 + local_478 * 0x10) == local_474) &&
             (*(uint *)(&DAT_005b4da4 + local_478 * 0x10) == local_47c)) {
            local_480 = local_478;
            break;
          }
        }
        if ((*(int *)(param_1[0x13] + 0x740) == 0) || (*(int *)(param_1[0x13] + 0x740) == 1)) {
          bVar4 = *(byte *)(param_1 + 0x9baa);
          _local_484 = CONCAT31(uStack_483,bVar4);
          *(undefined1 *)(param_1 + 0x9baa) = (undefined1)local_470;
          if (((local_470 & 0x10) != 0) && (param_1[0xa398] == 0)) {
            param_1[0xa398] = 1;
            if (param_1[0x9bab] != -1) {
              local_490 = (int)(*(int *)(&DAT_005b4da0 + local_480 * 0x10) +
                               (*(int *)(&DAT_005b4da0 + local_480 * 0x10) >> 0x1f & 0x3fU)) >> 6;
              local_494 = (int)(*(int *)(&DAT_005b4da4 + local_480 * 0x10) +
                               (*(int *)(&DAT_005b4da4 + local_480 * 0x10) >> 0x1f & 0x3fU)) >> 6;
              local_48c = (int)(*(int *)(&DAT_005b4da0 + param_1[0x9bab] * 0x10) +
                               (*(int *)(&DAT_005b4da0 + param_1[0x9bab] * 0x10) >> 0x1f & 0x3fU))
                          >> 6;
              local_488 = (int)(*(int *)(&DAT_005b4da4 + param_1[0x9bab] * 0x10) +
                               (*(int *)(&DAT_005b4da4 + param_1[0x9bab] * 0x10) >> 0x1f & 0x3fU))
                          >> 6;
              if (((local_490 != local_48c) || (local_494 != local_488)) &&
                 ((uint)bVar4 == (local_470 & 0xff))) {
                param_1[0x9bab] = local_480;
                ExceptionList = local_10;
                return 1;
              }
            }
            iVar8 = __ftol();
            if (((iVar8 < 0x7ac) || (iVar8 = __ftol(), 0x7b1 < iVar8)) ||
               ((iVar8 = __ftol(), iVar8 < 0x6eb || (iVar8 = __ftol(), 0x6f0 < iVar8)))) {
              iVar8 = __ftol();
              if (((0x941 < iVar8) && (iVar8 = __ftol(), iVar8 < 0x96c)) &&
                 ((iVar8 = __ftol(), 0x6bf < iVar8 && (iVar8 = __ftol(), iVar8 < 0x6e0)))) {
                FUN_00403df2(&DAT_00a3c278,3000);
                (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
                ExceptionList = local_10;
                return 1;
              }
              iVar8 = FUN_0040c0f0();
              if (iVar8 == 0) {
                if (param_1[0x9b87] == 1) {
                  ExceptionList = local_10;
                  return 1;
                }
                iVar8 = __ftol();
                if ((((_DAT_005a43b8 < (double)iVar8) &&
                     (iVar8 = __ftol(), (double)iVar8 < _DAT_005a43b0)) &&
                    (iVar8 = __ftol(), _DAT_005a43a8 < (double)iVar8)) &&
                   ((iVar8 = __ftol(), (double)iVar8 < _DAT_005a43a0 && (param_1[0x9daf] != 0)))) {
                  ExceptionList = local_10;
                  return 1;
                }
                param_1[0x9bab] = local_480;
                if (local_480 < 0) {
                  (**(code **)(*(int *)param_1[0x23] + 0x8c))(&DAT_00a356f8,0x10,0);
                }
                else {
                  _memset(local_598,0,0x100);
                  FUN_0058f078(local_598,&DAT_00a3a978,(&PTR_DAT_005b4dac)[local_480 * 4]);
                  local_498 = 0;
                  if ((((char)param_1[0x9baf] == '\0') && (*(char *)(param_1[0x13] + 0x79b) != '\0')
                      ) && (*(char *)(param_1[0x13] + 0x79b) != '\x03')) {
                    local_498 = 1;
                  }
                  if (((char)param_1[0x9baf] == '\a') &&
                     (*(char *)(param_1[0x13] + 0x79b) != '\x01')) {
                    local_498 = 1;
                  }
                  if (((char)param_1[0x9baf] == '\b') &&
                     (*(char *)(param_1[0x13] + 0x79b) != '\x02')) {
                    local_498 = 1;
                  }
                  if ((char)param_1[0x9baf] == -1) {
                    local_498 = 1;
                  }
                  if ((*(int *)(&DAT_005b4da8 + local_480 * 0x10) < 1) || (local_498 != 1)) {
                    (**(code **)(*(int *)param_1[0x23] + 0x8c))(local_598,0x10,0);
                  }
                  else {
                    _memset(local_698,0,0x100);
                    FUN_0058f078(local_698,&DAT_00a3a8f8,
                                 *(undefined4 *)(&DAT_005b4da8 + local_480 * 0x10));
                    (**(code **)(*(int *)param_1[0x23] + 0x8c))(local_598,0x10,local_698);
                  }
                }
                (**(code **)(*(int *)param_1[0x23] + 0x60))(1);
              }
            }
            else {
              FUN_00403df2(&DAT_00a39178,3000);
              (**(code **)(*(int *)param_1[0x21] + 0x88))(1,1);
            }
          }
        }
        if ((((local_470 & 0x10) == 0) && (iVar8 = FUN_0040c0f0(), iVar8 == 1)) &&
           (*(int *)(param_1[0x23] + 0x1ec) == 0x10)) {
          (**(code **)(*(int *)param_1[0x23] + 0x60))(0);
        }
      }
      local_44 = 0x21;
      for (local_3c = 0; local_3c < 0x60; local_3c = local_3c + 1) {
        if (local_3c == 0x35) {
          local_44 = 0x14;
        }
        sVar1 = (&DAT_00e22c00)[local_3c * 4];
        iVar8 = __ftol();
        if (sVar1 == iVar8 || sVar1 - iVar8 < 0) {
          sVar1 = (&DAT_00e22c00)[local_3c * 4];
          iVar8 = __ftol();
          local_8a8 = -(sVar1 - iVar8);
        }
        else {
          sVar1 = (&DAT_00e22c00)[local_3c * 4];
          local_8a8 = __ftol();
          local_8a8 = sVar1 - local_8a8;
        }
        if (local_8a8 < local_44 + -1) {
          sVar1 = (&DAT_00e22c02)[local_3c * 4];
          iVar8 = __ftol();
          if (sVar1 == iVar8 || sVar1 - iVar8 < 0) {
            sVar1 = (&DAT_00e22c02)[local_3c * 4];
            iVar8 = __ftol();
            local_8ac = -(sVar1 - iVar8);
          }
          else {
            sVar1 = (&DAT_00e22c02)[local_3c * 4];
            local_8ac = __ftol();
            local_8ac = sVar1 - local_8ac;
          }
          if (local_8ac < local_44 + -1) {
            FUN_0047591f(local_3c,param_1[0x9b9d]);
          }
        }
      }
      if (param_1[0x9b9d] == 1) {
        param_1[0x9b9d] = 0;
      }
      FUN_0047e9b6();
    }
  }
  ExceptionList = local_10;
  return 1;
}

