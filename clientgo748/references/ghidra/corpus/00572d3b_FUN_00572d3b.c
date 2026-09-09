// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00572d3b | Name: FUN_00572d3b


undefined4 FUN_00572d3b(int *param_1)

{
  byte bVar1;
  undefined4 *puVar2;
  char cVar3;
  undefined4 uVar4;
  uint uVar5;
  int iVar6;
  byte *pbVar7;
  
  puVar2 = (undefined4 *)param_1[5];
  iVar6 = puVar2[1];
  pbVar7 = (byte *)*puVar2;
  if (iVar6 == 0) {
    cVar3 = (*(code *)puVar2[3])(param_1);
    if (cVar3 != '\0') {
      pbVar7 = (byte *)*puVar2;
      iVar6 = puVar2[1];
      goto LAB_00572d62;
    }
LAB_00572d77:
    uVar4 = 0;
  }
  else {
LAB_00572d62:
    bVar1 = *pbVar7;
    iVar6 = iVar6 + -1;
    pbVar7 = pbVar7 + 1;
    if (iVar6 == 0) {
      cVar3 = (*(code *)puVar2[3])(param_1);
      if (cVar3 == '\0') goto LAB_00572d77;
      pbVar7 = (byte *)*puVar2;
      iVar6 = puVar2[1];
    }
    uVar5 = (uint)*pbVar7;
    if ((bVar1 != 0xff) || (uVar5 != 0xd8)) {
      *(undefined4 *)(*param_1 + 0x14) = 0x34;
      *(uint *)(*param_1 + 0x18) = (uint)bVar1;
      *(uint *)(*param_1 + 0x1c) = uVar5;
      (**(code **)*param_1)(param_1);
    }
    param_1[0x5e] = uVar5;
    *puVar2 = pbVar7 + 1;
    puVar2[1] = iVar6 + -1;
    uVar4 = 1;
  }
  return uVar4;
}

