// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058980c | Name: FUN_0058980c


void FUN_0058980c(int *param_1)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  
  iVar1 = param_1[0x69];
  param_1[0x1d] = *(int *)(iVar1 + 0x10);
  param_1[0x1c] = *(int *)(iVar1 + 0x14);
  iVar3 = param_1[0x13];
  if (iVar3 == 0) {
    if (param_1[0x19] == 3) {
      *(code **)(iVar1 + 4) = FUN_005893db;
    }
    else {
      *(code **)(iVar1 + 4) = FUN_00589353;
    }
  }
  else if (iVar3 == 1) {
    if (param_1[0x19] == 3) {
      *(code **)(iVar1 + 4) = FUN_00589553;
    }
    else {
      *(code **)(iVar1 + 4) = FUN_00589468;
    }
    *(undefined4 *)(iVar1 + 0x30) = 0;
    if (*(char *)(iVar1 + 0x1c) == '\0') {
      FUN_00589163(param_1);
    }
    if (*(int *)(iVar1 + 0x34) == 0) {
      FUN_0058928e();
    }
  }
  else if (iVar3 == 2) {
    *(undefined1 *)(iVar1 + 0x54) = 0;
    piVar2 = (int *)(iVar1 + 0x44);
    *(code **)(iVar1 + 4) = FUN_00589652;
    if (*piVar2 == 0) {
      FUN_005897d6();
    }
    iVar1 = param_1[0x17];
    iVar3 = 0;
    if (0 < param_1[0x19]) {
      do {
        FUN_00577ec2(*piVar2,iVar1 * 2 + 4);
        iVar3 = iVar3 + 1;
        piVar2 = piVar2 + 1;
      } while (iVar3 < param_1[0x19]);
    }
  }
  else {
    *(undefined4 *)(*param_1 + 0x14) = 0x2f;
    (**(code **)*param_1)(param_1);
  }
  return;
}

