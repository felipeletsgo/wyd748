// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00404f54 | Name: FUN_00404f54


undefined4 __thiscall FUN_00404f54(int param_1,int param_2,int param_3)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = (**(code **)(**(int **)(param_1 + 0x24) + 0x48))();
  if (((param_2 == iVar1) && (param_3 == 0)) && (*(int *)(param_1 + -0x188) != 0)) {
    (**(code **)(*(int *)(param_1 + -0x1e4) + 0x60))(0);
    (**(code **)**(undefined4 **)(param_1 + -0x188))(*(undefined4 *)(param_1 + -0x1a0),0);
    uVar2 = 1;
  }
  else {
    if (((*(int *)(param_1 + 0x28) != 0) &&
        (iVar1 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(), param_2 == iVar1)) &&
       ((param_3 == 0 && (*(int *)(param_1 + -0x188) != 0)))) {
      (**(code **)(*(int *)(param_1 + -0x1e4) + 0x60))(0);
      (**(code **)**(undefined4 **)(param_1 + -0x188))(*(undefined4 *)(param_1 + -0x1a0),1);
      return 1;
    }
    uVar2 = 0;
  }
  return uVar2;
}

