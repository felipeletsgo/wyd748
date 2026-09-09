// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042fe6c | Name: FUN_0042fe6c


void __thiscall
FUN_0042fe6c(int *param_1,float param_2,float param_3,float param_4,float param_5,undefined4 param_6
            ,int param_7,int param_8,int param_9)

{
  int local_8;
  
  param_1[0xa9c5] = (int)param_2;
  param_1[0xa9c6] = (int)param_3;
  param_1[0xa9ca] = 0;
  param_1[0xa9cb] = 0;
  param_1[0xa9cc] = (int)(param_2 + param_4);
  param_1[0xa9cd] = (int)param_3;
  param_1[0xa9d1] = param_8;
  param_1[0xa9d2] = 0;
  param_1[0xa9d3] = (int)(param_2 + param_4);
  param_1[0xa9d4] = (int)(param_3 + param_5);
  param_1[0xa9d8] = param_8;
  param_1[0xa9d9] = param_9;
  param_1[0xa9da] = (int)param_2;
  param_1[0xa9db] = (int)(param_3 + param_5);
  param_1[0xa9df] = 0;
  param_1[0xa9e0] = param_9;
  for (local_8 = 0; local_8 < 4; local_8 = local_8 + 1) {
    param_1[local_8 * 7 + 0xa9c9] = param_7;
  }
  (**(code **)(*param_1 + 0x38))(0,param_6);
  (**(code **)(*(int *)param_1[0xa8f9] + 0x130))(param_1[0xa8f9],0x144);
  (**(code **)(*(int *)param_1[0xa8f9] + 0x120))(param_1[0xa8f9],6,2,param_1 + 0xa9c5,0x1c);
  return;
}

