// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059e78b | Name: FUN_0059e78b


undefined4 * __thiscall FUN_0059e78b(undefined4 *param_1,int param_2)

{
  int iVar1;
  size_t sVar2;
  void *pvVar3;
  
  *param_1 = &PTR_FUN_005aa91c;
  iVar1 = *(int *)(param_2 + 8);
  param_1[2] = iVar1;
  if (iVar1 == 0) {
    param_1[1] = *(undefined4 *)(param_2 + 4);
  }
  else {
    sVar2 = _strlen(*(char **)(param_2 + 4));
    pvVar3 = operator_new(sVar2 + 1);
    param_1[1] = pvVar3;
    if (pvVar3 != (void *)0x0) {
      FUN_0058ee20(pvVar3,*(undefined4 *)(param_2 + 4));
    }
  }
  return param_1;
}

