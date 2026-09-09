// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053ceb1 | Name: FUN_0053ceb1


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_0053ceb1(int param_1)

{
  bool bVar1;
  int iVar2;
  float *pfVar3;
  float10 fVar4;
  undefined1 local_60 [16];
  undefined1 local_50 [16];
  undefined1 local_40 [16];
  undefined4 local_30;
  float local_2c;
  float local_28;
  float local_24;
  float local_20;
  int local_1c;
  float local_18;
  undefined1 local_14 [8];
  int local_c;
  float local_8;
  
  if (DAT_005d03f0 == 1) {
    return 0;
  }
  if ((((*(int *)(param_1 + 0x18) != -1) && (*(int *)(param_1 + 0x18) != DAT_005bdef0)) &&
      (*(int *)(param_1 + 0x18) != DAT_005bdef4)) &&
     ((*(int *)(param_1 + 0x18) != DAT_005bdef8 && (*(int *)(param_1 + 0x18) != DAT_005bdefc)))) {
    return 0;
  }
  local_1c = 0x1c;
  FUN_00430f20(*(undefined4 *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x48),
               *(undefined4 *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x50));
  fVar4 = (float10)FUN_0053d6c0(local_14);
  local_18 = (float)fVar4;
  local_8 = (float)local_1c;
  if (local_18 <= _DAT_005a59bc) {
    local_c = 0x14;
    if ((*(int *)(param_1 + 0x60) != 0) && (*(int *)(param_1 + 0x48) == 1)) {
      local_30 = DAT_0067cf38;
      FUN_0049cb58(&local_2c,*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x2c));
      local_2c = 0.0;
      local_28 = 0.0;
      local_24 = 0.0;
      iVar2 = *(int *)(param_1 + 0x60);
      *(undefined4 *)(iVar2 + 0x288) = 0;
      *(undefined4 *)(iVar2 + 0x28c) = 0;
      *(undefined4 *)(iVar2 + 0x290) = 0;
      *(float *)(iVar2 + 0x294) = local_20;
      *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2a8) = 0x3f800000;
      *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2ac) = 0x3f800000;
      *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2b0) = 0x3f800000;
      iVar2 = __ftol();
      if ((iVar2 >> 7 < 0x1b) ||
         (((iVar2 = __ftol(), 0x1e < iVar2 >> 7 || (iVar2 = __ftol(), iVar2 >> 7 < 0x15)) ||
          (iVar2 = __ftol(), 0x18 < iVar2 >> 7)))) {
        bVar1 = false;
      }
      else {
        bVar1 = true;
      }
      if (bVar1) {
        pfVar3 = (float *)FUN_0049cb58(local_40,*(undefined4 *)(param_1 + 0x28),
                                       *(undefined4 *)(param_1 + 0x2c));
        *(float *)(*(int *)(param_1 + 0x60) + 0x2b8) =
             (float)DAT_005ccf9c[0xa9b9] * _DAT_005a34a0 + *pfVar3 * _DAT_005a34a0;
        iVar2 = FUN_0049cb58(local_50,*(undefined4 *)(param_1 + 0x28),
                             *(undefined4 *)(param_1 + 0x2c));
        *(float *)(*(int *)(param_1 + 0x60) + 700) =
             (float)DAT_005ccf9c[0xa9ba] * _DAT_005a34a0 + *(float *)(iVar2 + 4) * _DAT_005a34a0;
        iVar2 = FUN_0049cb58(local_60,*(undefined4 *)(param_1 + 0x28),
                             *(undefined4 *)(param_1 + 0x2c));
        *(float *)(*(int *)(param_1 + 0x60) + 0x2c0) =
             (float)DAT_005ccf9c[0xa9bb] * _DAT_005a34a0 + *(float *)(iVar2 + 8) * _DAT_005a34a0;
      }
      else {
        *(float *)(*(int *)(param_1 + 0x60) + 0x2b8) =
             (float)DAT_005ccf9c[0xa9b9] * _DAT_005a415c + local_2c * _DAT_005a4158;
        *(float *)(*(int *)(param_1 + 0x60) + 700) =
             (float)DAT_005ccf9c[0xa9ba] * _DAT_005a415c + local_28 * _DAT_005a4158;
        *(float *)(*(int *)(param_1 + 0x60) + 0x2c0) =
             (float)DAT_005ccf9c[0xa9bb] * _DAT_005a415c + local_24 * _DAT_005a4158;
      }
      if (local_18 <= (float)(local_c * local_c)) {
        local_20 = 1.0;
      }
      else {
        local_20 = _DAT_005a3660 -
                   (local_18 - (float)(local_c * local_c)) /
                   (float)(local_1c * local_1c - local_c * local_c);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,4,4);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,5,2);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,6,0);
      }
      iVar2 = *(int *)(param_1 + 0x60);
      *(float *)(iVar2 + 0x288) = local_2c;
      *(float *)(iVar2 + 0x28c) = local_28;
      *(float *)(iVar2 + 0x290) = local_24;
      *(float *)(iVar2 + 0x294) = local_20;
      (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
      FUN_004be1ef(0,0x3f800000,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x13,2);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x19,7);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
      if ((float)(local_c * local_c) < local_18) {
        (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,5,2);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,6,1);
      }
    }
    return 1;
  }
  return 1;
}

