// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042359c | Name: FUN_0042359c


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_0042359c(int param_1)

{
  float fVar1;
  float fVar2;
  uint uVar3;
  undefined4 uVar4;
  float10 fVar5;
  
  uVar3 = (**(code **)(*DAT_0092e654 + 8))();
  if (uVar3 < *(uint *)(param_1 + 200)) {
    *(undefined4 *)(param_1 + 0x28c) = 0;
  }
  else if (*(int *)(param_1 + 0xcc) != 0) {
    fVar1 = (float)(uVar3 - *(int *)(param_1 + 200)) / (float)*(int *)(param_1 + 0xcc);
    if (*(int *)(param_1 + 0x290) == 0) {
      if (_DAT_005a40ec <= fVar1) {
        *(undefined4 *)(param_1 + 0x28c) = 1;
      }
      else {
        *(undefined4 *)(param_1 + 0x28c) = 0;
      }
      *(float *)(param_1 + 0xd0) = (fVar1 - _DAT_005a40ec) / _DAT_005a4160;
      if (_DAT_005a415c <= *(float *)(param_1 + 0xd0)) {
        *(float *)(param_1 + 0xd0) = (*(float *)(param_1 + 0xd0) - _DAT_005a415c) / _DAT_005a4158;
      }
      else {
        *(undefined4 *)(param_1 + 0xd0) = 0;
      }
    }
    else {
      if (_DAT_005a4154 <= fVar1) {
        if ((fVar1 <= _DAT_005a4154) || (_DAT_005a4160 <= fVar1)) {
          *(undefined4 *)(param_1 + 0x28c) = 0;
        }
        else {
          *(undefined4 *)(param_1 + 0x28c) = 1;
        }
      }
      else {
        *(undefined4 *)(param_1 + 0x28c) = 0;
      }
      *(float *)(param_1 + 0xd0) = (fVar1 - _DAT_005a4154) / _DAT_005a40ec;
      if (_DAT_005a415c <= *(float *)(param_1 + 0xd0)) {
        *(float *)(param_1 + 0xd0) = (*(float *)(param_1 + 0xd0) - _DAT_005a415c) / _DAT_005a4158;
      }
      else {
        *(undefined4 *)(param_1 + 0xd0) = 0;
      }
    }
    if (*(int *)(param_1 + 0xac) == 5) {
      if (_DAT_005a4150 <= *(float *)(param_1 + 0xd0)) {
        *(undefined4 *)(param_1 + 0xd4) = 0x3f800000;
      }
      else {
        fVar2 = _DAT_005a4150 - *(float *)(param_1 + 0xd0);
        *(float *)(param_1 + 0xd4) = fVar2 + fVar2 + _DAT_005a3660;
      }
    }
    else if (*(int *)(param_1 + 0xac) == 6) {
      if (_DAT_005a4150 <= *(float *)(param_1 + 0xd0)) {
        *(undefined4 *)(param_1 + 0xd4) = 0x3f19999a;
      }
      else {
        fVar2 = _DAT_005a4150 - *(float *)(param_1 + 0xd0);
        *(float *)(param_1 + 0xd4) = fVar2 + fVar2 + _DAT_005a414c;
      }
    }
    if (_DAT_005a3660 <= fVar1) {
      (**(code **)(*DAT_013b71e8 + 100))(param_1,*(int *)(param_1 + 0xcc),0);
    }
    else {
      if ((*(int *)(param_1 + 0xac) == 5) || (*(int *)(param_1 + 0xac) == 6)) {
        uVar4 = __ftol();
        *(undefined4 *)(param_1 + 0xbc) = uVar4;
      }
      else {
        if (((*(short *)(param_1 + 0xb4) == 1) || (*(short *)(param_1 + 0xb4) == 5)) ||
           (*(short *)(param_1 + 0xb4) == 7)) {
          uVar4 = __ftol();
          *(undefined4 *)(param_1 + 0xbc) = uVar4;
        }
        if (((*(short *)(param_1 + 0xb4) == 2) || (*(short *)(param_1 + 0xb4) == 6)) ||
           (*(short *)(param_1 + 0xb4) == 8)) {
          uVar4 = __ftol();
          *(undefined4 *)(param_1 + 0xbc) = uVar4;
        }
        if (((*(short *)(param_1 + 0xb4) == 3) || (*(short *)(param_1 + 0xb4) == 5)) ||
           (*(short *)(param_1 + 0xb4) == 8)) {
          uVar4 = __ftol();
          *(undefined4 *)(param_1 + 0xb8) = uVar4;
        }
        if (((*(short *)(param_1 + 0xb4) == 4) || (*(short *)(param_1 + 0xb4) == 6)) ||
           (*(short *)(param_1 + 0xb4) == 7)) {
          uVar4 = __ftol();
          *(undefined4 *)(param_1 + 0xb8) = uVar4;
        }
        if (*(short *)(param_1 + 0xb4) == 9) {
          *(int *)(param_1 + 0xb8) = *(int *)(param_1 + 0xc0) + -0x32;
          uVar4 = __ftol();
          *(undefined4 *)(param_1 + 0xbc) = uVar4;
        }
      }
      if ((*(int *)(param_1 + 0xac) == 5) || (*(int *)(param_1 + 0xac) == 6)) {
        fVar5 = (float10)FUN_00423b00(fVar1 * _DAT_005a4148);
        *(float *)(param_1 + 0x298) = (float)(fVar5 * (float10)*(float *)(param_1 + 0xd4));
      }
      else {
        *(float *)(param_1 + 0x298) = (fVar1 + _DAT_005a4158) * *(float *)(param_1 + 0xd4);
      }
    }
  }
  return 1;
}

