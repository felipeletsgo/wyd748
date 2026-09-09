// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f710b | Name: FUN_004f710b


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004f710b(int *param_1)

{
  int iVar1;
  float local_18;
  float local_14;
  float local_10;
  int local_c;
  int local_8;
  
  if ((short)param_1[0x58] != 0) {
    (**(code **)(*(int *)param_1[0x8f] + 0x60))(0);
    (**(code **)(*(int *)param_1[0x6b] + 0x60))(0);
    local_8 = (**(code **)(*(int *)DAT_013b71e8[0x6c21] + 0x4c))();
    if ((local_8 != 0) &&
       (((((float)param_1[10] < (float)(*(int *)(local_8 + 0x47c) + -0x12) ||
          ((float)(*(int *)(local_8 + 0x47c) + 0x12) < (float)param_1[10])) ||
         ((float)param_1[0xb] < (float)(*(int *)(local_8 + 0x480) + -0x12))) ||
        ((float)(*(int *)(local_8 + 0x480) + 0x12) < (float)param_1[0xb])))) {
      if (*(int **)(DAT_0067cf38 + 0x80) == param_1) {
        *(undefined4 *)(DAT_0067cf38 + 0x80) = 0;
      }
      if (((param_1[9] != 0x195) && (param_1[9] != 0x647)) &&
         (((uint)param_1[9] < 0xaa3 || (0xaa7 < (uint)param_1[9])))) {
        (**(code **)(*DAT_013b71e8 + 100))(param_1);
      }
    }
    if (param_1[0x12] == 1) {
      FUN_004f7482();
      FUN_0049cb58(&local_18,param_1[10],param_1[0xb]);
      _memset(param_1 + 0x5a,0,0x44);
      iVar1 = (**(code **)(*param_1 + 0x6c))();
      if ((iVar1 == 1) && (((short)param_1[0x58] < 0xc49 || (0xc4d < (short)param_1[0x58])))) {
        param_1[0x66] = 0x3f800000;
        param_1[0x67] = 0x3f800000;
        param_1[0x68] = 0x3f800000;
      }
      else {
        if ((param_1[0x6e] == 3) || ((param_1[0x6e] == 2 || (param_1[0x6e] == 1)))) {
          param_1[0x66] = (int)(local_18 * _DAT_005a4158 + _DAT_005a4160);
          param_1[0x67] = (int)(local_14 * _DAT_005a4158 + _DAT_005a4160);
          param_1[0x68] = (int)(local_10 * _DAT_005a4158 + _DAT_005a4160);
        }
        else {
          param_1[0x66] = (int)(local_18 * _DAT_005a40ec + _DAT_005a40ec);
          param_1[0x67] = (int)(local_14 * _DAT_005a40ec + _DAT_005a40ec);
          param_1[0x68] = (int)(local_10 * _DAT_005a40ec + _DAT_005a40ec);
        }
        if ((short)param_1[0x58] == 0x6bf) {
          param_1[0x66] = (int)(local_18 * _DAT_005a4154 + _DAT_005a414c);
          param_1[0x67] = (int)(local_14 * _DAT_005a4154 + _DAT_005a414c);
          param_1[0x68] = (int)(local_10 * _DAT_005a4154 + _DAT_005a414c);
        }
      }
      param_1[0x5a] = (int)local_18;
      param_1[0x5b] = (int)local_14;
      param_1[0x5c] = (int)local_10;
      param_1[0x5d] = local_c;
      param_1[0x62] = param_1[0x5a];
      param_1[99] = param_1[0x5b];
      param_1[100] = param_1[0x5c];
      param_1[0x65] = param_1[0x5d];
      param_1[0x6a] = 0;
    }
  }
  return 1;
}

