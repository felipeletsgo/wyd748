// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00585742 | Name: FUN_00585742


undefined1 FUN_00585742(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  char cVar4;
  uint uVar5;
  int iVar6;
  undefined4 uVar7;
  uint uVar8;
  int iVar9;
  int iVar10;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  int local_4c;
  int local_40;
  int local_3c;
  int local_38;
  int local_30 [4];
  uint local_20;
  undefined2 *local_1c;
  int local_18;
  uint *local_14;
  int local_10;
  int local_c;
  uint local_8;
  
  iVar2 = *(int *)(param_1 + 0x194);
  if (((*(int *)(param_1 + 0xfc) != 0) && (*(int *)(iVar2 + 0x30) == 0)) &&
     (cVar4 = FUN_005856e4(param_1), cVar4 == '\0')) {
    return 0;
  }
  local_58 = **(undefined4 **)(param_1 + 0x14);
  local_54 = (*(undefined4 **)(param_1 + 0x14))[1];
  local_10 = 0;
  local_38 = iVar2 + 0x18;
  local_30[0] = *(int *)(iVar2 + 0x20);
  local_30[1] = *(undefined4 *)(iVar2 + 0x24);
  local_50 = *(undefined4 *)(param_1 + 0x178);
  local_30[2] = *(undefined4 *)(iVar2 + 0x28);
  iVar9 = *(int *)(iVar2 + 0x10);
  iVar6 = *(int *)(iVar2 + 0x14);
  local_3c = param_1;
  local_30[3] = *(undefined4 *)(iVar2 + 0x2c);
  if (0 < *(int *)(param_1 + 0x13c)) {
    local_14 = (uint *)(param_1 + 0x140);
    do {
      local_1c = *(undefined2 **)(param_2 + local_10 * 4);
      iVar10 = *(int *)(param_1 + 0x124 + *local_14 * 4);
      local_8 = *(int *)(iVar2 + 0x34 + *(int *)(iVar10 + 0x14) * 4);
      local_20 = *local_14;
      local_18 = iVar10;
      local_c = *(int *)(iVar2 + 0x44 + *(int *)(iVar10 + 0x18) * 4);
      if (iVar6 < 8) {
        cVar4 = FUN_00585510(&local_58,iVar9,iVar6,0);
        if (cVar4 == '\0') {
          return 0;
        }
        iVar6 = local_40;
        iVar9 = local_4c;
        if (7 < local_40) goto LAB_0058581e;
        uVar7 = 1;
LAB_0058584b:
        uVar5 = FUN_005855fa(&local_58,iVar9,iVar6,local_8,uVar7);
        iVar6 = local_40;
        iVar9 = local_4c;
        if ((int)uVar5 < 0) {
          return 0;
        }
      }
      else {
LAB_0058581e:
        uVar5 = iVar9 >> ((char)iVar6 - 8U & 0x1f) & 0xff;
        iVar3 = *(int *)(local_8 + 0xd4 + uVar5 * 4);
        if (iVar3 == 0) {
          uVar7 = 9;
          goto LAB_0058584b;
        }
        uVar5 = (uint)*(byte *)(uVar5 + 0x4d4 + local_8);
        iVar6 = iVar6 - iVar3;
      }
      uVar8 = 0;
      if (uVar5 != 0) {
        if ((iVar6 < (int)uVar5) &&
           (cVar4 = FUN_00585510(&local_58,iVar9,iVar6,uVar5), iVar6 = local_40, iVar9 = local_4c,
           cVar4 == '\0')) {
          return 0;
        }
        iVar6 = iVar6 - uVar5;
        local_8 = (1 << ((byte)uVar5 & 0x1f)) - 1;
        uVar8 = local_8 & iVar9 >> ((byte)iVar6 & 0x1f);
        iVar10 = local_18;
        if ((int)uVar8 < *(int *)(&DAT_005a93a8 + uVar5 * 4)) {
          uVar8 = *(int *)(&DAT_005a93e8 + uVar5 * 4) + uVar8;
        }
      }
      if (*(char *)(iVar10 + 0x30) == '\0') {
LAB_005859fa:
        local_8 = 1;
        do {
          if (iVar6 < 8) {
            cVar4 = FUN_00585510(&local_58,iVar9,iVar6,0);
            if (cVar4 == '\0') {
              return 0;
            }
            iVar6 = local_40;
            iVar9 = local_4c;
            if (7 < local_40) goto LAB_00585a31;
            uVar7 = 1;
LAB_00585a5b:
            uVar5 = FUN_005855fa(&local_58,iVar9,iVar6,local_c,uVar7);
            iVar6 = local_40;
            iVar9 = local_4c;
            if ((int)uVar5 < 0) {
              return 0;
            }
          }
          else {
LAB_00585a31:
            uVar5 = iVar9 >> ((char)iVar6 - 8U & 0x1f) & 0xff;
            iVar10 = *(int *)(local_c + 0xd4 + uVar5 * 4);
            if (iVar10 == 0) {
              uVar7 = 9;
              goto LAB_00585a5b;
            }
            uVar5 = (uint)*(byte *)(uVar5 + 0x4d4 + local_c);
            iVar6 = iVar6 - iVar10;
          }
          uVar8 = uVar5 & 0xf;
          if (uVar8 == 0) {
            if ((int)uVar5 >> 4 != 0xf) break;
            local_8 = local_8 + 0xf;
          }
          else {
            local_8 = local_8 + ((int)uVar5 >> 4);
            if ((iVar6 < (int)uVar8) &&
               (cVar4 = FUN_00585510(&local_58,iVar9,iVar6,uVar8), iVar6 = local_40,
               iVar9 = local_4c, cVar4 == '\0')) {
              return 0;
            }
            iVar6 = iVar6 - uVar8;
          }
          local_8 = local_8 + 1;
        } while ((int)local_8 < 0x40);
      }
      else {
        piVar1 = local_30 + local_20;
        *piVar1 = *piVar1 + uVar8;
        *local_1c = (short)*piVar1;
        if (*(int *)(iVar10 + 0x24) < 2) goto LAB_005859fa;
        local_8 = 1;
        do {
          if (iVar6 < 8) {
            cVar4 = FUN_00585510(&local_58,iVar9,iVar6,0);
            if (cVar4 == '\0') {
              return 0;
            }
            iVar6 = local_40;
            iVar9 = local_4c;
            if (7 < local_40) goto LAB_00585922;
            uVar7 = 1;
LAB_0058594c:
            uVar5 = FUN_005855fa(&local_58,iVar9,iVar6,local_c,uVar7);
            iVar6 = local_40;
            iVar9 = local_4c;
            if ((int)uVar5 < 0) {
              return 0;
            }
          }
          else {
LAB_00585922:
            uVar5 = iVar9 >> ((char)iVar6 - 8U & 0x1f) & 0xff;
            iVar10 = *(int *)(local_c + 0xd4 + uVar5 * 4);
            if (iVar10 == 0) {
              uVar7 = 9;
              goto LAB_0058594c;
            }
            uVar5 = (uint)*(byte *)(uVar5 + 0x4d4 + local_c);
            iVar6 = iVar6 - iVar10;
          }
          uVar8 = uVar5 & 0xf;
          local_20 = uVar8;
          if (uVar8 == 0) {
            if ((int)uVar5 >> 4 != 0xf) break;
            local_8 = local_8 + 0xf;
          }
          else {
            local_8 = local_8 + ((int)uVar5 >> 4);
            if ((iVar6 < (int)uVar8) &&
               (cVar4 = FUN_00585510(&local_58,iVar9,iVar6,uVar8), iVar6 = local_40,
               iVar9 = local_4c, cVar4 == '\0')) {
              return 0;
            }
            iVar6 = iVar6 - uVar8;
            uVar5 = (1 << (sbyte)uVar8) - 1U & iVar9 >> ((byte)iVar6 & 0x1f);
            if ((int)uVar5 < *(int *)(&DAT_005a93a8 + local_20 * 4)) {
              uVar5 = *(int *)(&DAT_005a93e8 + local_20 * 4) + uVar5;
            }
            local_1c[(&DAT_005a88d0)[local_8]] = (short)uVar5;
          }
          local_8 = local_8 + 1;
        } while ((int)local_8 < 0x40);
      }
      local_10 = local_10 + 1;
      local_14 = local_14 + 1;
    } while (local_10 < *(int *)(param_1 + 0x13c));
  }
  **(undefined4 **)(param_1 + 0x14) = local_58;
  *(undefined4 *)(*(int *)(param_1 + 0x14) + 4) = local_54;
  *(undefined4 *)(param_1 + 0x178) = local_50;
  *(int *)(iVar2 + 0x30) = *(int *)(iVar2 + 0x30) + -1;
  *(int *)(iVar2 + 0x20) = local_30[0];
  *(int *)(iVar2 + 0x24) = local_30[1];
  *(int *)(iVar2 + 0x28) = local_30[2];
  *(int *)(iVar2 + 0x14) = iVar6;
  *(int *)(iVar2 + 0x10) = iVar9;
  *(int *)(iVar2 + 0x2c) = local_30[3];
  return 1;
}

