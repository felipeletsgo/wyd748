// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00585cd3 | Name: FUN_00585cd3


undefined1 FUN_00585cd3(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  char cVar3;
  undefined4 uVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  uint uVar8;
  int iVar9;
  int iVar10;
  int *piVar11;
  int *piVar12;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  int local_50;
  int local_44;
  int local_40;
  int local_3c;
  int local_34 [5];
  undefined2 *local_20;
  undefined4 local_1c;
  int local_18;
  int *local_14;
  int local_10;
  int local_c;
  uint local_8;
  
  iVar9 = param_1;
  local_c = *(int *)(param_1 + 0x194);
  local_1c = *(undefined4 *)(param_1 + 0x174);
  if (((*(int *)(param_1 + 0xfc) != 0) && (*(int *)(*(int *)(param_1 + 0x194) + 0x34) == 0)) &&
     (cVar3 = FUN_00585c78(), cVar3 == '\0')) {
    return 0;
  }
  iVar6 = local_c;
  local_10 = 0;
  iVar1 = *(int *)(param_1 + 0x13c);
  local_40 = param_1;
  local_5c = **(undefined4 **)(param_1 + 0x14);
  local_58 = (*(undefined4 **)(param_1 + 0x14))[1];
  local_54 = *(undefined4 *)(param_1 + 0x178);
  iVar2 = *(int *)(local_c + 0x10);
  iVar10 = *(int *)(local_c + 0x14);
  local_3c = local_c + 0x18;
  piVar11 = (int *)(local_c + 0x20);
  piVar12 = local_34;
  for (iVar7 = 5; iVar7 != 0; iVar7 = iVar7 + -1) {
    *piVar12 = *piVar11;
    piVar11 = piVar11 + 1;
    piVar12 = piVar12 + 1;
  }
  if (0 < iVar1) {
    local_14 = (int *)(param_1 + 0x140);
    param_1 = iVar2;
    do {
      local_20 = *(undefined2 **)(param_2 + local_10 * 4);
      local_18 = *local_14;
      iVar1 = *(int *)(iVar6 + 0x38 + *(int *)(*(int *)(iVar9 + 0x124 + *local_14 * 4) + 0x14) * 4);
      if (iVar10 < 8) {
        cVar3 = FUN_00585510(&local_5c,param_1,iVar10,0);
        if (cVar3 == '\0') {
          return 0;
        }
        param_1 = local_50;
        iVar6 = local_c;
        iVar10 = local_44;
        if (7 < local_44) goto LAB_00585db7;
        uVar4 = 1;
LAB_00585ddf:
        uVar5 = FUN_005855fa(&local_5c,param_1,iVar10,iVar1,uVar4);
        if ((int)uVar5 < 0) {
          return 0;
        }
        param_1 = local_50;
        iVar6 = local_c;
        iVar10 = local_44;
      }
      else {
LAB_00585db7:
        uVar5 = param_1 >> ((char)iVar10 - 8U & 0x1f) & 0xff;
        iVar2 = *(int *)(iVar1 + 0xd4 + uVar5 * 4);
        if (iVar2 == 0) {
          uVar4 = 9;
          goto LAB_00585ddf;
        }
        uVar5 = (uint)*(byte *)(uVar5 + 0x4d4 + iVar1);
        iVar10 = iVar10 - iVar2;
      }
      uVar8 = 0;
      if (uVar5 != 0) {
        if (iVar10 < (int)uVar5) {
          cVar3 = FUN_00585510(&local_5c,param_1,iVar10,uVar5);
          if (cVar3 == '\0') {
            return 0;
          }
          param_1 = local_50;
          iVar6 = local_c;
          iVar10 = local_44;
        }
        iVar10 = iVar10 - uVar5;
        local_8 = (1 << ((byte)uVar5 & 0x1f)) - 1;
        uVar8 = local_8 & param_1 >> ((byte)iVar10 & 0x1f);
        if ((int)uVar8 < *(int *)(&DAT_005a9490 + uVar5 * 4)) {
          uVar8 = *(int *)(&DAT_005a94d0 + uVar5 * 4) + uVar8;
        }
      }
      local_14 = local_14 + 1;
      piVar11 = local_34 + local_18 + 1;
      *piVar11 = *piVar11 + uVar8;
      local_10 = local_10 + 1;
      *local_20 = (short)(*piVar11 << ((byte)local_1c & 0x1f));
      iVar2 = param_1;
    } while (local_10 < *(int *)(iVar9 + 0x13c));
  }
  param_1 = iVar2;
  **(undefined4 **)(iVar9 + 0x14) = local_5c;
  *(undefined4 *)(*(int *)(iVar9 + 0x14) + 4) = local_58;
  *(undefined4 *)(iVar9 + 0x178) = local_54;
  *(int *)(iVar6 + 0x34) = *(int *)(iVar6 + 0x34) + -1;
  *(int *)(iVar6 + 0x10) = param_1;
  *(int *)(iVar6 + 0x14) = iVar10;
  piVar11 = local_34;
  piVar12 = (int *)(iVar6 + 0x20);
  for (iVar9 = 5; iVar9 != 0; iVar9 = iVar9 + -1) {
    *piVar12 = *piVar11;
    piVar11 = piVar11 + 1;
    piVar12 = piVar12 + 1;
  }
  return 1;
}

