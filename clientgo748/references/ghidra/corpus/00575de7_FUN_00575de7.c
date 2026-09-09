// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00575de7 | Name: FUN_00575de7


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00575de7(float *param_1,float *param_2,int param_3,float param_4)

{
  float *pfVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  int iVar10;
  int iVar11;
  float *pfVar12;
  int iVar13;
  uint uVar14;
  int iVar15;
  uint uVar16;
  float *pfVar17;
  float local_c4 [16];
  float *local_84;
  int local_80;
  float local_7c;
  float local_78;
  float local_74;
  float *local_6c;
  undefined4 *local_68;
  float local_64;
  float local_60;
  float local_5c;
  float local_58;
  float local_54;
  float local_50;
  float local_48;
  float local_44;
  float local_40;
  float local_3c;
  float local_34 [6];
  float local_1c;
  float local_14;
  float local_10;
  float local_c;
  undefined4 uStack_8;
  
  iVar10 = (int)param_4;
  if (param_4 == 4.2039e-45) {
    local_6c = (float *)&DAT_005a85f8;
    local_68 = &DAT_005a85ec;
  }
  else {
    local_6c = (float *)&DAT_005a85dc;
    local_68 = (undefined4 *)&DAT_005a85cc;
  }
  local_34[4] = 0.0;
  local_34[5] = 0.0;
  local_14 = DAT_005c6d80;
  local_10 = DAT_005c6d84;
  local_1c = 0.0;
  local_c = DAT_005c6d88;
  uStack_8 = DAT_005c6d8c;
  pfVar17 = (float *)(param_3 + 8);
  iVar11 = 0x10;
  local_84 = pfVar17;
  pfVar12 = pfVar17;
  iVar13 = iVar11;
  do {
    if (pfVar12[-2] < local_14) {
      local_14 = pfVar12[-2];
    }
    if (pfVar12[-1] < local_10) {
      local_10 = pfVar12[-1];
    }
    if (*pfVar12 < local_c) {
      local_c = *pfVar12;
    }
    if (local_34[4] < pfVar12[-2]) {
      local_34[4] = pfVar12[-2];
    }
    if (local_34[5] < pfVar12[-1]) {
      local_34[5] = pfVar12[-1];
    }
    if (local_1c < *pfVar12) {
      local_1c = *pfVar12;
    }
    fVar4 = local_1c;
    pfVar12 = pfVar12 + 4;
    iVar13 = iVar13 + -1;
  } while (iVar13 != 0);
  fVar5 = local_34[4] - local_14;
  fVar7 = local_34[5] - local_10;
  fVar8 = local_1c - local_c;
  local_48 = fVar5 * fVar5 + fVar7 * fVar7 + fVar8 * fVar8;
  if ((float)PTR_DAT_005a6810 <= local_48) {
    fVar9 = 1.0 / local_48;
    local_7c = fVar5 * fVar9;
    local_78 = fVar7 * fVar9;
    local_74 = fVar9 * fVar8;
    local_58 = (local_34[4] + local_14) * _DAT_005a67d4;
    local_54 = (local_34[5] + local_10) * _DAT_005a67d4;
    local_50 = (local_1c + local_c) * _DAT_005a67d4;
    local_34[3] = 0.0;
    local_34[2] = 0.0;
    local_34[1] = 0.0;
    fVar3 = 0.0;
    do {
      local_44 = (pfVar17[-2] - local_58) * fVar5 * fVar9;
      local_40 = (pfVar17[-1] - local_54) * fVar7 * fVar9;
      fVar2 = *pfVar17;
      pfVar17 = pfVar17 + 4;
      iVar11 = iVar11 + -1;
      fVar2 = (fVar2 - local_50) * fVar9 * fVar8;
      fVar6 = local_40 + fVar2 + local_44;
      fVar3 = fVar6 * fVar6 + fVar3;
      fVar6 = (local_40 + local_44) - fVar2;
      local_34[1] = fVar6 * fVar6 + local_34[1];
      fVar6 = (local_44 - local_40) + fVar2;
      local_34[2] = fVar6 * fVar6 + local_34[2];
      fVar2 = (local_44 - local_40) - fVar2;
      local_34[3] = fVar2 * fVar2 + local_34[3];
    } while (iVar11 != 0);
    local_34[0] = fVar3;
    uVar16 = 0;
    uVar14 = 1;
    do {
      if (fVar3 < local_34[uVar14]) {
        fVar3 = local_34[uVar14];
        uVar16 = uVar14;
      }
      uVar14 = uVar14 + 1;
    } while (uVar14 < 4);
    fVar5 = local_10;
    if ((uVar16 & 2) != 0) {
      fVar5 = local_34[5];
      local_34[5] = local_10;
    }
    local_10 = fVar5;
    if ((uVar16 & 1) != 0) {
      local_1c = local_c;
      local_c = fVar4;
    }
    if (_DAT_005a85c8 <= local_48) {
      local_60 = (float)((int)param_4 + -1);
      if ((int)param_4 + -1 < 0) {
        local_60 = local_60 + _DAT_005a67dc;
      }
      FUN_005759c5();
      local_48 = 0.0;
      while( true ) {
        iVar11 = 0x10;
        if (iVar10 != 0) {
          pfVar12 = local_c4 + 1;
          iVar15 = (int)local_68 - (int)local_6c;
          pfVar17 = local_6c;
          iVar13 = iVar10;
          do {
            pfVar12[-1] = local_34[4] * *(float *)(iVar15 + (int)pfVar17) + local_14 * *pfVar17;
            *pfVar12 = local_34[5] * *(float *)(iVar15 + (int)pfVar17) + local_10 * *pfVar17;
            fVar4 = *pfVar17;
            pfVar1 = (float *)(iVar15 + (int)pfVar17);
            pfVar17 = pfVar17 + 1;
            pfVar12[1] = local_1c * *pfVar1 + local_c * fVar4;
            pfVar12 = pfVar12 + 4;
            iVar13 = iVar13 + -1;
          } while (iVar13 != 0);
        }
        fVar5 = local_34[4] - local_14;
        fVar4 = local_34[5] - local_10;
        fVar8 = local_1c - local_c;
        fVar7 = fVar5 * fVar5 + fVar4 * fVar4 + fVar8 * fVar8;
        if (fVar7 < _DAT_005a85c8) break;
        fVar7 = local_60 / fVar7;
        local_44 = fVar7 * fVar5;
        local_40 = fVar4 * fVar7;
        local_3c = fVar7 * fVar8;
        local_34[2] = 0.0;
        local_34[1] = 0.0;
        local_34[0] = 0.0;
        local_50 = 0.0;
        local_54 = 0.0;
        local_58 = 0.0;
        local_5c = 0.0;
        param_4 = 0.0;
        pfVar12 = local_84;
        do {
          fVar3 = (*pfVar12 - local_c) * fVar7 * fVar8 +
                  (pfVar12[-2] - local_14) * fVar7 * fVar5 +
                  (pfVar12[-1] - local_10) * fVar4 * fVar7;
          local_64 = fVar3;
          if (fVar3 < local_60) {
            local_64 = fVar3 + _DAT_005a67d4;
            local_80 = (int)ROUND(fVar3 + _DAT_005a67d4);
          }
          else {
            local_80 = iVar10 + -1;
          }
          pfVar17 = pfVar12 + -2;
          local_78 = local_c4[local_80 * 4 + 1] - pfVar12[-1];
          fVar3 = *pfVar12;
          fVar2 = local_6c[local_80] * _DAT_005a67f0;
          pfVar12 = pfVar12 + 4;
          iVar11 = iVar11 + -1;
          fVar9 = (float)local_68[local_80] * _DAT_005a67f0;
          param_4 = fVar2 * local_6c[local_80] + param_4;
          local_58 = fVar2 * (local_c4[local_80 * 4] - *pfVar17) + local_58;
          local_54 = fVar2 * local_78 + local_54;
          local_50 = fVar2 * (local_c4[local_80 * 4 + 2] - fVar3) + local_50;
          local_5c = fVar9 * (float)local_68[local_80] + local_5c;
          local_34[0] = fVar9 * (local_c4[local_80 * 4] - *pfVar17) + local_34[0];
          local_34[1] = local_78 * fVar9 + local_34[1];
          local_34[2] = fVar9 * (local_c4[local_80 * 4 + 2] - fVar3) + local_34[2];
        } while (iVar11 != 0);
        if (_DAT_005a67ec < param_4) {
          param_4 = _DAT_005a67f4 / param_4;
          local_14 = local_58 * param_4 + local_14;
          local_10 = local_54 * param_4 + local_10;
          local_c = param_4 * local_50 + local_c;
        }
        if (_DAT_005a67ec < local_5c) {
          local_5c = _DAT_005a67f4 / local_5c;
          local_34[4] = local_34[0] * local_5c + local_34[4];
          local_34[5] = local_34[1] * local_5c + local_34[5];
          local_1c = local_5c * local_34[2] + local_1c;
        }
        if (((((local_58 * local_58 < _DAT_005a85c4) && (local_54 * local_54 < _DAT_005a85c4)) &&
             (local_50 * local_50 < _DAT_005a85c4)) &&
            (((local_34[0] * local_34[0] < _DAT_005a85c4 &&
              (local_34[1] * local_34[1] < _DAT_005a85c4)) &&
             (local_34[2] * local_34[2] < _DAT_005a85c4)))) ||
           (local_48 = (float)((int)local_48 + 1), 7 < (uint)local_48)) break;
      }
      *param_1 = local_14;
      param_1[1] = local_10;
      param_1[2] = local_c;
      *param_2 = local_34[4];
      param_2[1] = local_34[5];
    }
    else {
      *param_1 = local_14;
      param_1[1] = local_10;
      param_1[2] = local_c;
      param_2[1] = local_34[5];
      *param_2 = local_34[4];
    }
  }
  else {
    *param_1 = local_14;
    param_1[1] = local_10;
    param_1[2] = local_c;
    param_2[1] = local_34[5];
    *param_2 = local_34[4];
  }
  param_2[2] = local_1c;
  return;
}

