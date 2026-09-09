// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042eeea | Name: FUN_0042eeea


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0042eeea(int *param_1,int param_2,uint param_3,undefined4 param_4)

{
  float fVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  undefined4 uVar6;
  float local_9c;
  float local_98;
  int local_8c;
  float local_84;
  float local_80;
  undefined4 local_7c;
  uint auStack_78 [4];
  float local_68;
  float local_64;
  undefined4 local_60;
  undefined4 local_54;
  undefined4 local_50;
  float local_4c;
  float local_48;
  undefined4 local_44;
  undefined4 local_38;
  undefined4 local_34;
  float local_30;
  float local_2c;
  undefined4 local_28;
  undefined4 local_1c;
  undefined4 local_18;
  float local_14;
  uint local_10;
  undefined4 local_c;
  uint local_8;
  
  FUN_00421910(&local_84,0x1c,4,FUN_004cb8e0);
  if (param_1[0xac15] == 0) {
    iVar2 = (**(code **)(*DAT_0092e654 + 8))();
    param_1[0xac15] = iVar2;
  }
  iVar2 = (**(code **)(*DAT_0092e654 + 8))();
  local_10 = iVar2 - param_1[0xac15];
  local_c = 2000;
  local_8 = local_10 % 2000;
  fVar1 = (float)local_8 * _DAT_005a41f8 - _DAT_005a34a0;
  FUN_00423b00((fVar1 + fVar1) * _DAT_005a4148);
  uVar3 = __ftol();
  uVar4 = __ftol();
  uVar5 = __ftol();
  for (local_8c = 0; local_8c < 4; local_8c = local_8c + 1) {
    auStack_78[local_8c * 7] = 0x3f800000;
    auStack_78[local_8c * 7 + 1] =
         param_3 & 0xff000000 | (uVar3 & 0xff) << 0x10 | (uVar4 & 0xff) << 8 | uVar5 & 0xff;
  }
  local_14 = *(float *)(param_2 + 4);
  local_2c = *(float *)(param_2 + 8);
  local_9c = *(float *)(param_2 + 0x154) * _DAT_005a34a0 * *(float *)(param_2 + 0x1c);
  local_98 = *(float *)(param_2 + 0x158) * _DAT_005a34a0 * *(float *)(param_2 + 0x1c);
  if (param_3 == 0xffffffff) {
    local_9c = local_9c + local_9c;
    local_98 = local_98 + local_98;
  }
  local_84 = local_14 - local_9c;
  local_80 = local_2c - local_98;
  local_7c = 0x3dcccccd;
  auStack_78[2] = 0;
  auStack_78[3] = 0;
  local_68 = local_14 + local_9c;
  local_64 = local_2c - local_98;
  local_60 = 0x3dcccccd;
  local_54 = 0x3f800000;
  local_50 = 0;
  local_4c = local_14 - local_9c;
  local_48 = local_2c + local_98;
  local_44 = 0x3dcccccd;
  local_38 = 0;
  local_34 = 0x3f800000;
  local_30 = local_14 + local_9c;
  local_2c = local_2c + local_98;
  local_28 = 0x3dcccccd;
  local_1c = 0x3f800000;
  local_18 = 0x3f800000;
  uVar6 = FUN_004b925c(param_4,360000);
  (**(code **)(*param_1 + 0x38))(0,uVar6);
  (**(code **)(*param_1 + 0x30))(0x1b,1);
  (**(code **)(*param_1 + 0x30))(0x13,5);
  (**(code **)(*param_1 + 0x30))(0x14,2);
  (**(code **)(*param_1 + 0x34))(0,4,2);
  (**(code **)(*param_1 + 0x34))(0,5,2);
  (**(code **)(*(int *)param_1[0xa8f9] + 0x130))(param_1[0xa8f9],0x144);
  (**(code **)(*(int *)param_1[0xa8f9] + 0x120))(param_1[0xa8f9],5,2,&local_84,0x1c);
  (**(code **)(*param_1 + 0x30))(0x1b,0);
  return;
}

