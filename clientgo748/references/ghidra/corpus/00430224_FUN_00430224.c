// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00430224 | Name: FUN_00430224


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
FUN_00430224(int *param_1,float param_2,float param_3,float param_4,float param_5,float param_6,
            float param_7,float param_8,float param_9,int *param_10,int param_11,int param_12,
            float param_13,float param_14)

{
  int iVar1;
  float local_38;
  float local_34;
  undefined1 local_30 [24];
  uint local_18;
  uint local_14;
  int local_10;
  float local_c;
  float local_8;
  
  local_8 = 256.0;
  local_c = 256.0;
  if (param_10 != (int *)0x0) {
    (**(code **)(*param_10 + 0x38))(param_10,0,local_30);
    local_8 = (float)local_18;
    local_c = (float)local_14;
  }
  param_1[0xa9c5] = (int)param_6;
  param_1[0xa9c6] = (int)param_7;
  param_1[0xa9cc] = (int)(param_8 * param_14 + param_6);
  param_1[0xa9cd] = (int)param_7;
  param_1[0xa9d3] = (int)(param_8 * param_14 + param_6);
  param_1[0xa9d4] = (int)(param_9 * param_14 + param_7);
  param_1[0xa9da] = (int)param_6;
  param_1[0xa9db] = (int)(param_9 * param_14 + param_7);
  if ((_DAT_005a4214 < param_13) || (param_13 < _DAT_005a4210)) {
    param_1[0xa9df] = (int)(param_2 / local_8);
    param_1[0xa9e0] = (int)((param_5 / _DAT_005a365c + param_3) / local_c);
    param_1[0xa9ca] = (int)((param_4 / _DAT_005a365c + param_2) / local_8);
    param_1[0xa9cb] = (int)(param_3 / local_c);
    param_1[0xa9d1] = (int)((param_2 + param_4) / local_8);
    param_1[0xa9d2] = (int)((param_5 / _DAT_005a365c + param_3) / local_c);
    param_1[0xa9d8] = (int)((param_4 / _DAT_005a365c + param_2) / local_8);
    param_1[0xa9d9] = (int)((param_3 + param_5) / local_c);
  }
  else {
    if (DAT_005ccf9c[0xa8e5] == 0) {
      local_34 = (param_2 + param_4) / local_8;
      local_38 = (param_3 + param_5) / local_c;
    }
    else {
      iVar1 = __ftol();
      local_34 = (float)iVar1 * _DAT_005a4208;
      iVar1 = __ftol();
      local_38 = (float)iVar1 * _DAT_005a4208;
    }
    param_1[0xa9ca] = (int)(_DAT_005a34a0 / local_8 + param_2 / local_8);
    param_1[0xa9cb] = (int)(_DAT_005a34a0 / local_c + param_3 / local_c);
    param_1[0xa9d1] = (int)(local_34 - _DAT_005a34a0 / local_8);
    param_1[0xa9d2] = (int)(_DAT_005a34a0 / local_c + param_3 / local_c);
    param_1[0xa9d8] = (int)(local_34 - _DAT_005a34a0 / local_8);
    param_1[0xa9d9] = (int)(local_38 - _DAT_005a34a0 / local_c);
    param_1[0xa9df] = (int)(_DAT_005a34a0 / local_8 + param_2 / local_8);
    param_1[0xa9e0] = (int)(local_38 - _DAT_005a34a0 / local_c);
  }
  for (local_10 = 0; local_10 < 4; local_10 = local_10 + 1) {
    param_1[local_10 * 7 + 0xa9c9] = param_11;
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
  (**(code **)(*param_1 + 0x38))(0,param_10);
  if (param_12 == 1) {
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

