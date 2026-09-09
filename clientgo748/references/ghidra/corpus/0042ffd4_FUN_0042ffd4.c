// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042ffd4 | Name: FUN_0042ffd4


void __thiscall
FUN_0042ffd4(int *param_1,float param_2,float param_3,float param_4,float param_5,int param_6,
            int param_7)

{
  int local_8;
  
  param_1[0xa9c5] = (int)param_2;
  param_1[0xa9c6] = (int)param_3;
  param_1[0xa9cc] = (int)(param_2 + param_4);
  param_1[0xa9cd] = (int)param_3;
  param_1[0xa9d3] = (int)(param_2 + param_4);
  param_1[0xa9d4] = (int)(param_3 + param_5);
  param_1[0xa9da] = (int)param_2;
  param_1[0xa9db] = (int)(param_3 + param_5);
  for (local_8 = 0; local_8 < 4; local_8 = local_8 + 1) {
    param_1[local_8 * 7 + 0xa9c9] = param_6;
  }
  if (DAT_005ccf9c[0xa8e5] == 1) {
    if (DAT_005ccf9c[0xa944] == 0x20) {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xff000000);
    }
    else {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xf000);
    }
  }
  else if (DAT_005ccf9c[0xa944] == 0x20) {
    (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xdd);
  }
  else {
    (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xd);
  }
  (**(code **)(*param_1 + 0x38))(0,0);
  if (param_7 == 1) {
    (**(code **)(*param_1 + 0x34))(0,4,4);
    (**(code **)(*param_1 + 0x30))(0x1b,1);
    (**(code **)(*param_1 + 0x30))(0x13,5);
    (**(code **)(*param_1 + 0x30))(0x14,6);
    (**(code **)(*param_1 + 0x30))(0x19,8);
    (**(code **)(*param_1 + 0x30))(0xf,0);
    (**(code **)(*param_1 + 0x30))(0xe,0);
  }
  else {
    (**(code **)(*param_1 + 0x30))(0x1b,0);
  }
  (**(code **)(*(int *)param_1[0xa8f9] + 0x130))(param_1[0xa8f9],0x144);
  (**(code **)(*(int *)param_1[0xa8f9] + 0x120))(param_1[0xa8f9],6,2,param_1 + 0xa9c5,0x1c);
  if (DAT_005ccf9c[0xa944] == 0x20) {
    (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xaa);
  }
  else {
    (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xff);
  }
  return;
}

