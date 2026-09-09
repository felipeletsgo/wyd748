// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f2b52 | Name: FUN_004f2b52


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_004f2b52(int *param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  undefined4 local_c;
  int local_8;
  
  if (((short)param_1[0x58] < 0xc49) || (0xc4d < (short)param_1[0x58])) {
    if ((param_1[8] == 0x2746) && ((float)param_1[0xc] < _DAT_005a4398)) {
      param_1[0xc] = 0;
      *(undefined2 *)(param_1 + 0xc0) = 0;
    }
    local_8 = *(int *)(DAT_0067cf38 + 0x4c);
    if ((local_8 != 0) &&
       (((((float)param_1[10] < (float)(*(int *)(local_8 + 0x47c) + -0x21) ||
          ((float)(*(int *)(local_8 + 0x47c) + 0x21) < (float)param_1[10])) ||
         ((float)param_1[0xb] < (float)(*(int *)(local_8 + 0x480) + -0x21))) ||
        ((float)(*(int *)(local_8 + 0x480) + 0x21) < (float)param_1[0xb])))) {
      if (*(int **)(DAT_0067cf38 + 0x80) == param_1) {
        *(undefined4 *)(DAT_0067cf38 + 0x80) = 0;
      }
      param_1[0xba] = 2;
      *(undefined2 *)(param_1 + 0xc0) = 0;
      if ((short)param_1[0x58] == 0x167) {
        (**(code **)(*DAT_013b71e8 + 100))(param_1);
      }
    }
    if (((short)param_1[0xc0] == 0) && (0 < (short)param_1[0x58])) {
      local_c = FUN_0054cd07(param_1 + 0x58,0x22);
      uVar1 = __ftol(0x7e);
      uVar1 = __ftol(DAT_0067cf38 + 0x94,uVar1);
      uVar1 = __ftol(uVar1);
      FUN_005554cc(local_c,2,2,uVar1);
    }
    if ((param_1[0xba] == 2) || (param_1[0xba] == 3)) {
      param_1[0xb9] = 0;
    }
    else if ((param_1[0xba] == 5) || (param_1[0xba] == 6)) {
      param_1[0xb9] =
           (int)(_DAT_005a4380 -
                ((float)(uint)(param_2 - param_1[0xbb]) * _DAT_005a41ec * _DAT_005a4148) /
                _DAT_005a365c);
      if (1000 < (uint)(param_2 - param_1[0xbb])) {
        if (param_1[0xba] == 5) {
          param_1[0xba] = 2;
        }
        else if (param_1[0xba] == 6) {
          param_1[0xba] = 3;
        }
      }
    }
    else if (param_1[0xba] == 1) {
      param_1[0xb9] = 0x3fc90fdb;
    }
    else if ((param_1[0xba] == 4) &&
            (param_1[0xb9] =
                  (int)(((float)(uint)(param_2 - param_1[0xbb]) * _DAT_005a41ec * _DAT_005a4148) /
                       _DAT_005a365c), 1000 < (uint)(param_2 - param_1[0xbb]))) {
      param_1[0xba] = 1;
    }
    if (param_1[0x12] == 1) {
      FUN_004f7482();
      FUN_0049cb58(&local_1c,param_1[10],param_1[0xb]);
      _memset(param_1 + 0x5a,0,0x44);
      iVar2 = (**(code **)(*param_1 + 0x6c))();
      if (iVar2 == 1) {
        param_1[0x66] = 0x3f800000;
        param_1[0x67] = 0x3f800000;
        param_1[0x68] = 0x3f800000;
      }
      else {
        local_1c = 0x3f333333;
        local_18 = 0x3f333333;
        local_14 = 0x3f333333;
        param_1[0x66] = 0x3e99999a;
        param_1[0x67] = 0x3e99999a;
        param_1[0x68] = 0x3e99999a;
      }
      param_1[0x5a] = local_1c;
      param_1[0x5b] = local_18;
      param_1[0x5c] = local_14;
      param_1[0x5d] = local_10;
      param_1[0x62] = param_1[0x5a];
      param_1[99] = param_1[0x5b];
      param_1[100] = param_1[0x5c];
      param_1[0x65] = param_1[0x5d];
      param_1[0x6a] = 0;
    }
    uVar1 = 1;
  }
  else {
    uVar1 = FUN_004f710b(param_2);
  }
  return uVar1;
}

