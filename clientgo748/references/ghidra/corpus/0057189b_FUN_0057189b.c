// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057189b | Name: FUN_0057189b


int FUN_0057189b(int *param_1,char param_2)

{
  int iVar1;
  
  if ((param_1[4] != 200) && (param_1[4] != 0xc9)) {
    *(undefined4 *)(*param_1 + 0x14) = 0x12;
    *(int *)(*param_1 + 0x18) = param_1[4];
    (**(code **)*param_1)(param_1);
  }
  iVar1 = FUN_0057175a(param_1);
  if (iVar1 == 1) {
    iVar1 = 1;
  }
  else if (iVar1 == 2) {
    if (param_2 != '\0') {
      *(undefined4 *)(*param_1 + 0x14) = 0x32;
      (**(code **)*param_1)(param_1);
    }
    FUN_00578e51(param_1);
    return 2;
  }
  return iVar1;
}

