// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005717f4 | Name: FUN_005717f4


undefined4 FUN_005717f4(int *param_1)

{
  int iVar1;
  
  iVar1 = param_1[4];
  if (((iVar1 == 0xcd) || (iVar1 == 0xce)) && ((char)param_1[0x10] == '\0')) {
    if ((uint)param_1[0x1e] < (uint)param_1[0x18]) {
      *(undefined4 *)(*param_1 + 0x14) = 0x42;
      (**(code **)*param_1)(param_1);
    }
    (**(code **)(param_1[0x5f] + 4))(param_1);
    param_1[4] = 0xd2;
  }
  else if (iVar1 == 0xcf) {
    param_1[4] = 0xd2;
  }
  else if (iVar1 != 0xd2) {
    *(undefined4 *)(*param_1 + 0x14) = 0x12;
    *(int *)(*param_1 + 0x18) = param_1[4];
    (**(code **)*param_1)(param_1);
  }
  do {
    if (*(char *)(param_1[99] + 0x11) != '\0') {
      (**(code **)(param_1[5] + 0x18))(param_1);
      FUN_00578e51(param_1);
      return 1;
    }
    iVar1 = (**(code **)param_1[99])(param_1);
  } while (iVar1 != 0);
  return 0;
}

