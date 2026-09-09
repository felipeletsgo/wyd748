// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005861db | Name: FUN_005861db


undefined1 FUN_005861db(int *param_1,int *param_2)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  char cVar5;
  undefined4 uVar6;
  uint uVar7;
  int iVar8;
  int aiStack_158 [64];
  undefined4 local_58;
  undefined4 local_54;
  int local_50;
  int local_4c;
  int local_40;
  int *local_3c;
  int local_38;
  uint local_30;
  uint local_2c;
  short *local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  uint local_c;
  int local_8;
  
  piVar4 = param_1;
  local_14 = param_1[0x5b];
  iVar2 = param_1[0x65];
  local_2c = 1 << ((byte)param_1[0x5d] & 0x1f);
  local_30 = -1 << ((byte)param_1[0x5d] & 0x1f);
  if (((param_1[0x3f] != 0) && (*(int *)(iVar2 + 0x34) == 0)) &&
     (cVar5 = FUN_00585c78(), cVar5 == '\0')) {
    return 0;
  }
  local_1c = 0;
  local_3c = param_1;
  local_58 = *(undefined4 *)param_1[5];
  local_54 = ((undefined4 *)param_1[5])[1];
  local_50 = param_1[0x5e];
  local_38 = iVar2 + 0x18;
  iVar8 = *(int *)(iVar2 + 0x14);
  local_18 = *param_2;
  local_10 = *(int *)(iVar2 + 0x20);
  local_20 = *(int *)(iVar2 + 0x48);
  local_8 = param_1[0x5a];
  param_1 = (int *)*(int *)(iVar2 + 0x10);
  if (local_10 == 0) {
    for (; local_8 <= local_14; local_8 = local_8 + 1) {
      if (iVar8 < 8) {
        cVar5 = FUN_00585510(&local_58,param_1,iVar8,0);
        if (cVar5 == '\0') goto LAB_00586541;
        param_1 = (int *)local_4c;
        iVar8 = local_40;
        if (7 < local_40) goto LAB_005862b4;
        uVar6 = 1;
LAB_005862e1:
        local_c = FUN_005855fa(&local_58,param_1,iVar8,local_20,uVar6);
        if ((int)local_c < 0) goto LAB_00586541;
        param_1 = (int *)local_4c;
        iVar8 = local_40;
      }
      else {
LAB_005862b4:
        uVar7 = (int)param_1 >> ((char)iVar8 - 8U & 0x1f) & 0xff;
        iVar3 = *(int *)(local_20 + 0xd4 + uVar7 * 4);
        if (iVar3 == 0) {
          uVar6 = 9;
          goto LAB_005862e1;
        }
        local_c = (uint)*(byte *)(uVar7 + 0x4d4 + local_20);
        iVar8 = iVar8 - iVar3;
      }
      local_24 = (int)local_c >> 4;
      local_c = local_c & 0xf;
      if (local_c == 0) {
        uVar7 = 0;
        if (local_24 != 0xf) {
          local_20 = 1 << ((byte)local_24 & 0x1f);
          local_10 = local_20;
          if (local_24 == 0) goto LAB_00586479;
          if (iVar8 < local_24) {
            cVar5 = FUN_00585510(&local_58,param_1,iVar8,local_24);
            if (cVar5 == '\0') goto LAB_00586541;
            param_1 = (int *)local_4c;
            iVar8 = local_40;
          }
          iVar8 = iVar8 - local_24;
          local_10 = local_10 + ((int)param_1 >> ((byte)iVar8 & 0x1f) & local_20 - 1U);
          goto LAB_00586479;
        }
      }
      else {
        if (local_c != 1) {
          *(undefined4 *)(*piVar4 + 0x14) = 0x72;
          (**(code **)(*piVar4 + 4))(piVar4,0xffffffff);
        }
        if (iVar8 < 1) {
          cVar5 = FUN_00585510(&local_58,param_1,iVar8,1);
          if (cVar5 == '\0') goto LAB_00586541;
          param_1 = (int *)local_4c;
          iVar8 = local_40;
        }
        iVar8 = iVar8 + -1;
        uVar7 = local_2c;
        if (((int)param_1 >> ((byte)iVar8 & 0x1f) & 1U) == 0) {
          uVar7 = local_30;
        }
      }
      do {
        local_c = uVar7;
        local_28 = (short *)(local_18 + (&DAT_005a88d0)[local_8] * 2);
        if (*local_28 == 0) {
          local_24 = local_24 + -1;
          if (local_24 < 0) break;
        }
        else {
          if (iVar8 < 1) {
            cVar5 = FUN_00585510(&local_58,param_1,iVar8,1);
            if (cVar5 == '\0') goto LAB_00586541;
            param_1 = (int *)local_4c;
            iVar8 = local_40;
          }
          iVar8 = iVar8 + -1;
          if ((((int)param_1 >> ((byte)iVar8 & 0x1f) & 1U) != 0) &&
             ((local_2c & (int)*local_28) == 0)) {
            uVar7 = local_2c;
            if (*local_28 < 0) {
              uVar7 = local_30;
            }
            *local_28 = *local_28 + (short)uVar7;
          }
        }
        local_8 = local_8 + 1;
        uVar7 = local_c;
      } while (local_8 <= local_14);
      if (local_c != 0) {
        iVar3 = (&DAT_005a88d0)[local_8];
        *(short *)(local_18 + iVar3 * 2) = (short)local_c;
        piVar1 = aiStack_158 + local_1c;
        local_1c = local_1c + 1;
        *piVar1 = iVar3;
      }
    }
  }
  else {
LAB_00586479:
    if (local_10 != 0) {
      for (; local_8 <= local_14; local_8 = local_8 + 1) {
        local_28 = (short *)(local_18 + (&DAT_005a88d0)[local_8] * 2);
        if (*local_28 != 0) {
          if (iVar8 < 1) {
            cVar5 = FUN_00585510(&local_58,param_1,iVar8,1);
            if (cVar5 == '\0') {
LAB_00586541:
              for (; 0 < local_1c; local_1c = local_1c + -1) {
                *(undefined2 *)(local_18 + aiStack_158[local_1c + -1] * 2) = 0;
              }
              return 0;
            }
            param_1 = (int *)local_4c;
            iVar8 = local_40;
          }
          iVar8 = iVar8 + -1;
          if ((((int)param_1 >> ((byte)iVar8 & 0x1f) & 1U) != 0) &&
             ((local_2c & (int)*local_28) == 0)) {
            uVar7 = local_2c;
            if (*local_28 < 0) {
              uVar7 = local_30;
            }
            *local_28 = *local_28 + (short)uVar7;
          }
        }
      }
      local_10 = local_10 + -1;
    }
  }
  *(undefined4 *)piVar4[5] = local_58;
  *(undefined4 *)(piVar4[5] + 4) = local_54;
  piVar4[0x5e] = local_50;
  *(int *)(iVar2 + 0x34) = *(int *)(iVar2 + 0x34) + -1;
  *(int **)(iVar2 + 0x10) = param_1;
  *(int *)(iVar2 + 0x20) = local_10;
  *(int *)(iVar2 + 0x14) = iVar8;
  return 1;
}

