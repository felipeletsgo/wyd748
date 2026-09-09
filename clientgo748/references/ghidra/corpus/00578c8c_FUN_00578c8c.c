// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00578c8c | Name: FUN_00578c8c


void FUN_00578c8c(int *param_1,int param_2)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  int iVar4;
  
  iVar1 = param_1[1];
  if ((param_2 < 0) || (1 < param_2)) {
    *(undefined4 *)(*param_1 + 0x14) = 0xc;
    *(int *)(*param_1 + 0x18) = param_2;
    (**(code **)*param_1)(param_1);
  }
  if (param_2 == 1) {
    for (iVar4 = *(int *)(iVar1 + 0x40); iVar4 != 0; iVar4 = *(int *)(iVar4 + 0x24)) {
      if (*(char *)(iVar4 + 0x22) != '\0') {
        *(undefined1 *)(iVar4 + 0x22) = 0;
        (**(code **)(iVar4 + 0x30))(param_1,iVar4 + 0x28);
      }
    }
    *(undefined4 *)(iVar1 + 0x40) = 0;
    for (iVar4 = *(int *)(iVar1 + 0x44); iVar4 != 0; iVar4 = *(int *)(iVar4 + 0x24)) {
      if (*(char *)(iVar4 + 0x22) != '\0') {
        *(undefined1 *)(iVar4 + 0x22) = 0;
        (**(code **)(iVar4 + 0x30))(param_1,iVar4 + 0x28);
      }
    }
    *(undefined4 *)(iVar1 + 0x44) = 0;
  }
  puVar2 = *(undefined4 **)(iVar1 + 0x38 + param_2 * 4);
  *(undefined4 *)(iVar1 + 0x38 + param_2 * 4) = 0;
  while (puVar2 != (undefined4 *)0x0) {
    puVar3 = (undefined4 *)*puVar2;
    iVar4 = puVar2[2] + 0x10 + puVar2[1];
    FUN_0058a361(param_1,puVar2,iVar4);
    *(int *)(iVar1 + 0x48) = *(int *)(iVar1 + 0x48) - iVar4;
    puVar2 = puVar3;
  }
  puVar2 = (undefined4 *)(iVar1 + 0x30 + param_2 * 4);
  puVar3 = (undefined4 *)*puVar2;
  *puVar2 = 0;
  while (puVar3 != (undefined4 *)0x0) {
    puVar2 = (undefined4 *)*puVar3;
    iVar4 = puVar3[2] + 0x10 + puVar3[1];
    FUN_0058a361(param_1,puVar3,iVar4);
    *(int *)(iVar1 + 0x48) = *(int *)(iVar1 + 0x48) - iVar4;
    puVar3 = puVar2;
  }
  return;
}

