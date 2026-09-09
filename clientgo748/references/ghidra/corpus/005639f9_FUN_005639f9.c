// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005639f9 | Name: FUN_005639f9


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float * FUN_005639f9(float *param_1,undefined4 param_2,int *param_3,undefined4 *param_4,
                    undefined4 *param_5,undefined4 *param_6)

{
  float fVar1;
  float fVar2;
  undefined4 *puVar3;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  
  puVar3 = &local_50;
  switch(((param_6 != (undefined4 *)0x0) << 1 | param_5 != (undefined4 *)0x0) << 1 |
         param_4 != (undefined4 *)0x0) {
  case '\0':
    local_18 = 0;
    local_1c = 0;
    local_20 = 0;
    local_24 = 0;
    local_2c = 0;
    local_30 = 0;
    local_34 = 0;
    local_38 = 0;
    local_40 = 0;
    local_44 = 0;
    local_48 = 0;
    local_4c = 0;
    local_14 = 0x3f800000;
    local_28 = 0x3f800000;
    local_3c = 0x3f800000;
    local_50 = 0x3f800000;
    break;
  case '\x01':
    puVar3 = param_4;
    break;
  case '\x02':
    puVar3 = param_5;
    break;
  case '\x03':
    param_6 = param_5;
    goto LAB_00563aca;
  case '\x04':
    puVar3 = param_6;
    break;
  case '\x05':
    goto LAB_00563aca;
  case '\x06':
    param_4 = param_5;
    goto LAB_00563aca;
  case '\a':
    thunk_FUN_00561a5b(&local_50,param_6,param_5);
    param_6 = &local_50;
LAB_00563aca:
    thunk_FUN_00561a5b(&local_50,param_6,param_4);
  }
  thunk_FUN_00561462(param_1,param_2,puVar3);
  if (param_3 != (int *)0x0) {
    fVar2 = (float)param_3[2];
    if (param_3[2] < 0) {
      fVar2 = fVar2 + _DAT_005a67dc;
    }
    fVar1 = (float)*param_3;
    if (*param_3 < 0) {
      fVar1 = fVar1 + _DAT_005a67dc;
    }
    *param_1 = fVar1 + fVar2 * (*param_1 + _DAT_005a67d8) * _DAT_005a67d4;
    fVar2 = (float)param_3[3];
    if (param_3[3] < 0) {
      fVar2 = fVar2 + _DAT_005a67dc;
    }
    fVar1 = (float)param_3[1];
    if (param_3[1] < 0) {
      fVar1 = fVar1 + _DAT_005a67dc;
    }
    param_1[1] = fVar1 + fVar2 * (1.0 - param_1[1]) * _DAT_005a67d4;
    param_1[2] = ((float)param_3[5] - (float)param_3[4]) * param_1[2] + (float)param_3[4];
  }
  return param_1;
}

