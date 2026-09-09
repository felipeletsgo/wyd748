// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00593891 | Name: FUN_00593891


void FUN_00593891(undefined4 param_1,int param_2,undefined4 param_3,undefined4 param_4,
                 undefined4 param_5,int param_6,int param_7,undefined4 *param_8,undefined4 param_9,
                 int param_10)

{
  int iVar1;
  
  if (param_7 != 0) {
    FUN_00593a51(param_1,param_2,param_6,param_7);
  }
  if (param_10 == 0) {
    param_10 = param_2;
  }
  FUN_0058ea87(param_10,param_1);
  FUN_005937f3(param_2,param_4,param_5,*param_8);
  *(int *)(param_2 + 8) = param_8[1] + 1;
  iVar1 = FUN_0059390c(param_1,param_2,param_3,param_5,*(undefined4 *)(param_6 + 0xc),param_9,0x100)
  ;
  if (iVar1 != 0) {
    FUN_0058ea45(iVar1,param_2);
  }
  return;
}

