// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004bbcb5 | Name: FUN_004bbcb5


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall
FUN_004bbcb5(int *param_1,int param_2,int param_3,float param_4,float param_5,uint param_6,
            int param_7,undefined4 param_8,short param_9)

{
  undefined4 uVar1;
  float10 fVar2;
  int local_13c;
  int local_138;
  int local_134;
  undefined1 local_12c [64];
  float local_ec;
  float local_e8;
  float local_e4;
  float local_e0;
  float local_dc;
  float local_d8;
  float local_d4;
  undefined4 local_d0;
  float local_cc;
  undefined1 local_c8 [64];
  float local_88;
  float local_84;
  float local_80;
  undefined4 local_7c;
  float local_68;
  float local_64;
  float local_60;
  undefined4 local_5c;
  float local_58;
  float local_54;
  float local_50;
  undefined4 local_48;
  undefined1 local_44 [64];
  
  local_cc = 6.68;
  if ((float)(uint)DAT_005ccf9c[0xa975] / (float)DAT_005ccf9c[0xa976] < _DAT_005a46cc) {
    local_cc = 6.26;
  }
  local_e4 = (float)param_2 / ((float)(uint)DAT_005ccf9c[0xa975] / local_cc) -
             local_cc / _DAT_005a365c;
  local_e8 = (float)param_3 / ((float)(uint)DAT_005ccf9c[0xa976] / _DAT_005a46c8) - _DAT_005a46c4;
  fVar2 = (float10)FUN_00493c30(local_e4,0x42480000);
  local_ec = (float)fVar2;
  fVar2 = (float10)FUN_00493c30(local_e8,0x42480000);
  local_e0 = (float)fVar2;
  FUN_00430f10();
  FUN_00430f10();
  FUN_00430f10();
  thunk_FUN_0056230d(local_12c,_DAT_005a46c0 * param_5,_DAT_005a46c0 * param_5,
                     _DAT_005a46c0 * param_5);
  thunk_FUN_0056239d(local_c8,-local_e4 - (float)param_1[0xcd] * _DAT_005a46c0 * param_5,
                     -local_e8 - (float)param_1[0xcf] * _DAT_005a46c0 * param_5,
                     _DAT_005a3534 - (float)param_1[0xce] * _DAT_005a46c0 * param_5);
  thunk_FUN_00562821(local_44,local_ec + param_4,_DAT_005a46bc + local_e0,0);
  thunk_FUN_00561a5b(local_44,DAT_005ccf9c + 0xa99b,local_44);
  thunk_FUN_00561a5b(local_44,local_44,local_12c);
  thunk_FUN_00561a5b(local_44,local_44,local_c8);
  (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x94))(DAT_005ccf9c[0xa8f9],0x100,local_44);
  local_dc = (float)(param_6 >> 0x10 & 0xff) / _DAT_005a41e4;
  local_d8 = (float)(param_6 >> 8 & 0xff) / _DAT_005a41e4;
  local_d4 = (float)(param_6 & 0xff) / _DAT_005a41e4;
  _memset(&local_88,0,0x44);
  local_58 = local_dc * _DAT_005a40ec + _DAT_005a4214;
  local_54 = local_dc * _DAT_005a40ec + _DAT_005a4214;
  local_50 = local_dc * _DAT_005a40ec + _DAT_005a4214;
  local_88 = local_dc;
  local_84 = local_d8;
  local_80 = local_d4;
  local_7c = local_d0;
  local_68 = local_dc;
  local_64 = local_d8;
  local_60 = local_d4;
  local_5c = local_d0;
  local_48 = 0;
  (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0xa8))(DAT_005ccf9c[0xa8f9],&local_88);
  if ((param_7 < 1) && ((param_9 < 4 || (8 < param_9)))) {
    (**(code **)(*param_1 + 0xc))(0,param_8);
  }
  else {
    (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,1);
    if (param_9 == 4) {
      if (((DAT_005ccf9c[0xa8e4] == 0) && (DAT_005ccf9c[0xa8e8] == 0)) &&
         (DAT_005ccf9c[0xa8e6] == 0)) {
        uVar1 = FUN_004b925c(param_7 + 0xb3,360000);
        (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar1);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,1,5);
        (**(code **)(*DAT_005ccf9c + 0x34))(1,1,4);
      }
      else {
        uVar1 = FUN_004b925c(param_7 + 0xb3,360000);
        (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar1);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
        (**(code **)(*DAT_005ccf9c + 0x34))(1,1,4);
      }
    }
    else if ((param_9 < 5) || (8 < param_9)) {
      if ((param_9 < 9) || (0xc < param_9)) {
        if ((param_9 < 0x74) || (0x7d < param_9)) {
          if ((DAT_005ccf9c[0xa8e4] == 0) &&
             (((DAT_005ccf9c[0xa8e8] == 0 && (DAT_005ccf9c[0xa8e6] == 0)) &&
              (DAT_005ccf9c[0xa972] == 0)))) {
            uVar1 = FUN_004b925c(*(int *)(s_Wrong_Match_Time_Clock___l____l__005ba9e4 +
                                         param_7 * 4 + 0x20) + 0xe9,360000);
            (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar1);
            if (param_7 < 7) {
              (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
              (**(code **)(*DAT_005ccf9c + 0x34))(1,1,0xb);
            }
            else {
              (**(code **)(*DAT_005ccf9c + 0x34))(0,1,5);
              (**(code **)(*DAT_005ccf9c + 0x34))(1,1,0xb);
            }
          }
          else {
            uVar1 = FUN_004b925c(param_7 + 0xe9,360000);
            (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar1);
            (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
            (**(code **)(*DAT_005ccf9c + 0x34))(1,1,7);
          }
        }
        else {
          local_13c = 0x99;
          if (param_9 == 0x74) {
            local_13c = 0x113;
          }
          if (param_9 == 0x75) {
            local_13c = 0x120;
          }
          if (param_9 == 0x76) {
            local_13c = 0x12d;
          }
          if (param_9 == 0x77) {
            local_13c = 0x13a;
          }
          if (param_9 == 0x78) {
            local_13c = 0x147;
          }
          if (param_9 == 0x79) {
            local_13c = 0x154;
          }
          if (param_9 == 0x7a) {
            local_13c = 0x161;
          }
          if (param_9 == 0x7b) {
            local_13c = 0x16e;
          }
          if (param_9 == 0x7c) {
            local_13c = 0x1a9;
          }
          if (param_9 == 0x7d) {
            local_13c = 0x188;
          }
          uVar1 = FUN_004b925c(local_13c + -1 + param_7,360000);
          (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar1);
          if (((DAT_005ccf9c[0xa8e4] == 0) && (DAT_005ccf9c[0xa8e8] == 0)) &&
             ((DAT_005ccf9c[0xa8e6] == 0 && (DAT_005ccf9c[0xa972] == 0)))) {
            (**(code **)(*DAT_005ccf9c + 0x34))(0,3,1);
            (**(code **)(*DAT_005ccf9c + 0x34))(0,1,0x18);
            (**(code **)(*DAT_005ccf9c + 0x34))(1,1,6);
          }
          else {
            (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
            (**(code **)(*DAT_005ccf9c + 0x34))(1,1,7);
          }
        }
      }
      else {
        local_138 = 0x99;
        if (param_9 == 10) {
          local_138 = 0xa6;
        }
        if (param_9 == 0xb) {
          local_138 = 0xf6;
        }
        if (param_9 == 0xc) {
          local_138 = 0x104;
        }
        if (((DAT_005ccf9c[0xa8e4] == 0) && (DAT_005ccf9c[0xa8e8] == 0)) &&
           (DAT_005ccf9c[0xa8e6] == 0)) {
          uVar1 = FUN_004b925c(local_138 + param_7,360000);
          (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar1);
          (**(code **)(*DAT_005ccf9c + 0x34))(0,1,5);
          (**(code **)(*DAT_005ccf9c + 0x34))(1,1,0xb);
        }
        else {
          uVar1 = FUN_004b925c(local_138 + param_7,360000);
          (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar1);
          (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
          (**(code **)(*DAT_005ccf9c + 0x34))(1,1,7);
        }
      }
    }
    else {
      local_134 = 0x99;
      if (param_9 == 6) {
        local_134 = 0xa6;
      }
      if (param_9 == 7) {
        local_134 = 0xf6;
      }
      if (param_9 == 8) {
        local_134 = 0x104;
      }
      if (((DAT_005ccf9c[0xa8e4] == 0) && (DAT_005ccf9c[0xa8e8] == 0)) &&
         (DAT_005ccf9c[0xa8e6] == 0)) {
        uVar1 = FUN_004b925c(local_134 + param_7,360000);
        (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar1);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
        (**(code **)(*DAT_005ccf9c + 0x34))(1,1,0xb);
      }
      else {
        uVar1 = FUN_004b925c(local_134 + param_7,360000);
        (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar1);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
        (**(code **)(*DAT_005ccf9c + 0x34))(1,1,7);
      }
    }
    (**(code **)(*param_1 + 0xc))(1,param_8);
    (**(code **)(*DAT_005ccf9c + 0x38))(1,0);
    (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,1);
    (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
    (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
  }
  return 1;
}

