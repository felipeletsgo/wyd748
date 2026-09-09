// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00576f7f | Name: FUN_00576f7f


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_00576f7f(char *param_1,int param_2)

{
  char *pcVar1;
  float fVar2;
  float fVar3;
  char cVar4;
  char cVar5;
  int iVar6;
  uint uVar7;
  float *pfVar8;
  float fVar9;
  uint uVar10;
  float *pfVar11;
  float local_a4 [32];
  int local_24;
  float local_20;
  float local_1c;
  float local_18;
  undefined *local_14;
  float *local_10;
  float *local_c;
  float local_8;
  
  pfVar8 = (float *)(param_2 + 0xc);
  local_10 = (float *)*pfVar8;
  pfVar11 = local_a4;
  local_c = local_10;
  for (iVar6 = 0x10; iVar6 != 0; iVar6 = iVar6 + -1) {
    *pfVar11 = 0.0;
    pfVar11 = pfVar11 + 1;
  }
  FUN_005759c5();
  pcVar1 = param_1;
  uVar7 = 0;
  pfVar11 = pfVar8;
  do {
    local_18 = local_a4[uVar7] + *pfVar11;
    local_14 = (undefined *)(int)ROUND(local_18 * _DAT_005a696c + _DAT_005a67d4);
    local_8 = (float)local_14;
    fVar9 = (float)(int)local_14 * _DAT_005a68b0;
    local_a4[uVar7 + 0x10] = fVar9;
    if ((float)local_c <= fVar9) {
      if ((float)local_10 < fVar9) {
        local_10 = (float *)fVar9;
      }
    }
    else {
      local_c = (float *)fVar9;
    }
    fVar9 = local_18 - fVar9;
    uVar10 = uVar7 & 3;
    if (uVar10 != 3) {
      local_a4[uVar7 + 1] = _DAT_005a862c * fVar9 + local_a4[uVar7 + 1];
    }
    if (uVar7 < 0xc) {
      if (uVar10 != 0) {
        local_a4[uVar7 + 3] = _DAT_005a8628 * fVar9 + local_a4[uVar7 + 3];
      }
      local_a4[uVar7 + 4] = _DAT_005a8624 * fVar9 + local_a4[uVar7 + 4];
      if (uVar10 != 3) {
        local_a4[uVar7 + 5] = fVar9 * _DAT_005a69c0 + local_a4[uVar7 + 5];
      }
    }
    uVar7 = uVar7 + 1;
    pfVar11 = pfVar11 + 4;
  } while (uVar7 < 0x10);
  local_8 = DAT_013b7f74;
  iVar6 = FUN_00576352(param_1 + 8,0);
  if (iVar6 < 0) {
    return iVar6;
  }
  if ((float)local_c == 1.0) {
    *pcVar1 = -1;
    pcVar1[1] = -1;
LAB_00577161:
    pcVar1[2] = '\0';
    pcVar1[3] = '\0';
    pcVar1[4] = '\0';
    pcVar1[5] = '\0';
    pcVar1[6] = '\0';
    pcVar1[7] = '\0';
  }
  else {
    if (((float)local_c == 0.0) || ((float)local_10 == 1.0)) {
      param_2 = 6;
      iVar6 = 6;
    }
    else {
      iVar6 = 8;
      param_2 = 8;
    }
    FUN_00575ae2(&local_8,&param_1,local_a4 + 0x10,iVar6);
    FUN_005759c5();
    uVar7 = (uint)ROUND(local_8 * _DAT_005a696c + _DAT_005a67d4);
    local_8 = (float)(int)ROUND((float)param_1 * _DAT_005a696c + _DAT_005a67d4);
    fVar9 = (float)(uVar7 & 0xff) * _DAT_005a68b0;
    local_a4[0x19] = (float)((uint)local_8 & 0xff) * _DAT_005a68b0;
    cVar4 = (char)uVar7;
    cVar5 = SUB41(local_8,0);
    if (iVar6 == 8) {
      if (cVar4 == cVar5) {
        *pcVar1 = cVar4;
        pcVar1[1] = cVar5;
        goto LAB_00577161;
      }
LAB_005771d6:
      pcVar1[1] = cVar4;
      *pcVar1 = cVar5;
      uVar7 = 1;
      do {
        fVar2 = (float)(int)(7 - uVar7);
        if ((int)(7 - uVar7) < 0) {
          fVar2 = fVar2 + _DAT_005a67dc;
        }
        fVar3 = (float)(int)uVar7;
        if ((int)uVar7 < 0) {
          fVar3 = fVar3 + _DAT_005a67dc;
        }
        uVar10 = uVar7 + 1;
        local_a4[uVar7 + 0x19] = (fVar3 * fVar9 + fVar2 * local_a4[0x19]) * _DAT_005a69b8;
        uVar7 = uVar10;
      } while (uVar10 < 7);
      local_14 = &DAT_005a8634;
      local_a4[0x18] = local_a4[0x19];
      local_a4[0x19] = fVar9;
    }
    else {
      if (iVar6 != 6) goto LAB_005771d6;
      *pcVar1 = cVar4;
      pcVar1[1] = cVar5;
      uVar7 = 1;
      do {
        fVar2 = (float)(int)(5 - uVar7);
        if ((int)(5 - uVar7) < 0) {
          fVar2 = fVar2 + _DAT_005a67dc;
        }
        fVar3 = (float)(int)uVar7;
        if ((int)uVar7 < 0) {
          fVar3 = fVar3 + _DAT_005a67dc;
        }
        uVar10 = uVar7 + 1;
        local_a4[uVar7 + 0x19] = (fVar3 * local_a4[0x19] + fVar2 * fVar9) * _DAT_005a8630;
        uVar7 = uVar10;
      } while (uVar10 < 5);
      local_14 = &DAT_005a8654;
      local_a4[0x1e] = 0.0;
      local_a4[0x1f] = 1.0;
      local_a4[0x18] = fVar9;
    }
    param_1 = (char *)(iVar6 + -1);
    local_1c = (float)(int)param_1;
    if ((int)param_1 < 0) {
      local_1c = local_1c + _DAT_005a67dc;
    }
    if (local_a4[0x18] == local_a4[0x19]) {
      local_18 = 0.0;
    }
    else {
      local_18 = local_1c / (local_a4[0x19] - local_a4[0x18]);
    }
    pfVar11 = local_a4;
    for (iVar6 = 0x10; iVar6 != 0; iVar6 = iVar6 + -1) {
      *pfVar11 = 0.0;
      pfVar11 = pfVar11 + 1;
    }
    FUN_005759c5();
    local_c = pfVar8;
    fVar9 = 0.0;
    do {
      uVar7 = 0;
      param_1._2_1_ = '\0';
      local_8 = (float)((int)fVar9 + 8U);
      if ((uint)fVar9 < (int)fVar9 + 8U) {
        local_10 = local_c;
        do {
          fVar2 = local_a4[(int)fVar9] + *local_10;
          fVar3 = (fVar2 - local_a4[0x18]) * local_18;
          if (fVar3 < _DAT_005a67ec == (fVar3 == _DAT_005a67ec)) {
            if (fVar3 < local_1c) {
              local_20 = fVar3 + _DAT_005a67d4;
              local_24 = (int)ROUND(fVar3 + _DAT_005a67d4);
              iVar6 = *(int *)(local_14 + local_24 * 4);
            }
            else if ((param_2 == 6) &&
                    (fVar3 = (local_a4[0x19] + _DAT_005a67d8) * _DAT_005a67d4,
                    fVar3 < fVar2 != (fVar3 == fVar2))) {
              iVar6 = 7;
            }
            else {
              iVar6 = 1;
            }
          }
          else if ((param_2 != 6) || (local_a4[0x18] * _DAT_005a67d4 < fVar2)) {
            iVar6 = 0;
          }
          else {
            iVar6 = 6;
          }
          fVar2 = fVar2 - local_a4[iVar6 + 0x18];
          uVar7 = uVar7 >> 3 | iVar6 << 0x15;
          uVar10 = (uint)fVar9 & 3;
          if (uVar10 != 3) {
            local_a4[(int)fVar9 + 1] = _DAT_005a862c * fVar2 + local_a4[(int)fVar9 + 1];
          }
          if ((uint)fVar9 < 0xc) {
            if (uVar10 != 0) {
              local_a4[(int)fVar9 + 3] = _DAT_005a8628 * fVar2 + local_a4[(int)fVar9 + 3];
            }
            local_a4[(int)fVar9 + 4] = _DAT_005a8624 * fVar2 + local_a4[(int)fVar9 + 4];
            if (uVar10 != 3) {
              local_a4[(int)fVar9 + 5] = fVar2 * _DAT_005a69c0 + local_a4[(int)fVar9 + 5];
            }
          }
          local_10 = local_10 + 4;
          fVar9 = (float)((int)fVar9 + 1);
        } while ((uint)fVar9 < (uint)local_8);
        param_1._2_1_ = (char)(uVar7 >> 0x10);
      }
      local_c = local_c + 0x20;
      pcVar1[4] = param_1._2_1_;
      pcVar1[2] = (char)uVar7;
      pcVar1[3] = (char)(uVar7 >> 8);
      fVar9 = local_8;
      pcVar1 = pcVar1 + 3;
    } while ((uint)local_8 < 0x10);
  }
  return 0;
}

