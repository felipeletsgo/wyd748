// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058462f | Name: FUN_0058462f


void FUN_0058462f(int *param_1,char param_2)

{
  int *piVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  int iVar4;
  int *piVar5;
  
  piVar1 = param_1;
  puVar2 = (undefined4 *)(**(code **)param_1[1])(param_1,1,0x50);
  param_1[0x60] = (int)puVar2;
  *puVar2 = &LAB_005845c7;
  if (param_2 != '\0') {
    *(undefined4 *)(*param_1 + 0x14) = 4;
    (**(code **)*param_1)(param_1);
  }
  if (*(char *)(param_1[0x67] + 8) == '\0') {
    iVar4 = param_1[0x45];
  }
  else {
    if (param_1[0x45] < 2) {
      *(undefined4 *)(*param_1 + 0x14) = 0x2e;
      (**(code **)*param_1)(param_1);
    }
    FUN_005840f1();
    iVar4 = param_1[0x45] + 2;
  }
  param_1 = (int *)0x0;
  if (0 < piVar1[8]) {
    piVar5 = (int *)(piVar1[0x31] + 0x24);
    puVar2 = puVar2 + 2;
    do {
      uVar3 = (**(code **)(piVar1[1] + 8))
                        (piVar1,1,piVar5[-2] * *piVar5,
                         ((piVar5[-6] * *piVar5) / piVar1[0x45]) * iVar4);
      param_1 = (int *)((int)param_1 + 1);
      *puVar2 = uVar3;
      puVar2 = puVar2 + 1;
      piVar5 = piVar5 + 0x15;
    } while ((int)param_1 < piVar1[8]);
  }
  return;
}

