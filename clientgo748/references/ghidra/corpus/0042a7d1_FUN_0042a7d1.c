// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042a7d1 | Name: FUN_0042a7d1


undefined4 * __thiscall
FUN_0042a7d1(undefined4 *param_1,int param_2,undefined4 param_3,uint param_4,undefined4 param_5)

{
  void *pvVar1;
  uint local_8;
  
  *param_1 = &PTR_FUN_005a4184;
  pvVar1 = operator_new(param_4 << 2);
  param_1[1] = pvVar1;
  for (local_8 = 0; local_8 < param_4; local_8 = local_8 + 1) {
    *(undefined4 *)(param_1[1] + local_8 * 4) = *(undefined4 *)(param_2 + local_8 * 4);
  }
  param_1[2] = param_3;
  param_1[4] = param_4;
  param_1[3] = param_5;
  FUN_0042a97c(*(undefined4 *)param_1[1],0);
  for (local_8 = 0; local_8 < param_4; local_8 = local_8 + 1) {
    (**(code **)(**(int **)(param_1[1] + local_8 * 4) + 0x34))
              (*(undefined4 *)(param_1[1] + local_8 * 4),0);
  }
  return param_1;
}

