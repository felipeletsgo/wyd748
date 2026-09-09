// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00579da5 | Name: FUN_00579da5


void FUN_00579da5(uint *param_1,int *param_2,int param_3)

{
  byte bVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  int *piVar6;
  int *piVar7;
  int *piVar8;
  byte *pbVar9;
  byte *pbVar10;
  int local_1c [4];
  uint local_c;
  uint local_8;
  
  if ((param_2 != (int *)0x0) && (param_1 != (uint *)0x0)) {
    local_1c[3] = *(int *)(&DAT_005a8abc + param_3 * 4);
    uVar4 = *param_1;
    uVar5 = uVar4 * local_1c[3];
    bVar1 = *(byte *)((int)param_1 + 0xb);
    if (bVar1 == 1) {
      local_1c[2] = 0;
      pbVar10 = (byte *)((uVar4 - 1 >> 3) + (int)param_2);
      pbVar9 = (byte *)((uVar5 - 1 >> 3) + (int)param_2);
      local_c = 7 - (uVar4 - 1 & 7);
      local_8 = 7 - (uVar5 - 1 & 7);
      if (uVar4 != 0) {
        do {
          bVar1 = *pbVar10;
          local_1c[1] = local_1c[3];
          if (0 < local_1c[3]) {
            do {
              *pbVar9 = (byte)(0x7f7f >> (7 - (byte)local_8 & 0x1f)) & *pbVar9 |
                        (bVar1 >> ((byte)local_c & 0x1f) & 1) << ((byte)local_8 & 0x1f);
              if (local_8 == 7) {
                local_8 = 0;
                pbVar9 = pbVar9 + -1;
              }
              else {
                local_8 = local_8 + 1;
              }
              local_1c[1] = local_1c[1] + -1;
            } while (local_1c[1] != 0);
          }
          if (local_c == 7) {
            local_c = 0;
            pbVar10 = pbVar10 + -1;
          }
          else {
            local_c = local_c + 1;
          }
          local_1c[2] = local_1c[2] + 1;
        } while ((uint)local_1c[2] < *param_1);
      }
    }
    else if (bVar1 == 2) {
      local_1c[2] = 0;
      pbVar10 = (byte *)((uVar4 - 1 >> 2) + (int)param_2);
      pbVar9 = (byte *)((uVar5 - 1 >> 2) + (int)param_2);
      local_c = (3 - (uVar4 - 1 & 3)) * 2;
      local_8 = (3 - (uVar5 - 1 & 3)) * 2;
      if (uVar4 != 0) {
        do {
          bVar1 = *pbVar10;
          local_1c[1] = local_1c[3];
          if (0 < local_1c[3]) {
            do {
              *pbVar9 = (byte)(0x3f3f >> (6 - (byte)local_8 & 0x1f)) & *pbVar9 |
                        (bVar1 >> ((byte)local_c & 0x1f) & 3) << ((byte)local_8 & 0x1f);
              if (local_8 == 6) {
                local_8 = 0;
                pbVar9 = pbVar9 + -1;
              }
              else {
                local_8 = local_8 + 2;
              }
              local_1c[1] = local_1c[1] + -1;
            } while (local_1c[1] != 0);
          }
          if (local_c == 6) {
            local_c = 0;
            pbVar10 = pbVar10 + -1;
          }
          else {
            local_c = local_c + 2;
          }
          local_1c[2] = local_1c[2] + 1;
        } while ((uint)local_1c[2] < *param_1);
      }
    }
    else if (bVar1 == 4) {
      local_1c[2] = 0;
      pbVar10 = (byte *)((uVar4 - 1 >> 1) + (int)param_2);
      pbVar9 = (byte *)((uVar5 - 1 >> 1) + (int)param_2);
      local_c = (uVar4 - 1 & 1) * -4 + 4;
      local_8 = (uVar5 - 1 & 1) * -4 + 4;
      if (uVar4 != 0) {
        do {
          bVar1 = *pbVar10;
          local_1c[1] = local_1c[3];
          if (0 < local_1c[3]) {
            do {
              *pbVar9 = (byte)(0xf0f >> (4 - (byte)local_8 & 0x1f)) & *pbVar9 |
                        (bVar1 >> ((byte)local_c & 0x1f) & 0xf) << ((byte)local_8 & 0x1f);
              if (local_8 == 4) {
                local_8 = 0;
                pbVar9 = pbVar9 + -1;
              }
              else {
                local_8 = local_8 + 4;
              }
              local_1c[1] = local_1c[1] + -1;
            } while (local_1c[1] != 0);
          }
          if (local_c == 4) {
            local_c = 0;
            pbVar10 = pbVar10 + -1;
          }
          else {
            local_c = local_c + 4;
          }
          local_1c[2] = local_1c[2] + 1;
        } while ((uint)local_1c[2] < *param_1);
      }
    }
    else {
      local_8 = 0;
      local_c = (uint)(bVar1 >> 3);
      piVar6 = (int *)((uVar4 - 1) * local_c + (int)param_2);
      param_2 = (int *)((uVar5 - 1) * local_c + (int)param_2);
      if (uVar4 != 0) {
        do {
          uVar4 = local_c;
          piVar7 = piVar6;
          piVar8 = local_1c;
          for (uVar3 = local_c >> 2; iVar2 = local_1c[3], uVar3 != 0; uVar3 = uVar3 - 1) {
            *piVar8 = *piVar7;
            piVar7 = piVar7 + 1;
            piVar8 = piVar8 + 1;
          }
          for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
            *(char *)piVar8 = (char)*piVar7;
            piVar7 = (int *)((int)piVar7 + 1);
            piVar8 = (int *)((int)piVar8 + 1);
          }
          if (0 < iVar2) {
            local_1c[2] = iVar2;
            do {
              piVar7 = (int *)((int)param_2 - local_c);
              piVar8 = local_1c;
              for (uVar4 = local_c >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
                *param_2 = *piVar8;
                piVar8 = piVar8 + 1;
                param_2 = param_2 + 1;
              }
              local_1c[2] = local_1c[2] + -1;
              for (uVar4 = local_c & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
                *(char *)param_2 = (char)*piVar8;
                piVar8 = (int *)((int)piVar8 + 1);
                param_2 = (int *)((int)param_2 + 1);
              }
              param_2 = piVar7;
            } while (local_1c[2] != 0);
          }
          piVar6 = (int *)((int)piVar6 - local_c);
          local_8 = local_8 + 1;
        } while (local_8 < *param_1);
      }
    }
    *param_1 = uVar5;
    param_1[1] = *(byte *)((int)param_1 + 0xb) * uVar5 + 7 >> 3;
  }
  return;
}

