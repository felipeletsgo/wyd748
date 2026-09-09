// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00562c38 | Name: FUN_00562c38


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float * FUN_00562c38(float *param_1,undefined4 param_2,float param_3)

{
  float fVar1;
  float10 fVar2;
  float10 fVar3;
  float local_18;
  float local_14;
  float local_10;
  
  thunk_FUN_005611dd(&local_18,param_2);
  fVar2 = (float10)fcos((float10)(param_3 * _DAT_005a67d4));
  fVar3 = (float10)fsin((float10)(param_3 * _DAT_005a67d4));
  param_1[3] = (float)fVar2;
  fVar1 = (float)fVar3;
  *param_1 = local_18 * fVar1;
  param_1[1] = local_14 * fVar1;
  param_1[2] = local_10 * fVar1;
  return param_1;
}

