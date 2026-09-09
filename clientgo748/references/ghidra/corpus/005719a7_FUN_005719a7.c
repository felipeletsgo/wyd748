// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005719a7 | Name: FUN_005719a7


int * FUN_005719a7(int *param_1,undefined4 param_2,undefined4 param_3)

{
  int *piVar1;
  
  piVar1 = param_1;
  if (param_1[4] != 0xcd) {
    *(undefined4 *)(*param_1 + 0x14) = 0x12;
    *(int *)(*param_1 + 0x18) = param_1[4];
    (**(code **)*param_1)(param_1);
  }
  if ((uint)piVar1[0x1e] < (uint)piVar1[0x18]) {
    if (piVar1[2] != 0) {
      *(int *)(piVar1[2] + 4) = piVar1[0x1e];
      *(int *)(piVar1[2] + 8) = piVar1[0x18];
      (**(code **)piVar1[2])(piVar1);
    }
    param_1 = (int *)0x0;
    (**(code **)(piVar1[0x60] + 4))(piVar1,param_2,&param_1,param_3);
    piVar1[0x1e] = piVar1[0x1e] + (int)param_1;
  }
  else {
    *(undefined4 *)(*piVar1 + 0x14) = 0x77;
    (**(code **)(*piVar1 + 4))(piVar1,0xffffffff);
    param_1 = (int *)0x0;
  }
  return param_1;
}

