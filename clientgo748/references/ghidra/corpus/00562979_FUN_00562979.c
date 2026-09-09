// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00562979 | Name: FUN_00562979


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00562979(float *param_1,float param_2,float param_3,float param_4,float param_5)

{
  float10 fVar1;
  float10 fVar2;
  
  fVar1 = (float10)fcos((float10)(param_2 * _DAT_005a67d4));
  fVar2 = (float10)fsin((float10)(param_2 * _DAT_005a67d4));
  *param_1 = ((float)fVar1 / (float)fVar2) / param_3;
  param_1[1] = 0.0;
  param_1[2] = 0.0;
  param_1[3] = 0.0;
  param_1[4] = 0.0;
  param_1[5] = (float)fVar1 / (float)fVar2;
  param_1[6] = 0.0;
  param_1[7] = 0.0;
  param_1[8] = 0.0;
  param_1[9] = 0.0;
  param_5 = param_5 / (param_5 - param_4);
  param_1[10] = param_5;
  param_1[0xb] = 1.0;
  param_1[0xc] = 0.0;
  param_1[0xd] = 0.0;
  param_1[0xe] = -(param_5 * param_4);
  param_1[0xf] = 0.0;
  return;
}

