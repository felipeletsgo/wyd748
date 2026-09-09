// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058c34a | Name: FUN_0058c34a


undefined4
FUN_0058c34a(uint *param_1,uint param_2,uint param_3,int param_4,int param_5,int *param_6,
            int param_7,uint *param_8,uint *param_9)

{
  undefined4 uVar1;
  uint *puVar2;
  uint *in_EAX;
  int iVar3;
  uint uVar4;
  uint uVar5;
  int *piVar6;
  int iVar7;
  uint uVar8;
  undefined4 *puVar9;
  uint uVar10;
  uint uVar11;
  int iVar12;
  byte bVar13;
  int iVar14;
  int local_fc [15];
  uint local_c0 [16];
  uint local_80 [17];
  int local_3c;
  int local_38;
  int local_34;
  uint local_30;
  int local_2c;
  uint *local_28;
  uint local_24;
  uint local_20;
  undefined4 local_1c;
  uint local_18;
  uint local_14;
  int local_10;
  uint local_c;
  uint local_8;
  
  local_80[0] = 0;
  local_80[1] = 0;
  local_80[2] = 0;
  local_80[3] = 0;
  local_80[4] = 0;
  local_80[5] = 0;
  local_80[6] = 0;
  local_80[7] = 0;
  local_80[8] = 0;
  local_80[9] = 0;
  local_80[10] = 0;
  local_80[0xb] = 0;
  local_80[0xc] = 0;
  local_80[0xd] = 0;
  local_80[0xe] = 0;
  local_80[0xf] = 0;
  piVar6 = (int *)param_1;
  uVar10 = param_2;
  do {
    local_80[*piVar6] = local_80[*piVar6] + 1;
    piVar6 = piVar6 + 1;
    uVar10 = uVar10 - 1;
  } while (uVar10 != 0);
  if (local_80[0] == param_2) {
    *param_6 = 0;
    *in_EAX = 0;
  }
  else {
    uVar11 = 1;
    local_8 = *in_EAX;
    uVar10 = 0xf;
    do {
      if (local_80[uVar11] != 0) break;
      uVar11 = uVar11 + 1;
    } while (uVar11 < 0x10);
    local_c = uVar11;
    if (*in_EAX < uVar11) {
      local_8 = uVar11;
    }
    do {
      if (local_80[uVar10] != 0) break;
      uVar10 = uVar10 - 1;
    } while (uVar10 != 0);
    local_30 = uVar10;
    if (uVar10 < local_8) {
      local_8 = uVar10;
    }
    *in_EAX = local_8;
    iVar12 = 1 << ((byte)uVar11 & 0x1f);
    for (; uVar11 < uVar10; uVar11 = uVar11 + 1) {
      if ((int)(iVar12 - local_80[uVar11]) < 0) {
        return 0xfffffffd;
      }
      iVar12 = (iVar12 - local_80[uVar11]) * 2;
    }
    uVar11 = local_80[uVar10];
    iVar12 = iVar12 - uVar11;
    local_38 = iVar12;
    if (iVar12 < 0) {
      return 0xfffffffd;
    }
    local_80[uVar10] = uVar11 + iVar12;
    iVar7 = 0;
    iVar3 = uVar10 - 1;
    local_c0[1] = 0;
    if (iVar3 != 0) {
      iVar14 = 0;
      do {
        iVar7 = iVar7 + *(int *)((int)local_80 + iVar14 + 4);
        iVar3 = iVar3 + -1;
        *(int *)((int)local_c0 + iVar14 + 8) = iVar7;
        iVar14 = iVar14 + 4;
      } while (iVar3 != 0);
    }
    uVar11 = 0;
    do {
      iVar7 = local_38;
      iVar3 = *param_1;
      param_1 = param_1 + 1;
      if (iVar3 != 0) {
        uVar8 = local_c0[iVar3];
        param_9[uVar8] = uVar11;
        local_c0[iVar3] = uVar8 + 1;
        iVar12 = iVar7;
      }
      uVar11 = uVar11 + 1;
    } while (uVar11 < param_2);
    uVar10 = local_c0[uVar10];
    local_10 = -1;
    param_1 = param_9;
    iVar3 = -local_8;
    local_24 = 0;
    local_c0[0] = 0;
    local_fc[0] = 0;
    local_2c = 0;
    local_14 = 0;
    if ((int)local_c <= (int)local_30) {
      local_34 = local_c - 1;
      local_28 = local_80 + local_c;
      do {
        local_20 = *local_28;
        uVar1 = local_1c;
        while (local_1c = uVar1, local_20 != 0) {
          local_1c._2_2_ = (undefined2)((uint)uVar1 >> 0x10);
          local_20 = local_20 - 1;
          local_3c = local_8 + iVar3;
          if (local_3c < (int)local_c) {
            local_80[0x10] = iVar3 - local_8;
            do {
              iVar12 = local_10;
              local_10 = local_10 + 1;
              iVar3 = iVar3 + local_8;
              local_80[0x10] = local_80[0x10] + local_8;
              local_3c = local_3c + local_8;
              uVar11 = local_30 - iVar3;
              if (local_8 < local_30 - iVar3) {
                uVar11 = local_8;
              }
              uVar8 = local_c - iVar3;
              uVar4 = 1 << ((byte)uVar8 & 0x1f);
              if ((local_20 + 1 < uVar4) &&
                 (iVar7 = uVar4 + (-1 - local_20), puVar2 = local_28, uVar8 < uVar11)) {
                while (uVar8 = uVar8 + 1, uVar8 < uVar11) {
                  uVar4 = puVar2[1];
                  uVar5 = iVar7 * 2;
                  if (uVar5 < uVar4 || uVar5 - uVar4 == 0) break;
                  iVar7 = uVar5 - uVar4;
                  puVar2 = puVar2 + 1;
                }
              }
              local_14 = 1 << ((byte)uVar8 & 0x1f);
              uVar11 = *param_8 + local_14;
              if (0x5a0 < uVar11) {
                return 0xfffffffc;
              }
              local_2c = param_7 + *param_8 * 8;
              local_fc[local_10] = local_2c;
              uVar4 = local_24;
              *param_8 = uVar11;
              if (local_10 == 0) {
                *param_6 = local_2c;
              }
              else {
                local_c0[local_10] = local_24;
                local_1c = CONCAT31(CONCAT21(local_1c._2_2_,(undefined1)local_8),(byte)uVar8);
                uVar4 = uVar4 >> ((byte)local_80[0x10] & 0x1f);
                iVar12 = local_fc[iVar12];
                local_18 = (local_2c - iVar12 >> 3) - uVar4;
                *(undefined4 *)(iVar12 + uVar4 * 8) = local_1c;
                *(uint *)(iVar12 + 4 + uVar4 * 8) = local_18;
              }
              iVar12 = local_38;
            } while (local_3c < (int)local_c);
          }
          bVar13 = (byte)iVar3;
          if (param_1 < param_9 + uVar10) {
            local_18 = *param_1;
            if (local_18 < param_3) {
              local_1c._0_1_ = (local_18 < 0x100) - 1U & 0x60;
            }
            else {
              iVar7 = (local_18 - param_3) * 4;
              local_1c._0_1_ = *(char *)(iVar7 + param_5) + 0x50;
              local_18 = *(uint *)(iVar7 + param_4);
            }
            param_1 = param_1 + 1;
          }
          else {
            local_1c._0_1_ = 0xc0;
          }
          local_1c = CONCAT31(CONCAT21(local_1c._2_2_,(char)local_c - bVar13),(byte)local_1c);
          iVar7 = 1 << ((char)local_c - bVar13 & 0x1f);
          uVar11 = local_24 >> (bVar13 & 0x1f);
          if (uVar11 < local_14) {
            puVar9 = (undefined4 *)(local_2c + uVar11 * 8);
            do {
              *puVar9 = local_1c;
              puVar9[1] = local_18;
              uVar11 = uVar11 + iVar7;
              puVar9 = puVar9 + iVar7 * 2;
              iVar12 = local_38;
            } while (uVar11 < local_14);
          }
          uVar11 = 1 << ((byte)local_34 & 0x1f);
          while ((local_24 & uVar11) != 0) {
            local_24 = local_24 ^ uVar11;
            uVar11 = uVar11 >> 1;
          }
          local_24 = local_24 ^ uVar11;
          uVar1 = local_1c;
          if (((1 << (bVar13 & 0x1f)) - 1U & local_24) != local_c0[local_10]) {
            do {
              iVar3 = iVar3 - local_8;
              local_10 = local_10 + -1;
            } while (((1 << ((byte)iVar3 & 0x1f)) - 1U & local_24) != local_c0[local_10]);
          }
        }
        local_c = local_c + 1;
        local_28 = local_28 + 1;
        local_34 = local_34 + 1;
      } while ((int)local_c <= (int)local_30);
    }
    if ((iVar12 != 0) && (local_30 != 1)) {
      return 0xfffffffb;
    }
  }
  return 0;
}

