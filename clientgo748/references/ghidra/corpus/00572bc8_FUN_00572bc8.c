// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00572bc8 | Name: FUN_00572bc8


undefined4 FUN_00572bc8(int *param_1)

{
  int iVar1;
  byte bVar2;
  undefined4 *puVar3;
  int *piVar4;
  char cVar5;
  undefined4 uVar6;
  byte *pbVar7;
  
  piVar4 = param_1;
  puVar3 = (undefined4 *)param_1[5];
  pbVar7 = (byte *)*puVar3;
  iVar1 = puVar3[1];
  if (puVar3[1] == 0) {
    cVar5 = (*(code *)puVar3[3])(param_1);
    if (cVar5 != '\0') {
      pbVar7 = (byte *)*puVar3;
      iVar1 = puVar3[1];
      goto LAB_00572bf2;
    }
LAB_00572c0c:
    uVar6 = 0;
  }
  else {
LAB_00572bf2:
    param_1 = (int *)iVar1;
    param_1 = (int *)((int)param_1 + -1);
    bVar2 = *pbVar7;
    pbVar7 = pbVar7 + 1;
    if (param_1 == (int *)0x0) {
      cVar5 = (*(code *)puVar3[3])(piVar4);
      if (cVar5 == '\0') goto LAB_00572c0c;
      param_1 = (int *)puVar3[1];
      pbVar7 = (byte *)*puVar3;
    }
    iVar1 = (uint)bVar2 * 0x100 + (uint)*pbVar7;
    *(undefined4 *)(*piVar4 + 0x14) = 0x5a;
    *(int *)(*piVar4 + 0x18) = piVar4[0x5e];
    *(int *)(*piVar4 + 0x1c) = iVar1;
    (**(code **)(*piVar4 + 4))(piVar4,1);
    *puVar3 = pbVar7 + 1;
    puVar3[1] = (int)param_1 + -1;
    (**(code **)(piVar4[5] + 0x10))(piVar4,iVar1 + -2);
    uVar6 = 1;
  }
  return uVar6;
}

