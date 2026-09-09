// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058e5dd | Name: FUN_0058e5dd


undefined4 FUN_0058e5dd(uint param_1,uint param_2,int param_3,int param_4,int param_5,byte *param_6)

{
  byte bVar1;
  int iVar2;
  int *piVar3;
  byte *pbVar4;
  byte *pbVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  byte *local_14;
  uint local_10;
  byte *local_c;
  uint local_8;
  
  piVar3 = (int *)param_6;
  local_c = *(byte **)param_6;
  local_8 = *(uint *)((int)param_6 + 4);
  param_6 = *(byte **)(param_5 + 0x34);
  uVar9 = *(uint *)(param_5 + 0x1c);
  if (param_6 < *(byte **)(param_5 + 0x30)) {
    local_14 = *(byte **)(param_5 + 0x30) + (-1 - (int)param_6);
  }
  else {
    local_14 = (byte *)(*(int *)(param_5 + 0x2c) - (int)param_6);
  }
  uVar8 = *(uint *)(&DAT_005c98e0 + param_1 * 4);
  uVar6 = *(uint *)(&DAT_005c98e0 + param_2 * 4);
  param_1 = *(uint *)(param_5 + 0x20);
  do {
    for (; uVar9 < 0x14; uVar9 = uVar9 + 8) {
      local_8 = local_8 - 1;
      param_1 = param_1 | (uint)*local_c << ((byte)uVar9 & 0x1f);
      local_c = local_c + 1;
    }
    pbVar4 = (byte *)(param_3 + (uVar8 & param_1) * 8);
    bVar1 = *pbVar4;
LAB_0058e682:
    uVar7 = (uint)bVar1;
    if (uVar7 != 0) {
      param_1 = param_1 >> (pbVar4[1] & 0x1f);
      uVar9 = uVar9 - pbVar4[1];
      if ((bVar1 & 0x10) != 0) {
        uVar7 = uVar7 & 0xf;
        local_10 = (*(uint *)(&DAT_005c98e0 + uVar7 * 4) & param_1) + *(int *)(pbVar4 + 4);
        param_1 = param_1 >> (sbyte)uVar7;
        for (uVar9 = uVar9 - uVar7; uVar9 < 0xf; uVar9 = uVar9 + 8) {
          local_8 = local_8 - 1;
          param_1 = param_1 | (uint)*local_c << ((byte)uVar9 & 0x1f);
          local_c = local_c + 1;
        }
        pbVar4 = (byte *)(param_4 + (uVar6 & param_1) * 8);
        param_1 = param_1 >> (pbVar4[1] & 0x1f);
        uVar9 = uVar9 - pbVar4[1];
        while( true ) {
          bVar1 = *pbVar4;
          if ((bVar1 & 0x10) != 0) {
            uVar7 = bVar1 & 0xf;
            for (; uVar9 < uVar7; uVar9 = uVar9 + 8) {
              local_8 = local_8 - 1;
              param_1 = param_1 | (uint)*local_c << ((byte)uVar9 & 0x1f);
              local_c = local_c + 1;
            }
            param_2 = (*(uint *)(&DAT_005c98e0 + uVar7 * 4) & param_1) + *(int *)(pbVar4 + 4);
            local_14 = local_14 + -local_10;
            param_1 = param_1 >> (sbyte)uVar7;
            uVar9 = uVar9 - uVar7;
            if ((uint)((int)param_6 - *(int *)(param_5 + 0x28)) < param_2) {
              param_2 = (*(int *)(param_5 + 0x28) - (int)param_6) + param_2;
              pbVar5 = (byte *)(*(int *)(param_5 + 0x2c) - param_2);
              pbVar4 = param_6;
              if (param_2 < local_10) {
                local_10 = local_10 - param_2;
                do {
                  bVar1 = *pbVar5;
                  pbVar4 = param_6 + 1;
                  pbVar5 = pbVar5 + 1;
                  param_2 = param_2 - 1;
                  *param_6 = bVar1;
                  param_6 = pbVar4;
                } while (param_2 != 0);
                pbVar5 = *(byte **)(param_5 + 0x28);
              }
            }
            else {
              pbVar5 = param_6 + -param_2;
              *param_6 = *pbVar5;
              param_6[1] = pbVar5[1];
              pbVar5 = pbVar5 + 2;
              local_10 = local_10 - 2;
              pbVar4 = param_6 + 2;
            }
            do {
              *pbVar4 = *pbVar5;
              param_6 = pbVar4 + 1;
              pbVar5 = pbVar5 + 1;
              local_10 = local_10 - 1;
              pbVar4 = param_6;
            } while (local_10 != 0);
            goto LAB_0058e7e1;
          }
          if ((bVar1 & 0x40) != 0) break;
          pbVar4 = pbVar4 + ((*(uint *)(&DAT_005c98e0 + (uint)bVar1 * 4) & param_1) +
                            *(int *)(pbVar4 + 4)) * 8;
          param_1 = param_1 >> (pbVar4[1] & 0x1f);
          uVar9 = uVar9 - pbVar4[1];
        }
        piVar3[6] = (int)"invalid distance code";
        uVar8 = piVar3[1] - local_8;
        if (uVar9 >> 3 < piVar3[1] - local_8) {
          uVar8 = uVar9 >> 3;
        }
LAB_0058e85a:
        param_2 = 0xfffffffd;
        goto LAB_0058e861;
      }
      if ((bVar1 & 0x40) == 0) break;
      uVar6 = uVar9 >> 3;
      if ((bVar1 & 0x20) == 0) {
        uVar8 = piVar3[1] - local_8;
        piVar3[6] = (int)"invalid literal/length code";
        if (uVar6 < uVar8) {
          uVar8 = uVar6;
        }
        goto LAB_0058e85a;
      }
      uVar8 = piVar3[1] - local_8;
      if (uVar6 < uVar8) {
        uVar8 = uVar6;
      }
      param_2 = 1;
      goto LAB_0058e861;
    }
    param_1 = param_1 >> (pbVar4[1] & 0x1f);
    uVar9 = uVar9 - pbVar4[1];
    local_14 = local_14 + -1;
    *param_6 = pbVar4[4];
    param_6 = param_6 + 1;
LAB_0058e7e1:
    if ((local_14 < (byte *)0x102) || (local_8 < 10)) {
      uVar8 = piVar3[1] - local_8;
      if (uVar9 >> 3 < piVar3[1] - local_8) {
        uVar8 = uVar9 >> 3;
      }
      param_2 = 0;
LAB_0058e861:
      *(uint *)(param_5 + 0x20) = param_1;
      *(uint *)(param_5 + 0x1c) = uVar9 + uVar8 * -8;
      piVar3[1] = uVar8 + local_8;
      iVar2 = *piVar3;
      *piVar3 = (int)local_c - uVar8;
      piVar3[2] = piVar3[2] + (((int)local_c - uVar8) - iVar2);
      *(byte **)(param_5 + 0x34) = param_6;
      return param_2;
    }
  } while( true );
  pbVar4 = pbVar4 + ((*(uint *)(&DAT_005c98e0 + uVar7 * 4) & param_1) + *(int *)(pbVar4 + 4)) * 8;
  bVar1 = *pbVar4;
  goto LAB_0058e682;
}

