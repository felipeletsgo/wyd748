// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00588614 | Name: FUN_00588614


void FUN_00588614(int param_1,int param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  int in_EAX;
  uint uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int unaff_EDI;
  int aiStack_42c [256];
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_8 = 0;
  local_1c = param_1 * 2 + 0x18 >> 1;
  local_24 = unaff_EDI * 2 + 0x1c >> 1;
  local_10 = *(int *)(in_EAX + 0x70);
  local_28 = param_2 * 2 + 0x18 >> 1;
  local_c = 0x7fffffff;
  if (0 < *(int *)(in_EAX + 0x70)) {
    piVar1 = *(int **)(in_EAX + 0x74);
    local_2c = *piVar1;
    local_14 = piVar1[1];
    local_18 = piVar1[2];
    do {
      iVar6 = local_c;
      uVar3 = (uint)*(byte *)(local_2c + local_8);
      iVar4 = param_1 + 0x18;
      if ((int)uVar3 < param_1) {
        iVar7 = (uVar3 - param_1) * 2;
        iVar7 = iVar7 * iVar7;
LAB_005886cc:
        iVar5 = iVar4;
      }
      else {
        iVar5 = param_1;
        if (iVar4 < (int)uVar3) {
          iVar7 = (uVar3 - iVar4) * 2;
          iVar7 = iVar7 * iVar7;
        }
        else {
          iVar7 = 0;
          if ((int)uVar3 <= local_1c) goto LAB_005886cc;
        }
      }
      iVar4 = (uVar3 - iVar5) * 2;
      uVar3 = (uint)*(byte *)(local_14 + local_8);
      if ((int)uVar3 < unaff_EDI) {
        iVar5 = (uVar3 - unaff_EDI) * 3;
        iVar7 = iVar7 + iVar5 * iVar5;
        iVar5 = uVar3 - (unaff_EDI + 0x1c);
      }
      else {
        if (unaff_EDI + 0x1c < (int)uVar3) {
          iVar5 = (uVar3 - (unaff_EDI + 0x1c)) * 3;
          iVar7 = iVar7 + iVar5 * iVar5;
        }
        else if ((int)uVar3 <= local_24) {
          iVar5 = uVar3 - (unaff_EDI + 0x1c);
          goto LAB_005886ed;
        }
        iVar5 = uVar3 - unaff_EDI;
      }
LAB_005886ed:
      uVar3 = (uint)*(byte *)(local_18 + local_8);
      if ((int)uVar3 < param_2) {
        iVar7 = iVar7 + (uVar3 - param_2) * (uVar3 - param_2);
LAB_00588790:
        iVar2 = param_2 + 0x18;
      }
      else {
        local_20 = param_2 + 0x18;
        iVar2 = param_2;
        if (local_20 < (int)uVar3) {
          iVar7 = iVar7 + (uVar3 - local_20) * (uVar3 - local_20);
        }
        else if ((int)uVar3 <= local_28) goto LAB_00588790;
      }
      iVar4 = iVar4 * iVar4 + iVar5 * 3 * iVar5 * 3 + (uVar3 - iVar2) * (uVar3 - iVar2);
      aiStack_42c[local_8] = iVar7;
      if (iVar4 < iVar6) {
        local_c = iVar4;
      }
      local_8 = local_8 + 1;
    } while (local_8 < local_10);
  }
  iVar6 = 0;
  iVar4 = 0;
  if (0 < local_10) {
    do {
      if (aiStack_42c[iVar6] <= local_c) {
        *(char *)(iVar4 + param_3) = (char)iVar6;
        iVar4 = iVar4 + 1;
      }
      iVar6 = iVar6 + 1;
    } while (iVar6 < local_10);
  }
  return;
}

