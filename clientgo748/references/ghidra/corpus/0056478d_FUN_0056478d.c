// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056478d | Name: FUN_0056478d


int FUN_0056478d(undefined4 param_1,undefined4 *param_2)

{
  void *pvVar1;
  int *piVar2;
  int iVar3;
  
  pvVar1 = operator_new(0x10);
  if (pvVar1 == (void *)0x0) {
    piVar2 = (int *)0x0;
  }
  else {
    piVar2 = (int *)FUN_0057150a();
  }
  if (piVar2 == (int *)0x0) {
    iVar3 = -0x7ff8fff2;
  }
  else {
    iVar3 = (**(code **)(*piVar2 + 0x18))(param_1);
    if (-1 < iVar3) {
      *param_2 = piVar2;
      piVar2 = (int *)0x0;
    }
  }
  if (piVar2 != (int *)0x0) {
    (**(code **)(*piVar2 + 0x14))(1);
  }
  return iVar3;
}

