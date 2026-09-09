// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00565891 | Name: FUN_00565891


undefined4 __thiscall FUN_00565891(int *param_1,uint *param_2,uint param_3)

{
  undefined1 *puVar1;
  undefined1 *puVar2;
  byte bVar3;
  ushort uVar4;
  uint uVar5;
  byte *pbVar6;
  byte bVar7;
  uint uVar8;
  uint uVar9;
  uint uVar10;
  undefined4 *puVar11;
  uint *puVar12;
  uint uVar13;
  int iVar14;
  uint uVar15;
  uint *puVar16;
  uint *puVar17;
  bool bVar18;
  bool bVar19;
  uint local_20;
  uint local_1c;
  uint local_18;
  int local_10;
  uint *local_8;
  
  pbVar6 = (byte *)param_2;
  if (param_3 < 0x12) {
    return 0x80004005;
  }
  if (((((*(byte *)((int)param_2 + 1) & 0xfe) != 0) ||
       (bVar3 = *(byte *)((int)param_2 + 2), (bVar3 & 0xf4) != 0)) ||
      (*(short *)((int)param_2 + 0xc) == 0)) || (*(short *)((int)param_2 + 0xe) == 0)) {
    return 0x80004005;
  }
  uVar8 = (uint)*(byte *)((int)param_2 + 7);
  local_18 = 0;
  if (*(byte *)((int)param_2 + 1) != 0) {
    if (uVar8 == 0xf) {
      local_18 = 0x18;
    }
    else if (uVar8 == 0x10) {
      local_18 = 0x19;
    }
    else if (uVar8 == 0x18) {
      local_18 = 0x14;
    }
    else {
      if (uVar8 != 0x20) {
        return 0x80004005;
      }
      local_18 = 0x15;
    }
  }
  uVar13 = (uint)*(byte *)((int)param_2 + 0x10);
  uVar9 = uVar13 + 7 >> 3;
  bVar7 = bVar3 & 3;
  if (bVar7 == 1) {
    if (*(byte *)((int)param_2 + 1) == 0) {
      return 0x80004005;
    }
    if (*(byte *)((int)param_2 + 0x10) != 8) {
      return 0x80004005;
    }
    local_10 = 0x29;
  }
  else if (bVar7 == 2) {
    if (uVar13 == 0xf) {
      local_10 = 0x18;
    }
    else if (uVar13 == 0x10) {
      local_10 = 0x19;
    }
    else if (uVar13 == 0x18) {
      local_10 = 0x14;
    }
    else {
      if (uVar13 != 0x20) {
        return 0x80004005;
      }
      local_10 = 0x15;
    }
  }
  else {
    if (bVar7 != 3) {
      return 0x80004005;
    }
    if (*(byte *)((int)param_2 + 0x10) != 8) {
      return 0x80004005;
    }
    local_10 = 0x32;
  }
  bVar18 = (*(byte *)((int)param_2 + 0x11) & 0x20) != 0x20;
  bVar19 = (*(byte *)((int)param_2 + 0x11) & 0x10) == 0x10;
  uVar10 = (uint)*(byte *)param_2;
  if (param_3 - 0x12 < uVar10) {
    return 0x80004005;
  }
  param_3 = (param_3 - 0x12) - uVar10;
  local_8 = (uint *)((int)param_2 + uVar10 + 0x12);
  uVar8 = (uint)*(ushort *)((int)param_2 + 5) * (uVar8 + 7 >> 3);
  if (param_3 < uVar8) {
    return 0x80004005;
  }
  if ((param_1[0x10] != 0) && (local_10 == 0x29)) {
    if (0x100 < (uint)*(ushort *)((int)param_2 + 3) + (uint)*(ushort *)((int)param_2 + 5)) {
      return 0x80004005;
    }
    puVar11 = operator_new(0x400);
    param_1[2] = (int)puVar11;
    if (puVar11 == (undefined4 *)0x0) {
      return 0x8007000e;
    }
    param_1[0xf] = 1;
    for (iVar14 = 0x100; iVar14 != 0; iVar14 = iVar14 + -1) {
      *puVar11 = 0xffffffff;
      puVar11 = puVar11 + 1;
    }
    puVar1 = (undefined1 *)(param_1[2] + (uint)*(ushort *)((int)param_2 + 3) * 4);
    puVar2 = puVar1 + (uint)*(ushort *)((int)param_2 + 5) * 4;
    uVar10 = param_3;
    uVar15 = param_3;
    param_2 = local_8;
    for (; puVar1 < puVar2; puVar1 = puVar1 + 4) {
      if (local_18 == 0x14) {
        uVar10 = (uint)*(byte *)((int)param_2 + 2);
        uVar15 = (uint)*(byte *)((int)param_2 + 1);
        local_1c = (uint)(byte)*param_2;
        param_2 = (uint *)((int)param_2 + 3);
LAB_00565b6e:
        local_20 = 0xff;
      }
      else if (local_18 == 0x15) {
        uVar5 = *param_2;
        local_20 = uVar5 >> 0x18;
        uVar15 = uVar5 >> 8 & 0xff;
        uVar10 = uVar5 >> 0x10 & 0xff;
        local_1c = uVar5 & 0xff;
        param_2 = param_2 + 1;
      }
      else {
        if (local_18 == 0x18) {
          uVar4 = (ushort)*param_2;
          uVar10 = uVar4 >> 10 & 0x1f;
          uVar15 = uVar4 >> 5 & 0x1f;
          uVar10 = uVar10 >> 2 | uVar10 << 3;
          uVar15 = uVar15 >> 2 | uVar15 << 3;
          local_1c = (uVar4 & 0x1f) >> 2 | (uVar4 & 0x1f) << 3;
          param_2 = (uint *)((int)param_2 + 2);
          goto LAB_00565b6e;
        }
        if (local_18 == 0x19) {
          uVar4 = (ushort)*param_2;
          local_20 = (uint)(uVar4 >> 0xf) * 0xff;
          uVar10 = uVar4 >> 10 & 0x1f;
          uVar15 = uVar4 >> 5 & 0x1f;
          uVar10 = uVar10 >> 2 | uVar10 << 3;
          uVar15 = uVar15 >> 2 | uVar15 << 3;
          local_1c = (uVar4 & 0x1f) >> 2 | (uVar4 & 0x1f) << 3;
          param_2 = (uint *)((int)param_2 + 2);
        }
      }
      puVar1[2] = (char)local_1c;
      puVar1[3] = (undefined1)local_20;
      *puVar1 = (char)uVar10;
      puVar1[1] = (char)uVar15;
    }
  }
  param_3 = param_3 - uVar8;
  local_8 = (uint *)((int)local_8 + uVar8);
  uVar8 = (uint)*(ushort *)(pbVar6 + 0xe) * (uint)*(ushort *)(pbVar6 + 0xc) * uVar9;
  *param_1 = local_10;
  param_1[1] = (int)local_8;
  param_1[0xc] = *(ushort *)(pbVar6 + 0xc) * uVar9;
  param_1[0xd] = 0;
  param_1[3] = (uint)*(ushort *)(pbVar6 + 0xc);
  param_1[4] = (uint)*(ushort *)(pbVar6 + 0xe);
  param_1[5] = 1;
  if (param_1[0x10] != 0) {
    if ((((bVar3 & 8) != 0) || (bVar18)) || (bVar19)) {
      puVar12 = operator_new(uVar8);
      param_1[1] = (int)puVar12;
      if (puVar12 == (uint *)0x0) {
        return 0x8007000e;
      }
      param_1[0xe] = 1;
      if (bVar18) {
        puVar12 = (uint *)((*(ushort *)(pbVar6 + 0xe) - 1) * param_1[0xc] + (int)puVar12);
      }
      local_18 = 0;
      if (*(short *)(pbVar6 + 0xe) != 0) {
        do {
          param_2 = puVar12;
          if (bVar19) {
            param_2 = (uint *)((param_1[0xc] - uVar9) + (int)puVar12);
          }
          uVar8 = (uint)*(ushort *)(pbVar6 + 0xc);
          local_1c = 0;
          if (uVar8 != 0) {
            do {
              if ((bVar3 & 8) == 0) {
                local_20 = 0;
              }
              else {
                if (param_3 == 0) {
                  return 0x80004005;
                }
                local_20 = (byte)*local_8 & 0x80;
                uVar8 = ((byte)*local_8 & 0x7f) + 1;
                local_8 = (uint *)((int)local_8 + 1);
                param_3 = param_3 - 1;
              }
              local_1c = local_1c + uVar8;
              while (uVar8 != 0) {
                uVar8 = uVar8 - 1;
                if (param_3 < uVar9) {
                  return 0x80004005;
                }
                puVar16 = local_8;
                puVar17 = param_2;
                for (uVar10 = uVar13 + 7 >> 5; uVar10 != 0; uVar10 = uVar10 - 1) {
                  *puVar17 = *puVar16;
                  puVar16 = puVar16 + 1;
                  puVar17 = puVar17 + 1;
                }
                for (uVar10 = uVar9 & 3; uVar10 != 0; uVar10 = uVar10 - 1) {
                  *(byte *)puVar17 = (byte)*puVar16;
                  puVar16 = (uint *)((int)puVar16 + 1);
                  puVar17 = (uint *)((int)puVar17 + 1);
                }
                if (local_20 == 0) {
                  local_8 = (uint *)((int)local_8 + uVar9);
                  param_3 = param_3 - uVar9;
                }
                uVar10 = uVar9;
                if (bVar19) {
                  uVar10 = -uVar9;
                }
                param_2 = (uint *)((int)param_2 + uVar10);
              }
              if (local_20 != 0) {
                local_8 = (uint *)((int)local_8 + uVar9);
                param_3 = param_3 - uVar9;
              }
              uVar8 = (uint)*(ushort *)(pbVar6 + 0xc);
            } while (local_1c < uVar8);
          }
          if (bVar18) {
            iVar14 = -param_1[0xc];
          }
          else {
            iVar14 = param_1[0xc];
          }
          puVar12 = (uint *)((int)puVar12 + iVar14);
          local_18 = local_18 + 1;
        } while (local_18 < *(ushort *)(pbVar6 + 0xe));
      }
    }
    else {
      if (param_3 < uVar8) {
        return 0x80004005;
      }
      param_1[1] = (int)local_8;
      param_1[0xe] = 0;
    }
  }
  return 0;
}

