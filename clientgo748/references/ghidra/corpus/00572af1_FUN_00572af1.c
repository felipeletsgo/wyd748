// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00572af1 | Name: FUN_00572af1


uint FUN_00572af1(int *param_1)

{
  int iVar1;
  byte bVar2;
  undefined4 *puVar3;
  uint uVar4;
  int iVar5;
  byte *pbVar6;
  byte *pbVar7;
  
  puVar3 = (undefined4 *)param_1[5];
  iVar5 = puVar3[1];
  pbVar6 = (byte *)*puVar3;
  if (iVar5 == 0) {
    uVar4 = (*(code *)puVar3[3])(param_1);
    if ((char)uVar4 != '\0') {
      pbVar6 = (byte *)*puVar3;
      iVar5 = puVar3[1];
      goto LAB_00572b18;
    }
LAB_00572b81:
    uVar4 = uVar4 & 0xffffff00;
  }
  else {
LAB_00572b18:
    bVar2 = *pbVar6;
    iVar5 = iVar5 + -1;
    pbVar6 = pbVar6 + 1;
    if (iVar5 == 0) {
      uVar4 = (*(code *)puVar3[3])(param_1);
      if ((char)uVar4 == '\0') goto LAB_00572b81;
      pbVar6 = (byte *)*puVar3;
      iVar5 = puVar3[1];
    }
    iVar5 = iVar5 + -1;
    pbVar7 = pbVar6 + 1;
    if ((uint)bVar2 * 0x100 + (uint)*pbVar6 != 4) {
      *(undefined4 *)(*param_1 + 0x14) = 9;
      (**(code **)*param_1)(param_1);
    }
    if (iVar5 == 0) {
      uVar4 = (*(code *)puVar3[3])(param_1);
      if ((char)uVar4 == '\0') goto LAB_00572b81;
      pbVar7 = (byte *)*puVar3;
      iVar5 = puVar3[1];
    }
    bVar2 = *pbVar7;
    iVar5 = iVar5 + -1;
    pbVar7 = pbVar7 + 1;
    if (iVar5 == 0) {
      uVar4 = (*(code *)puVar3[3])(param_1);
      if ((char)uVar4 == '\0') goto LAB_00572b81;
      pbVar7 = (byte *)*puVar3;
      iVar5 = puVar3[1];
    }
    iVar1 = (uint)bVar2 * 0x100 + (uint)*pbVar7;
    *(undefined4 *)(*param_1 + 0x14) = 0x51;
    *(int *)(*param_1 + 0x18) = iVar1;
    (**(code **)(*param_1 + 4))(param_1,1);
    param_1[0x3f] = iVar1;
    *puVar3 = pbVar7 + 1;
    puVar3[1] = iVar5 + -1;
    uVar4 = CONCAT31((int3)((uint)iVar1 >> 8),1);
  }
  return uVar4;
}

