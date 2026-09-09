// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005878ea | Name: FUN_005878ea


void FUN_005878ea(int *param_1)

{
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  bool bVar4;
  
  puVar1 = (undefined4 *)(**(code **)param_1[1])(param_1,1,0x18);
  param_1[0x68] = (int)puVar1;
  *puVar1 = &DAT_0058758f;
  iVar2 = param_1[9];
  if (iVar2 == 1) {
    bVar4 = param_1[8] == 1;
LAB_00587939:
    if (!bVar4) {
LAB_0058793b:
      *(undefined4 *)(*param_1 + 0x14) = 8;
      (**(code **)*param_1)(param_1);
    }
  }
  else {
    if (1 < iVar2) {
      if (iVar2 < 4) {
        bVar4 = param_1[8] == 3;
      }
      else {
        if (5 < iVar2) goto LAB_0058792f;
        bVar4 = param_1[8] == 4;
      }
      goto LAB_00587939;
    }
LAB_0058792f:
    if (param_1[8] < 1) goto LAB_0058793b;
  }
  iVar2 = param_1[10];
  if (iVar2 == 1) {
    iVar2 = 1;
    param_1[0x19] = 1;
    if ((param_1[9] == 1) || (param_1[9] == 3)) {
      puVar1[1] = FUN_0058743a;
      if (1 < param_1[8]) {
        iVar3 = 0x54;
        do {
          *(undefined1 *)(param_1[0x31] + 0x30 + iVar3) = 0;
          iVar2 = iVar2 + 1;
          iVar3 = iVar3 + 0x54;
        } while (iVar2 < param_1[8]);
      }
      goto LAB_005879f2;
    }
  }
  else if (iVar2 == 2) {
    param_1[0x19] = 3;
    if (param_1[9] == 3) {
      puVar1[1] = FUN_005876cc;
      goto LAB_005879a5;
    }
    bVar4 = param_1[9] == 2;
LAB_005879b1:
    if (bVar4) goto LAB_005879b3;
  }
  else {
    if (iVar2 == 4) {
      param_1[0x19] = 4;
      if (param_1[9] != 5) {
        bVar4 = param_1[9] == 4;
        goto LAB_005879b1;
      }
      puVar1[1] = FUN_0058745d;
LAB_005879a5:
      FUN_0058731f();
      goto LAB_005879f2;
    }
    if (iVar2 == param_1[9]) {
      param_1[0x19] = param_1[8];
LAB_005879b3:
      puVar1[1] = FUN_005873cf;
      goto LAB_005879f2;
    }
  }
  *(undefined4 *)(*param_1 + 0x14) = 0x19;
  (**(code **)*param_1)(param_1);
LAB_005879f2:
  if (*(char *)((int)param_1 + 0x4a) == '\0') {
    param_1[0x1a] = param_1[0x19];
  }
  else {
    param_1[0x1a] = 1;
  }
  return;
}

