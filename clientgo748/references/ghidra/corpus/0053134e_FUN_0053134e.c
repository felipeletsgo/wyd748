// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053134e | Name: FUN_0053134e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_0053134e(int *param_1)

{
  float fVar1;
  int iVar2;
  int iVar3;
  float10 fVar4;
  float local_30;
  float local_2c;
  float local_28;
  undefined4 local_24;
  float local_20;
  float local_1c;
  float local_18;
  float local_14;
  float local_10;
  uint local_c;
  uint local_8;
  
  if (param_1[0x18] != 0) {
    iVar2 = (**(code **)(*DAT_0092e654 + 8))();
    local_c = (uint)param_1 % 100;
    local_8 = __ftol();
    fVar1 = ((float)(((iVar2 - param_1[0x6b]) + local_8) % 40000) / _DAT_005a57f0) * _DAT_005a4148;
    fVar4 = (float10)FUN_00423b00(fVar1 + fVar1);
    local_10 = (float)fVar4;
    if (param_1[0x65] == 0) {
      fVar4 = (float10)FUN_00423b00((float)local_8 * _DAT_005a439c +
                                    local_10 * _DAT_005a4148 * (float)param_1[0x66]);
      local_14 = (float)fVar4;
      param_1[10] = (int)(local_10 * (float)param_1[0x67] * _DAT_005a4154 + (float)param_1[0x68]);
      param_1[0xb] = (int)(local_14 * (float)param_1[0x67] * _DAT_005a40ec + (float)param_1[0x6a]);
      fVar4 = (float10)FUN_00493c30(param_1[10],param_1[0xb]);
      param_1[0xd] = (int)(float)fVar4;
    }
    else if (param_1[0x65] == 1) {
      fVar4 = (float10)FUN_00493bf0((float)local_8 * _DAT_005a439c +
                                    local_10 * _DAT_005a4148 * (float)param_1[0x66]);
      local_18 = (float)fVar4;
      param_1[10] = (int)(local_18 * (float)param_1[0x67] * _DAT_005a4154 + (float)param_1[0x68]);
      param_1[0xb] = (int)(local_10 * (float)param_1[0x67] * _DAT_005a40ec + (float)param_1[0x6a]);
      fVar4 = (float10)FUN_00493c30(param_1[10],param_1[0xb]);
      param_1[0xd] = (int)(float)fVar4;
    }
    else if (param_1[0x65] == 2) {
      fVar4 = (float10)FUN_00423b00((float)local_8 * _DAT_005a439c +
                                    (local_10 + local_10) * _DAT_005a4148);
      local_1c = (float)fVar4;
      fVar4 = (float10)FUN_00493bf0((float)local_8 * _DAT_005a439c +
                                    (local_10 + local_10) * _DAT_005a4148);
      local_20 = (float)fVar4;
      param_1[10] = (int)(local_20 * (float)param_1[0x67] * _DAT_005a4214 + (float)param_1[0x68]);
      param_1[0xb] = (int)(local_1c * (float)param_1[0x67] * _DAT_005a4214 + (float)param_1[0x6a]);
      fVar4 = (float10)FUN_00493c30(param_1[10],param_1[0xb]);
      param_1[0xd] = (int)(float)fVar4;
    }
    (**(code **)(*param_1 + 0x48))(0,param_1[0xd],0);
    (**(code **)(*param_1 + 0x44))(param_1[10],param_1[0xc],param_1[0xb]);
    iVar3 = (**(code **)(*param_1 + 0x4c))();
    if (iVar3 == 1) {
      FUN_0049cb58(&local_30,param_1[10],param_1[0xb]);
      local_30 = *(float *)(DAT_005ccf9c + 0x2a6e4) * _DAT_005a34a0 + local_30 * _DAT_005a4158;
      local_2c = *(float *)(DAT_005ccf9c + 0x2a6e8) * _DAT_005a34a0 + local_2c * _DAT_005a4158;
      local_28 = *(float *)(DAT_005ccf9c + 0x2a6ec) * _DAT_005a34a0 + local_28 * _DAT_005a4158;
      *(undefined4 *)(param_1[0x18] + 0x298) = 0;
      *(undefined4 *)(param_1[0x18] + 0x29c) = 0;
      *(undefined4 *)(param_1[0x18] + 0x2a0) = 0;
      iVar3 = param_1[0x18];
      *(float *)(iVar3 + 0x288) = local_30;
      *(float *)(iVar3 + 0x28c) = local_2c;
      *(float *)(iVar3 + 0x290) = local_28;
      *(undefined4 *)(iVar3 + 0x294) = local_24;
      *(undefined4 *)(param_1[0x18] + 0x2a8) = 0x3f800000;
      *(undefined4 *)(param_1[0x18] + 0x2ac) = 0x3f800000;
      *(undefined4 *)(param_1[0x18] + 0x2b0) = 0x3f800000;
      *(float *)(param_1[0x18] + 0x2b8) =
           local_30 * _DAT_005a4154 + *(float *)(DAT_005ccf9c + 0x2a6f4) * _DAT_005a4154;
      *(float *)(param_1[0x18] + 700) =
           local_30 * _DAT_005a4154 + *(float *)(DAT_005ccf9c + 0x2a6f8) * _DAT_005a4154;
      *(float *)(param_1[0x18] + 0x2c0) =
           local_30 * _DAT_005a4154 + *(float *)(DAT_005ccf9c + 0x2a6fc) * _DAT_005a4154;
      FUN_004bda22(iVar2 + local_8);
    }
  }
  return 1;
}

