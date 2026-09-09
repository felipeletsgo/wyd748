// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004aad84 | Name: FUN_004aad84


void __thiscall FUN_004aad84(int *param_1,char param_2,undefined4 param_3)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  iVar3 = param_1[0x9f47];
  iVar2 = param_1[0x9f48];
  if (param_2 == '\t') {
    iVar1 = FUN_00493c10();
    if (iVar1 == 1) {
      (**(code **)(*(int *)param_1[10] + 0x40))(iVar2);
    }
    else {
      iVar2 = FUN_00493c10();
      if (iVar2 == 1) {
        (**(code **)(*(int *)param_1[10] + 0x40))(iVar3);
      }
    }
  }
  else if (param_2 == '\r') {
    iVar3 = FUN_00493c10();
    if ((iVar3 == 1) && (*(int *)(param_1[0x9b98] + 0x2c) == 1)) {
      (**(code **)(*param_1 + 0x58))(0x1201,0);
    }
  }
  else if (param_2 == '\x1b') {
    if (((char)param_1[0x9f49] == '\0') || ((char)param_1[0x9f49] == '\x02')) {
      (**(code **)(*(int *)param_1[0x23] + 0x8c))(&DAT_00a34c78,0x1203,0);
    }
    else {
      (**(code **)(*(int *)param_1[0x23] + 0x8c))(&DAT_00a34278,0x1203,0,param_1,iVar2,iVar3);
    }
    (**(code **)(*(int *)param_1[0x23] + 0x60))(1);
  }
  FUN_0049ac84(param_2,param_3);
  return;
}

