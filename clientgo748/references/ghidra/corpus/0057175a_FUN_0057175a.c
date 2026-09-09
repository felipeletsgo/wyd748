// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057175a | Name: FUN_0057175a


int FUN_0057175a(int *param_1)

{
  int iVar1;
  
  iVar1 = param_1[4];
  if (iVar1 == 200) {
    (**(code **)(param_1[99] + 4))(param_1);
    (**(code **)(param_1[5] + 8))(param_1);
    param_1[4] = 0xc9;
  }
  else if (iVar1 != 0xc9) {
    if (iVar1 == 0xca) {
      return 1;
    }
    if ((0xca < iVar1) && ((iVar1 < 0xd1 || (iVar1 == 0xd2)))) {
      iVar1 = (**(code **)param_1[99])(param_1);
      return iVar1;
    }
    *(undefined4 *)(*param_1 + 0x14) = 0x12;
    *(int *)(*param_1 + 0x18) = param_1[4];
    (**(code **)*param_1)(param_1);
    return 0;
  }
  iVar1 = (**(code **)param_1[99])(param_1);
  if (iVar1 == 1) {
    FUN_005715e7();
    param_1[4] = 0xca;
  }
  return iVar1;
}

