// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00579639 | Name: FUN_00579639


int FUN_00579639(int *param_1,int param_2)

{
  byte bVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 *puVar4;
  int iVar5;
  
  if (((param_1 == (int *)0x0) || (puVar4 = (undefined4 *)param_1[7], puVar4 == (undefined4 *)0x0))
     || (*param_1 == 0)) {
LAB_00579876:
    return -2;
  }
  iVar3 = -5;
  iVar5 = 0;
  if (param_2 == 4) {
    iVar5 = iVar3;
  }
LAB_00579869:
  switch(*puVar4) {
  case 0:
    if (param_1[1] == 0) {
      return iVar3;
    }
    param_1[2] = param_1[2] + 1;
    param_1[1] = param_1[1] + -1;
    *(uint *)(param_1[7] + 4) = (uint)*(byte *)*param_1;
    puVar4 = (undefined4 *)param_1[7];
    uVar2 = puVar4[1];
    *param_1 = *param_1 + 1;
    if (((byte)uVar2 & 0xf) == 8) {
      if (((uint)puVar4[1] >> 4) + 8 <= (uint)puVar4[4]) {
        *puVar4 = 1;
        iVar3 = iVar5;
        goto switchD_00579676_caseD_1;
      }
      *puVar4 = 0xd;
      param_1[6] = (int)"invalid window size";
    }
    else {
      *puVar4 = 0xd;
      param_1[6] = (int)"unknown compression method";
    }
    goto LAB_0057985c;
  case 1:
switchD_00579676_caseD_1:
    if (param_1[1] == 0) {
      return iVar3;
    }
    puVar4 = (undefined4 *)param_1[7];
    param_1[2] = param_1[2] + 1;
    param_1[1] = param_1[1] + -1;
    bVar1 = *(byte *)*param_1;
    *param_1 = (int)((byte *)*param_1 + 1);
    if ((puVar4[1] * 0x100 + (uint)bVar1) % 0x1f != 0) {
      *puVar4 = 0xd;
      param_1[6] = (int)"incorrect header check";
      goto LAB_0057985c;
    }
    if ((bVar1 & 0x20) != 0) {
      *(undefined4 *)param_1[7] = 2;
      iVar3 = iVar5;
      goto switchD_00579676_caseD_2;
    }
    *puVar4 = 7;
    iVar3 = iVar5;
    break;
  case 2:
switchD_00579676_caseD_2:
    if (param_1[1] == 0) {
      return iVar3;
    }
    param_1[2] = param_1[2] + 1;
    param_1[1] = param_1[1] + -1;
    *(uint *)(param_1[7] + 8) = (uint)*(byte *)*param_1 << 0x18;
    *param_1 = *param_1 + 1;
    *(undefined4 *)param_1[7] = 3;
    iVar3 = iVar5;
  case 3:
    goto switchD_00579676_caseD_3;
  case 4:
    goto switchD_00579676_caseD_4;
  case 5:
    goto switchD_00579676_caseD_5;
  case 6:
    *(undefined4 *)param_1[7] = 0xd;
    param_1[6] = (int)"need dictionary";
    *(undefined4 *)(param_1[7] + 4) = 0;
    return -2;
  case 7:
    iVar3 = FUN_00589a69(*(undefined4 *)(param_1[7] + 0x14),param_1,iVar3);
    if (iVar3 == -3) {
      *(undefined4 *)param_1[7] = 0xd;
      *(undefined4 *)(param_1[7] + 4) = 0;
      iVar3 = -3;
    }
    else {
      if (iVar3 == 0) {
        iVar3 = iVar5;
      }
      if (iVar3 != 1) {
        return iVar3;
      }
      FUN_00589971(*(undefined4 *)(param_1[7] + 0x14),param_1,param_1[7] + 4);
      puVar4 = (undefined4 *)param_1[7];
      if (puVar4[3] == 0) {
        *puVar4 = 8;
        iVar3 = iVar5;
        goto switchD_00579676_caseD_8;
      }
      *puVar4 = 0xc;
      iVar3 = iVar5;
    }
    break;
  case 8:
switchD_00579676_caseD_8:
    if (param_1[1] == 0) {
      return iVar3;
    }
    param_1[2] = param_1[2] + 1;
    param_1[1] = param_1[1] + -1;
    *(uint *)(param_1[7] + 8) = (uint)*(byte *)*param_1 << 0x18;
    *param_1 = *param_1 + 1;
    *(undefined4 *)param_1[7] = 9;
    iVar3 = iVar5;
  case 9:
    if (param_1[1] == 0) {
      return iVar3;
    }
    param_1[2] = param_1[2] + 1;
    param_1[1] = param_1[1] + -1;
    *(int *)(param_1[7] + 8) = *(int *)(param_1[7] + 8) + (uint)*(byte *)*param_1 * 0x10000;
    *param_1 = *param_1 + 1;
    *(undefined4 *)param_1[7] = 10;
    iVar3 = iVar5;
  case 10:
    goto switchD_00579676_caseD_a;
  case 0xb:
    goto switchD_00579676_caseD_b;
  case 0xc:
    goto LAB_00579876;
  case 0xd:
    return -3;
  default:
    goto LAB_00579876;
  }
LAB_00579866:
  puVar4 = (undefined4 *)param_1[7];
  goto LAB_00579869;
switchD_00579676_caseD_a:
  if (param_1[1] == 0) {
    return iVar3;
  }
  param_1[2] = param_1[2] + 1;
  param_1[1] = param_1[1] + -1;
  *(int *)(param_1[7] + 8) = *(int *)(param_1[7] + 8) + (uint)*(byte *)*param_1 * 0x100;
  *param_1 = *param_1 + 1;
  *(undefined4 *)param_1[7] = 0xb;
  iVar3 = iVar5;
switchD_00579676_caseD_b:
  if (param_1[1] == 0) {
    return iVar3;
  }
  param_1[2] = param_1[2] + 1;
  param_1[1] = param_1[1] + -1;
  *(int *)(param_1[7] + 8) = *(int *)(param_1[7] + 8) + (uint)*(byte *)*param_1;
  puVar4 = (undefined4 *)param_1[7];
  *param_1 = *param_1 + 1;
  if (puVar4[1] == puVar4[2]) {
    *(undefined4 *)param_1[7] = 0xc;
LAB_00579876:
    return 1;
  }
  *puVar4 = 0xd;
  param_1[6] = (int)"incorrect data check";
LAB_0057985c:
  *(undefined4 *)(param_1[7] + 4) = 5;
  iVar3 = iVar5;
  goto LAB_00579866;
switchD_00579676_caseD_3:
  if (param_1[1] == 0) {
    return iVar3;
  }
  param_1[2] = param_1[2] + 1;
  param_1[1] = param_1[1] + -1;
  *(int *)(param_1[7] + 8) = *(int *)(param_1[7] + 8) + (uint)*(byte *)*param_1 * 0x10000;
  *param_1 = *param_1 + 1;
  *(undefined4 *)param_1[7] = 4;
  iVar3 = iVar5;
switchD_00579676_caseD_4:
  if (param_1[1] == 0) {
    return iVar3;
  }
  param_1[2] = param_1[2] + 1;
  param_1[1] = param_1[1] + -1;
  *(int *)(param_1[7] + 8) = *(int *)(param_1[7] + 8) + (uint)*(byte *)*param_1 * 0x100;
  *param_1 = *param_1 + 1;
  *(undefined4 *)param_1[7] = 5;
  iVar3 = iVar5;
switchD_00579676_caseD_5:
  if (param_1[1] != 0) {
    param_1[2] = param_1[2] + 1;
    param_1[1] = param_1[1] + -1;
    *(int *)(param_1[7] + 8) = *(int *)(param_1[7] + 8) + (uint)*(byte *)*param_1;
    *param_1 = *param_1 + 1;
    param_1[0xc] = ((undefined4 *)param_1[7])[2];
    *(undefined4 *)param_1[7] = 6;
    return 2;
  }
  return iVar3;
}

