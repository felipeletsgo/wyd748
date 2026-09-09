// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00585ed0 | Name: FUN_00585ed0


undefined1 FUN_00585ed0(uint param_1,int *param_2)

{
  int iVar1;
  int iVar2;
  char cVar3;
  int iVar4;
  undefined4 uVar5;
  uint uVar6;
  int iVar7;
  int iVar8;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  int local_40;
  int local_34;
  int local_30;
  int local_2c;
  int local_24;
  undefined4 local_20;
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  iVar2 = param_1;
  local_1c = *(int *)(param_1 + 0x16c);
  iVar1 = *(int *)(param_1 + 0x194);
  local_20 = *(undefined4 *)(param_1 + 0x174);
  if (((*(int *)(param_1 + 0xfc) != 0) && (*(int *)(iVar1 + 0x34) == 0)) &&
     (cVar3 = FUN_00585c78(), cVar3 == '\0')) {
    return 0;
  }
  if (*(int *)(iVar1 + 0x20) == 0) {
    local_30 = param_1;
    local_4c = **(undefined4 **)(param_1 + 0x14);
    local_48 = (*(undefined4 **)(param_1 + 0x14))[1];
    local_44 = *(undefined4 *)(param_1 + 0x178);
    iVar8 = *(int *)(iVar1 + 0x10);
    iVar4 = *(int *)(iVar1 + 0x14);
    local_2c = iVar1 + 0x18;
    local_24 = *param_2;
    local_18 = *(int *)(iVar1 + 0x48);
    local_c = 0;
    local_10 = iVar8;
    for (local_8 = *(int *)(param_1 + 0x168); local_8 <= local_1c; local_8 = local_8 + 1) {
      if (iVar4 < 8) {
        cVar3 = FUN_00585510(&local_4c,iVar8,iVar4,0);
        if (cVar3 == '\0') {
          return 0;
        }
        local_10 = local_40;
        iVar4 = local_34;
        iVar8 = local_40;
        if (7 < local_34) goto LAB_00585f95;
        uVar5 = 1;
LAB_00585fc5:
        param_1 = FUN_005855fa(&local_4c,iVar8,iVar4,local_18,uVar5);
        if ((int)param_1 < 0) {
          return 0;
        }
        local_10 = local_40;
        iVar4 = local_34;
        iVar8 = local_40;
      }
      else {
LAB_00585f95:
        uVar6 = iVar8 >> ((char)iVar4 - 8U & 0x1f) & 0xff;
        iVar7 = *(int *)(local_18 + 0xd4 + uVar6 * 4);
        if (iVar7 == 0) {
          uVar5 = 9;
          goto LAB_00585fc5;
        }
        iVar4 = iVar4 - iVar7;
        param_1 = (uint)*(byte *)(uVar6 + 0x4d4 + local_18);
      }
      local_14 = (int)param_1 >> 4;
      param_1 = param_1 & 0xf;
      if (param_1 == 0) {
        if (local_14 != 0xf) {
          iVar7 = 1 << ((byte)local_14 & 0x1f);
          local_c = iVar7;
          if (local_14 != 0) {
            if ((iVar4 < local_14) &&
               (cVar3 = FUN_00585510(&local_4c,iVar8,iVar4,local_14), iVar4 = local_34,
               iVar8 = local_40, cVar3 == '\0')) {
              return 0;
            }
            iVar4 = iVar4 - local_14;
            local_c = local_c + (iVar8 >> ((byte)iVar4 & 0x1f) & iVar7 - 1U);
          }
          local_c = local_c + -1;
          break;
        }
        local_8 = local_8 + 0xf;
      }
      else {
        local_8 = local_8 + local_14;
        if (iVar4 < (int)param_1) {
          cVar3 = FUN_00585510(&local_4c,iVar8,iVar4,param_1);
          if (cVar3 == '\0') {
            return 0;
          }
          local_10 = local_40;
          iVar4 = local_34;
          iVar8 = local_40;
        }
        iVar4 = iVar4 - param_1;
        uVar6 = (1 << (sbyte)param_1) - 1U & iVar8 >> ((byte)iVar4 & 0x1f);
        if ((int)uVar6 < *(int *)(&DAT_005a9490 + param_1 * 4)) {
          uVar6 = *(int *)(&DAT_005a94d0 + param_1 * 4) + uVar6;
        }
        *(short *)(local_24 + (&DAT_005a88d0)[local_8] * 2) =
             (short)(uVar6 << ((byte)local_20 & 0x1f));
        iVar8 = local_10;
      }
    }
    **(undefined4 **)(iVar2 + 0x14) = local_4c;
    *(undefined4 *)(*(int *)(iVar2 + 0x14) + 4) = local_48;
    *(undefined4 *)(iVar2 + 0x178) = local_44;
    *(int *)(iVar1 + 0x14) = iVar4;
    *(int *)(iVar1 + 0x10) = iVar8;
  }
  else {
    local_c = *(int *)(iVar1 + 0x20) + -1;
  }
  *(int *)(iVar1 + 0x34) = *(int *)(iVar1 + 0x34) + -1;
  *(int *)(iVar1 + 0x20) = local_c;
  return 1;
}

