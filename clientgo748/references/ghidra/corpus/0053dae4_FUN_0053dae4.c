// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053dae4 | Name: FUN_0053dae4


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_0053dae4(int param_1)

{
  float fVar1;
  int iVar2;
  undefined4 uVar3;
  float local_30;
  float local_2c;
  float local_18;
  float local_14;
  float local_10;
  float local_8;
  
  iVar2 = FUN_00494dcf();
  if (iVar2 == 30000) {
    iVar2 = *(int *)(DAT_013b71e8 + 0x1b084);
    local_18 = *(float *)(iVar2 + 0x34);
    if (local_18 < _DAT_005a4104) {
      local_18 = 5.0;
    }
    local_14 = local_18 * _DAT_005a415c + local_18 * _DAT_005a415c;
    local_8 = *(float *)(iVar2 + 0x50) - *(float *)(iVar2 + 0x48);
    local_30 = *(float *)(iVar2 + 0x50) + *(float *)(iVar2 + 0x48);
    local_10 = 1.0;
    local_2c = 1.0;
    if (_DAT_005a45ec <= *(float *)(iVar2 + 0x2c)) {
      local_10 = 1.6;
      local_14 = 7.2;
      local_2c = 1.4;
      local_8 = local_8 - _DAT_005a365c;
      local_30 = local_30 + _DAT_005a365c;
    }
    fVar1 = *(float *)(param_1 + 0x80);
    if (((((local_10 * local_14) / 1.414 + local_8 + *(float *)(param_1 + 0x9c) <=
           *(float *)(param_1 + 0xa0)) ||
         (*(float *)(param_1 + 0xa0) <=
          (local_14 / 1.414 + local_8 + *(float *)(param_1 + 0x9c)) - local_14 * 1.414)) ||
        (*(float *)(param_1 + 0xa0) <= -*(float *)(param_1 + 0x9c) + local_30)) ||
       (local_10 * 1.414 * local_14 * local_2c + -*(float *)(param_1 + 0x9c) + local_30 <=
        *(float *)(param_1 + 0xa0))) {
      if (*(float *)(iVar2 + 0x2c) < _DAT_005a45ec) {
        if (((fVar1 + *(float *)(param_1 + 0xa0) <
              local_14 / 1.414 + local_8 + *(float *)(param_1 + 0x9c)) &&
            ((local_14 / 1.414 + local_8 + *(float *)(param_1 + 0x9c)) - local_14 * 1.414 <
             fVar1 + *(float *)(param_1 + 0xa0))) &&
           ((-*(float *)(param_1 + 0x9c) + local_30 < fVar1 + *(float *)(param_1 + 0xa0) &&
            (fVar1 + *(float *)(param_1 + 0xa0) <
             local_14 * 1.414 * local_2c + -*(float *)(param_1 + 0x9c) + local_30)))) {
          *(undefined4 *)(param_1 + 0x2c) = 1;
          return 1;
        }
        if (((*(float *)(param_1 + 0xa0) - fVar1 <
              local_14 / 1.414 + local_8 + *(float *)(param_1 + 0x9c)) &&
            ((local_14 / 1.414 + local_8 + *(float *)(param_1 + 0x9c)) - local_14 * 1.414 <
             *(float *)(param_1 + 0xa0) - fVar1)) &&
           ((-*(float *)(param_1 + 0x9c) + local_30 < *(float *)(param_1 + 0xa0) - fVar1 &&
            (*(float *)(param_1 + 0xa0) - fVar1 <
             local_14 * 1.414 * local_2c + -*(float *)(param_1 + 0x9c) + local_30)))) {
          *(undefined4 *)(param_1 + 0x2c) = 1;
          return 1;
        }
      }
      else if ((((*(float *)(param_1 + 0xa0) - fVar1 <
                  (local_10 * local_14) / 1.414 + local_8 + *(float *)(param_1 + 0x9c)) &&
                ((local_14 / 1.414 + local_8 + *(float *)(param_1 + 0x9c)) - local_14 * 1.414 <
                 *(float *)(param_1 + 0xa0) - fVar1)) &&
               (-*(float *)(param_1 + 0x9c) + local_30 < *(float *)(param_1 + 0xa0) - fVar1)) &&
              (*(float *)(param_1 + 0xa0) - fVar1 <
               local_10 * 1.414 * local_14 * local_2c + -*(float *)(param_1 + 0x9c) + local_30)) {
        *(undefined4 *)(param_1 + 0x2c) = 1;
        return 1;
      }
      *(undefined4 *)(param_1 + 0x2c) = 0;
      uVar3 = 0;
    }
    else {
      *(undefined4 *)(param_1 + 0x2c) = 1;
      uVar3 = 1;
    }
  }
  else {
    *(undefined4 *)(param_1 + 0x2c) = 1;
    uVar3 = 1;
  }
  return uVar3;
}

