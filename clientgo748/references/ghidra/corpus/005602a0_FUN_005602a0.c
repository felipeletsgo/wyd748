// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005602a0 | Name: FUN_005602a0


int FUN_005602a0(int *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,int param_5,
                undefined4 param_6,undefined4 param_7,int param_8)

{
  int iVar1;
  int iVar2;
  
  if ((param_1 == (int *)0x0) || (param_8 == 0)) {
    iVar1 = -0x7789f794;
  }
  else {
    iVar2 = param_5;
    if (param_5 == -1) {
      iVar2 = 0;
    }
    iVar1 = FUN_00560278(param_1,&param_2,&param_3,&param_4,iVar2,&param_6,param_7);
    if ((-1 < iVar1) &&
       (iVar1 = (**(code **)(*param_1 + 0x50))
                          (param_1,param_2,param_3,param_4,iVar2,param_6,param_7,param_8),
       -1 < iVar1)) {
      iVar1 = 0;
    }
  }
  return iVar1;
}

