// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00527baf | Name: FUN_00527baf


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __fastcall FUN_00527baf(int param_1)

{
  int iVar1;
  float10 fVar2;
  undefined4 local_18;
  undefined4 local_14;
  
  if (DAT_0067cf38 == 0) {
    fVar2 = (float10)_DAT_005a3534;
  }
  else if (*(int *)(param_1 + 0x24c) == 0x2d) {
    *(undefined4 *)(param_1 + 0x228) = 0;
    fVar2 = (float10)_DAT_005a3534;
  }
  else if ((*(int *)(param_1 + 0x24c) == 0x32) && (*(int *)(DAT_0067cf38 + 0x40) != 0)) {
    iVar1 = FUN_0049ad62(*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x2c));
    *(float *)(param_1 + 0x228) = (float)iVar1 * _DAT_005a4214;
    fVar2 = (float10)*(float *)(param_1 + 0x228);
  }
  else if (((*(int *)(param_1 + 0x5c) == 0x28) ||
           ((((*(int *)(param_1 + 0x5c) == 0x18 || (*(int *)(param_1 + 0x5c) == 0x14)) ||
             (*(int *)(param_1 + 0x5c) == 0x27)) || (*(int *)(param_1 + 0x5c) == 8)))) ||
          ((*(char *)(param_1 + 0x79c) != '\0' &&
           (((*(int *)(param_1 + 0x7a0) == 0x28 || (*(int *)(param_1 + 0x7a0) == 0x14)) ||
            (*(int *)(param_1 + 0x7a0) == 0x27)))))) {
    if (*(float *)(param_1 + 0x30) - *(float *)(param_1 + 0x228) <= _DAT_005a3534) {
      local_14 = -(*(float *)(param_1 + 0x30) - *(float *)(param_1 + 0x228));
    }
    else {
      local_14 = *(float *)(param_1 + 0x30) - *(float *)(param_1 + 0x228);
    }
    if (local_14 <= _DAT_005a4214) {
      fVar2 = (float10)*(float *)(param_1 + 0x228);
    }
    else {
      fVar2 = (float10)*(float *)(param_1 + 0x228) -
              ((float10)*(float *)(param_1 + 0x228) - (float10)*(float *)(param_1 + 0x30)) *
              (float10)_DAT_005a40f4;
    }
  }
  else if ((*(int *)(param_1 + 0x5c) == 5) && (*(short *)(param_1 + 0x1d2) != 1)) {
    if (*(float *)(param_1 + 0x30) - *(float *)(param_1 + 0x228) <= _DAT_005a3534) {
      local_18 = -(*(float *)(param_1 + 0x30) - *(float *)(param_1 + 0x228));
    }
    else {
      local_18 = *(float *)(param_1 + 0x30) - *(float *)(param_1 + 0x228);
    }
    if (local_18 <= _DAT_005a4214) {
      fVar2 = (float10)*(float *)(param_1 + 0x228);
    }
    else {
      fVar2 = (float10)*(float *)(param_1 + 0x228) -
              ((float10)*(float *)(param_1 + 0x228) - (float10)*(float *)(param_1 + 0x30)) *
              (float10)_DAT_005a40f4;
    }
  }
  else {
    fVar2 = (float10)*(float *)(param_1 + 0x228);
  }
  return fVar2;
}

