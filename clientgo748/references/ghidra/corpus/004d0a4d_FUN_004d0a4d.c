// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d0a4d | Name: FUN_004d0a4d


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004d0a4d(int param_1)

{
  undefined4 *puVar1;
  float *pfVar2;
  int iVar3;
  int iVar4;
  float10 fVar5;
  undefined1 *puVar6;
  float fVar7;
  undefined1 *puVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined1 local_11c [12];
  undefined1 local_110 [12];
  undefined1 local_104 [12];
  undefined1 local_f8 [12];
  undefined1 local_ec [12];
  undefined1 local_e0 [12];
  undefined1 local_d4 [12];
  undefined1 local_c8 [12];
  undefined1 local_bc [12];
  undefined4 local_b0;
  undefined4 local_ac;
  undefined4 local_a8;
  undefined4 local_a4;
  float local_a0;
  float local_9c;
  float local_98;
  float local_94;
  float local_90;
  float local_8c;
  float local_88;
  float local_84;
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
  float local_58;
  float local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  int local_38;
  float local_34;
  float local_30;
  int local_2c;
  undefined1 local_28 [12];
  uint local_1c;
  float local_18;
  int local_14;
  uint local_10;
  float local_c;
  uint local_8;
  
  iVar3 = (**(code **)(*DAT_0092e654 + 8))();
  local_8 = iVar3 - *(int *)(param_1 + 0x198c);
  if (local_8 < 0x2711) {
    FUN_00430f50(0,0xc11ccccd,0);
    local_c = (float)(uint)(iVar3 - *(int *)(param_1 + 0x198c)) / _DAT_005a35c0 - _DAT_005a3660;
    local_30 = 1.8;
    local_38 = 0;
    if (7000 < local_8) {
      local_8 = 7000;
    }
    local_18 = (float)local_8 / _DAT_005a4968;
    fVar5 = (float10)FUN_00423b00(local_18 * _DAT_005a4148);
    local_34 = (float)fVar5;
    local_14 = 0;
    local_2c = 0;
    local_10 = 0;
    if ((*(int *)(param_1 + 0x24) == 0) || (*(int *)(param_1 + 0x1e44) == 0)) {
      local_14 = __ftol();
      local_2c = __ftol();
    }
    else if ((*(int *)(param_1 + 0x24) == 1) || (*(int *)(param_1 + 0x1e44) == 1)) {
      local_2c = __ftol();
    }
    else if ((*(int *)(param_1 + 0x24) == 2) || (*(int *)(param_1 + 0x1e44) == 2)) {
      local_14 = __ftol();
      local_10 = __ftol();
    }
    else if ((*(int *)(param_1 + 0x24) == 3) || (*(int *)(param_1 + 0x1e44) == 3)) {
      local_14 = __ftol();
      local_2c = __ftol();
      local_10 = __ftol();
    }
    else if ((*(int *)(param_1 + 0x24) == 4) || (*(int *)(param_1 + 0x1e44) == 4)) {
      local_14 = __ftol();
    }
    else if ((*(int *)(param_1 + 0x24) == 5) || (*(int *)(param_1 + 0x1e44) == 5)) {
      local_2c = __ftol();
      local_10 = __ftol();
    }
    *(uint *)(param_1 + 0x28) = local_14 << 0x10 | local_2c << 8 | local_10;
    for (local_1c = 0; local_1c < 100; local_1c = local_1c + 1) {
      if (_DAT_005a3534 <= local_c) {
        if (*(int *)(param_1 + 0x24) < 6) {
          if (*(int *)(param_1 + 0x1990) == 0) {
            *(undefined4 *)(param_1 + 0x1990) = 1;
            local_80 = 0x13c;
            if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x13c), iVar3 != 0)) {
              uVar10 = 0;
              uVar9 = 0;
              FUN_00429a6d(local_80);
              FUN_0042ad2b(uVar9,uVar10);
            }
          }
          puVar1 = (undefined4 *)(param_1 + 0xb0 + local_1c * 0x40);
          local_7c = *puVar1;
          local_78 = puVar1[1];
          local_74 = puVar1[2];
          puVar1 = (undefined4 *)(param_1 + 0xa4 + local_1c * 0x40);
          local_70 = *puVar1;
          local_6c = puVar1[1];
          local_68 = puVar1[2];
          fVar5 = (float10)FUN_004d1940(-local_30 * local_c);
          local_58 = (float)((((float10)_DAT_005a3660 - fVar5) * (float10)_DAT_005a34a0) /
                            ((float10)local_30 * (float10)local_30));
          local_64 = local_7c;
          local_60 = local_78;
          local_5c = local_74;
          FUN_004d1960(local_30);
          puVar8 = local_110;
          fVar7 = local_58;
          FUN_00493d50(local_104,local_28);
          uVar9 = FUN_004c8c60(puVar8,fVar7);
          puVar8 = local_11c;
          puVar6 = local_ec;
          fVar7 = local_30;
          FUN_004c8c60(local_e0,local_c);
          uVar10 = FUN_00493e10(puVar6,fVar7);
          FUN_00493d50(local_f8,uVar10);
          uVar9 = FUN_00493d50(puVar8,uVar9);
          FUN_004310a0(uVar9);
          *(float *)(param_1 + 0xc0 + local_1c * 0x40) =
               local_c / *(float *)(param_1 + 0xbc + local_1c * 0x40);
          local_54 = *(float *)(param_1 + 0xc0 + local_1c * 0x40) + _DAT_005a34a0;
          fVar5 = (float10)FUN_004d1940(_DAT_005a4964 * local_54 * local_54);
          *(float *)(param_1 + 200 + local_1c * 0x40) = (float)(fVar5 * (float10)_DAT_005a40ec);
          if (_DAT_005a3660 <= *(float *)(param_1 + 0xc0 + local_1c * 0x40)) {
            local_38 = local_38 + 1;
          }
        }
        else if (*(int *)(param_1 + 0x24) == 6) {
          if (*(int *)(param_1 + 0x1990) == 0) {
            *(undefined4 *)(param_1 + 0x1990) = 1;
            local_b0 = 0x13c;
            if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x13c), iVar3 != 0)) {
              uVar10 = 0;
              uVar9 = 0;
              FUN_00429a6d(local_b0);
              FUN_0042ad2b(uVar9,uVar10);
            }
          }
          puVar1 = (undefined4 *)(param_1 + 0xb0 + local_1c * 0x40);
          local_ac = *puVar1;
          local_a8 = puVar1[1];
          local_a4 = puVar1[2];
          pfVar2 = (float *)(param_1 + 0xa4 + local_1c * 0x40);
          local_a0 = *pfVar2;
          local_9c = pfVar2[1];
          local_98 = pfVar2[2];
          CSimpleArray<>();
          local_94 = 0.5;
          if (_DAT_005a34a0 <= local_c) {
            local_90 = *(float *)(param_1 + 0x1994 + local_1c * 0xc) * 0.5 + local_a0;
            local_88 = *(float *)(param_1 + 0x199c + local_1c * 0xc) * 0.5 + local_98;
          }
          else {
            fVar5 = (float10)FUN_00423b00(local_c * _DAT_005a4148);
            local_94 = (float)(fVar5 * (float10)_DAT_005a34a0);
            local_90 = local_94 * *(float *)(param_1 + 0x1994 + local_1c * 0xc) + local_a0;
            local_88 = local_94 * *(float *)(param_1 + 0x199c + local_1c * 0xc) + local_98;
          }
          fVar7 = local_94 * *(float *)(param_1 + 0x1998 + local_1c * 0xc) + local_9c;
          fVar5 = (float10)FUN_00423b00((local_c * _DAT_005a4148) / _DAT_005a3834,fVar7);
          local_8c = (float)((float10)fVar7 - fVar5 * (float10)local_c * (float10)_DAT_005a40f0);
          FUN_004310a0(&local_90);
          *(float *)(param_1 + 0xc0 + local_1c * 0x40) =
               local_c / *(float *)(param_1 + 0xbc + local_1c * 0x40);
          local_84 = *(float *)(param_1 + 0xc0 + local_1c * 0x40) + _DAT_005a34a0;
          fVar5 = (float10)FUN_004d1940(_DAT_005a4964 * local_84 * local_84);
          *(float *)(param_1 + 200 + local_1c * 0x40) = (float)(fVar5 * (float10)_DAT_005a40ec);
          if (_DAT_005a3660 <= *(float *)(param_1 + 0xc0 + local_1c * 0x40)) {
            local_38 = local_38 + 1;
          }
        }
      }
      else {
        puVar1 = (undefined4 *)(param_1 + 0x98 + local_1c * 0x40);
        local_50 = *puVar1;
        local_4c = puVar1[1];
        local_48 = puVar1[2];
        puVar1 = (undefined4 *)(param_1 + 0xa4 + local_1c * 0x40);
        local_44 = *puVar1;
        local_40 = puVar1[1];
        local_3c = puVar1[2];
        uVar9 = 0x3fc00000;
        puVar8 = local_c8;
        iVar3 = _rand();
        iVar4 = _rand();
        FUN_004c8c60(local_bc,local_c - (((float)iVar3 - (float)iVar4) / _DAT_005a4960 +
                                        _DAT_005a3660) / _DAT_005a4314);
        uVar9 = FUN_00493e10(puVar8,uVar9);
        uVar9 = FUN_00493d50(local_d4,uVar9);
        FUN_004310a0(uVar9);
      }
    }
  }
  else {
    (**(code **)(*DAT_013b71e8 + 100))(param_1);
  }
  return 1;
}

