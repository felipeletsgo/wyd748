// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040268c | Name: FUN_0040268c


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
FUN_0040268c(int *param_1,undefined4 param_2,float param_3,float param_4,int param_5)

{
  size_t sVar1;
  float local_c;
  float local_8;
  
  (**(code **)(*param_1 + 0x8c))(param_1[0x336]);
  if ((char)param_1[0x337] == '\x01') {
    param_1[0x339] = (int)(param_3 + (float)param_1[0x13]);
    param_1[0x33a] = (int)(param_4 + (float)param_1[0x14]);
    param_1[0x33b] = param_1[0x15];
    param_1[0x33c] = param_1[0x16];
    param_1[0x38a] = param_5;
    if (_DAT_005a34a8 * DAT_005b490c < (float)param_1[0x339]) {
      return;
    }
    if (_DAT_005a34a4 * DAT_005b4910 < (float)param_1[0x33a]) {
      return;
    }
    if ((float)param_1[0x339] + (float)param_1[0x33b] < _DAT_005a3534) {
      return;
    }
    if ((float)param_1[0x33a] + (float)param_1[0x33c] < _DAT_005a3534) {
      return;
    }
    FUN_0040c26d(param_2,param_1 + 0x338,param_5);
  }
  if (DAT_005b892c == 2) {
    local_8 = 1.0;
    local_c = 1.0;
  }
  else {
    local_c = DAT_005b490c;
    local_8 = DAT_005b4910;
  }
  if (param_1[0x335] == 0) {
    param_1[0x1ca] = (int)(_DAT_005a366c * local_c + param_3 + (float)param_1[0x13]);
    if (*(char *)((int)param_1 + 0xcdd) == '\x01') {
      param_1[0x1ca] =
           (int)(_DAT_005a366c * local_c + param_3 + (float)param_1[0x13] + _DAT_005a3668);
      param_1[0x225] =
           (int)(_DAT_005a366c * local_c + param_3 + (float)param_1[0x13] + _DAT_005a3664);
      param_1[0x280] =
           (int)(_DAT_005a366c * local_c + param_3 + (float)param_1[0x13] + _DAT_005a3660);
      param_1[0x2db] = (int)(_DAT_005a366c * local_c + param_3 + (float)param_1[0x13]);
    }
  }
  else if (param_1[0x335] == 3) {
    param_1[0x1ca] = (int)(_DAT_005a365c * local_c + param_3 + (float)param_1[0x13]);
    if (*(char *)((int)param_1 + 0xcdd) == '\x01') {
      param_1[0x1ca] =
           (int)(_DAT_005a365c * local_c + param_3 + (float)param_1[0x13] + _DAT_005a3658);
      param_1[0x225] =
           (int)(_DAT_005a365c * local_c + param_3 + (float)param_1[0x13] + _DAT_005a366c);
      param_1[0x280] =
           (int)(_DAT_005a365c * local_c + param_3 + (float)param_1[0x13] + _DAT_005a365c);
      param_1[0x2db] = (int)(_DAT_005a365c * local_c + param_3 + (float)param_1[0x13]);
    }
  }
  else if (param_1[0x335] == 2) {
    sVar1 = _strlen((char *)((int)param_1 + 0x76a));
    param_1[0x1ca] =
         (int)((param_3 + (float)param_1[0x13] + (float)param_1[0x15]) -
              (float)(int)(sVar1 * 8 + 8) * local_c);
    if (*(char *)((int)param_1 + 0xcdd) == '\x01') {
      param_1[0x1ca] =
           (int)(((param_3 + (float)param_1[0x13] + (float)param_1[0x15]) -
                 (float)(int)(sVar1 * 8 + 8) * local_c) + _DAT_005a3658);
      param_1[0x225] =
           (int)(((param_3 + (float)param_1[0x13] + (float)param_1[0x15]) -
                 (float)(int)(sVar1 * 8 + 8) * local_c) + _DAT_005a366c);
      param_1[0x280] =
           (int)(((param_3 + (float)param_1[0x13] + (float)param_1[0x15]) -
                 (float)(int)(sVar1 * 8 + 8) * local_c) + _DAT_005a365c);
      param_1[0x2db] =
           (int)((param_3 + (float)param_1[0x13] + (float)param_1[0x15]) -
                (float)(int)(sVar1 * 8 + 8) * local_c);
    }
  }
  else if (param_1[0x335] == 1) {
    sVar1 = _strlen((char *)((int)param_1 + 0x76a));
    param_1[0x1ca] =
         (int)(((float)param_1[0x15] - (float)(int)(sVar1 * 6) * local_8) / _DAT_005a365c +
              param_3 + (float)param_1[0x13]);
    if (*(char *)((int)param_1 + 0xcdd) == '\x01') {
      param_1[0x1ca] =
           (int)(((float)param_1[0x15] - (float)(int)(sVar1 * 6) * local_8) / _DAT_005a365c +
                 param_3 + (float)param_1[0x13] + _DAT_005a3658);
      param_1[0x225] =
           (int)(((param_3 + (float)param_1[0x13] + (float)param_1[0x15]) -
                 (float)(int)(sVar1 * 8 + 8) * local_c) + _DAT_005a366c);
      param_1[0x280] =
           (int)(((param_3 + (float)param_1[0x13] + (float)param_1[0x15]) -
                 (float)(int)(sVar1 * 8 + 8) * local_c) + _DAT_005a365c);
      param_1[0x2db] =
           (int)((param_3 + (float)param_1[0x13] + (float)param_1[0x15]) -
                (float)(int)(sVar1 * 8 + 8) * local_c);
    }
  }
  else if ((param_1[0x335] == 4) &&
          (param_1[0x1ca] = (int)(_DAT_005a366c * local_c + param_3 + (float)param_1[0x13] + 104.0),
          *(char *)((int)param_1 + 0xcdd) == '\x01')) {
    param_1[0x1ca] =
         (int)(_DAT_005a366c * local_c + param_3 + (float)param_1[0x13] + _DAT_005a3658 + 104.0);
    param_1[0x225] =
         (int)(_DAT_005a366c * local_c + param_3 + (float)param_1[0x13] + _DAT_005a366c + 104.0);
    param_1[0x280] =
         (int)(_DAT_005a366c * local_c + param_3 + (float)param_1[0x13] + _DAT_005a365c + 104.0);
    param_1[0x2db] = (int)(_DAT_005a366c * local_c + param_3 + (float)param_1[0x13] + 104.0);
  }
  param_1[0x1cb] =
       (int)(((float)param_1[0x16] - _DAT_005a3668 * local_8) / _DAT_005a365c + _DAT_005a365c +
            param_4 + (float)param_1[0x14]);
  param_1[0x1cc] = param_1[0x15];
  param_1[0x1cd] = param_1[0x16];
  param_1[0x21b] = param_5;
  if (((((float)param_1[0x1ca] <= _DAT_005a34a8 * DAT_005b490c) &&
       ((float)param_1[0x1cb] <= _DAT_005a34a4 * DAT_005b4910)) &&
      (_DAT_005a3534 <= (float)param_1[0x1ca] + (float)param_1[0x1cc])) &&
     ((_DAT_005a3534 <= (float)param_1[0x1cb] + (float)param_1[0x1cd] &&
      (FUN_0040c26d(param_2,param_1 + 0x1c9,param_5), *(char *)((int)param_1 + 0xcdd) == '\x01'))))
  {
    param_1[0x226] =
         (int)(((float)param_1[0x16] - _DAT_005a3668 * local_8) / _DAT_005a365c + _DAT_005a365c +
              param_4 + (float)param_1[0x14]);
    param_1[0x227] = param_1[0x15];
    param_1[0x228] = param_1[0x16];
    param_1[0x276] = param_5;
    FUN_0040c26d(param_2,param_1 + 0x224,param_5);
    param_1[0x281] =
         (int)(((float)param_1[0x16] - _DAT_005a3668 * local_8) / _DAT_005a365c + _DAT_005a365c +
              param_4 + (float)param_1[0x14]);
    param_1[0x282] = param_1[0x15];
    param_1[0x283] = param_1[0x16];
    param_1[0x2d1] = param_5;
    FUN_0040c26d(param_2,param_1 + 0x27f,param_5);
    param_1[0x2dc] =
         (int)(((float)param_1[0x16] - _DAT_005a3668 * local_8) / _DAT_005a365c + _DAT_005a365c +
              param_4 + (float)param_1[0x14]);
    param_1[0x2dd] = param_1[0x15];
    param_1[0x2de] = param_1[0x16];
    param_1[0x32c] = param_5;
    FUN_0040c26d(param_2,param_1 + 0x2da,param_5);
  }
  return;
}

