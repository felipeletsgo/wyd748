// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059e74e | Name: FUN_0059e74e


undefined4 * __thiscall FUN_0059e74e(undefined4 *param_1,undefined4 *param_2)

{
  size_t sVar1;
  void *pvVar2;
  
  *param_1 = &PTR_FUN_005aa91c;
  sVar1 = _strlen((char *)*param_2);
  pvVar2 = operator_new(sVar1 + 1);
  param_1[1] = pvVar2;
  if (pvVar2 != (void *)0x0) {
    FUN_0058ee20(pvVar2,*param_2);
  }
  param_1[2] = 1;
  return param_1;
}

