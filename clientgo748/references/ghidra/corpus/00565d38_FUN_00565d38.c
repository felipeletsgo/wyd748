// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00565d38 | Name: FUN_00565d38


uint __thiscall FUN_00565d38(undefined4 *param_1,uint *param_2,uint param_3)

{
  byte bVar1;
  bool bVar2;
  byte *pbVar3;
  uint uVar4;
  uint uVar5;
  int iVar6;
  uint uVar7;
  byte *pbVar8;
  uint local_18;
  uint *local_14;
  uint local_c;
  int local_8;
  
  if (param_3 < 2) {
    return 0x80004005;
  }
  if ((char)*param_2 == 'P') {
    if (*(char *)((int)param_2 + 1) == '3') {
      bVar2 = true;
    }
    else {
      if (*(char *)((int)param_2 + 1) != '6') {
        return 0x80004005;
      }
      bVar2 = false;
    }
    pbVar8 = (byte *)((int)param_2 + 2);
    param_3 = param_3 - 2;
    local_8 = 0;
    local_18 = 0;
    local_c = 0xff;
    pbVar3 = pbVar8;
    while (param_3 != 0) {
      uVar7 = 0;
      if ((!bVar2) && (local_8 == 3)) {
        if (0xff < local_c) {
          return 0x80004005;
        }
        if ((1 < param_3) && (*pbVar8 == 0xd)) {
          pbVar8 = pbVar8 + 1;
          param_3 = param_3 - 1;
        }
        iVar6 = param_3 - 1;
        pbVar8 = pbVar8 + 1;
        for (; (iVar6 != 0 && (param_2 < local_14)); param_2 = param_2 + 1) {
          *param_2 = ((((uint)*pbVar8 * 0xff) / local_c | 0xffffff00) << 8 |
                     ((uint)pbVar8[1] * 0xff) / local_c) << 8 | ((uint)pbVar8[2] * 0xff) / local_c;
          iVar6 = iVar6 + -3;
          pbVar8 = pbVar8 + 3;
        }
        return -(uint)(param_2 != local_14) & 0x80004005;
      }
      iVar6 = FUN_005912ad(*pbVar8);
      if (iVar6 == 0) {
        if (*pbVar8 == 0x23) {
          for (; (param_3 != 0 && (*pbVar8 != 10)); pbVar8 = pbVar8 + 1) {
            param_3 = param_3 - 1;
          }
          goto LAB_00565dc2;
        }
        while ((param_3 != 0 && (iVar6 = FUN_005912ad(*pbVar8), iVar6 == 0))) {
          iVar6 = FUN_00591258(*pbVar8);
          if (iVar6 == 0) {
            return 0x80004005;
          }
          bVar1 = *pbVar8;
          pbVar8 = pbVar8 + 1;
          param_3 = param_3 - 1;
          uVar7 = (bVar1 - 0x30) + uVar7 * 10;
          pbVar3 = pbVar8;
        }
        uVar4 = uVar7;
        uVar5 = local_c;
        if (local_8 == 0) {
LAB_00565f07:
          local_c = uVar5;
          local_18 = uVar4;
          if (uVar7 == 0) {
            return 0x80004005;
          }
        }
        else if (local_8 == 1) {
          if (uVar7 == 0) {
            return 0x80004005;
          }
          param_2 = operator_new(uVar7 * local_18 * 4);
          param_1[1] = param_2;
          if (param_2 == (uint *)0x0) {
            return 0x8007000e;
          }
          local_14 = param_2 + uVar7 * local_18;
          param_1[0xd] = 0;
          param_1[0xe] = 1;
          *param_1 = 0x16;
          param_1[0xc] = local_18 << 2;
          param_1[3] = local_18;
          param_1[4] = uVar7;
          param_1[5] = 1;
          pbVar8 = pbVar3;
        }
        else {
          uVar4 = local_18;
          uVar5 = uVar7;
          if (local_8 == 2) goto LAB_00565f07;
          if (local_8 == 3) {
            if (local_14 <= param_2) {
              return 0x80004005;
            }
            *param_2 = ((uVar7 * 0xff) / local_c | 0xffffff00) << 0x10;
          }
          else if (local_8 == 4) {
            *param_2 = *param_2 | (uVar7 * 0xff) / local_c << 8;
          }
          else if (local_8 == 5) {
            *param_2 = *param_2 | (uVar7 * 0xff) / local_c;
            param_2 = param_2 + 1;
            if (param_2 == local_14) {
              return 0;
            }
            local_8 = 2;
          }
        }
        local_8 = local_8 + 1;
      }
      else {
LAB_00565dc2:
        pbVar8 = pbVar8 + 1;
        param_3 = param_3 - 1;
        pbVar3 = pbVar8;
      }
    }
  }
  return 0x80004005;
}

