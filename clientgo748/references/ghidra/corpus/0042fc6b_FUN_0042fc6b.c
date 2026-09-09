// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042fc6b | Name: FUN_0042fc6b


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
FUN_0042fc6b(int *param_1,float param_2,float param_3,float param_4,float param_5,float param_6,
            float param_7,undefined4 param_8,int param_9,float param_10,float param_11)

{
  int local_8;
  
  param_1[0xa9c5] = (int)param_6;
  param_1[0xa9c6] = (int)param_7;
  param_1[0xa9ca] = (int)(param_2 / (float)DAT_005b4918);
  param_1[0xa9cb] = (int)(param_3 / (float)((int)(DAT_005b4918 + (DAT_005b4918 >> 0x1f & 7U)) >> 3))
  ;
  param_1[0xa9cc] = (int)(param_4 * param_10 + param_6);
  param_1[0xa9cd] = (int)param_7;
  param_1[0xa9d1] = (int)((param_2 + param_4 + _DAT_005a34a0) / (float)DAT_005b4918);
  param_1[0xa9d2] =
       (int)((param_3 + _DAT_005a34a0) /
            (float)((int)(DAT_005b4918 + (DAT_005b4918 >> 0x1f & 7U)) >> 3));
  param_1[0xa9d3] = (int)(param_4 * param_10 + param_6);
  param_1[0xa9d4] = (int)(param_5 * param_11 + param_7);
  param_1[0xa9d8] = (int)((param_2 + param_4 + _DAT_005a34a0) / (float)DAT_005b4918);
  param_1[0xa9d9] =
       (int)((param_3 + param_5 + _DAT_005a34a0) /
            (float)((int)(DAT_005b4918 + (DAT_005b4918 >> 0x1f & 7U)) >> 3));
  param_1[0xa9da] = (int)param_6;
  param_1[0xa9db] = (int)(param_5 * param_11 + param_7);
  param_1[0xa9df] = (int)(param_2 / (float)DAT_005b4918);
  param_1[0xa9e0] =
       (int)((param_3 + param_5 + _DAT_005a34a0) /
            (float)((int)(DAT_005b4918 + (DAT_005b4918 >> 0x1f & 7U)) >> 3));
  for (local_8 = 0; local_8 < 4; local_8 = local_8 + 1) {
    param_1[local_8 * 7 + 0xa9c9] = param_9;
  }
  (**(code **)(*param_1 + 0x38))(0,param_8);
  (**(code **)(*(int *)param_1[0xa8f9] + 0x130))(param_1[0xa8f9],0x144);
  (**(code **)(*(int *)param_1[0xa8f9] + 0x120))(param_1[0xa8f9],6,2,param_1 + 0xa9c5,0x1c);
  return;
}

