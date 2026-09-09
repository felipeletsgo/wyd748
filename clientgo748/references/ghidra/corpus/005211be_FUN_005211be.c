// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005211be | Name: FUN_005211be


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
FUN_005211be(int param_1,int param_2,int param_3,char *param_4,int param_5,int *param_6)

{
  size_t sVar1;
  int local_10;
  float local_c;
  float local_8;
  
  if (*(int *)(param_1 + 0x214) == 0) {
    FUN_00430f10();
    local_c = (float)param_2 + _DAT_005a34a0;
    local_8 = (float)param_3 + _DAT_005a34a0;
    FUN_0040bd30(&local_c);
    if (param_6 != (int *)0x0) {
      sVar1 = _strlen(param_4);
      *param_6 = sVar1 + 2;
    }
    for (local_10 = 1; local_10 < 0x30; local_10 = local_10 + 1) {
      FUN_0040bd30(&local_c);
      if (param_4[local_10 + -1] == '6') {
        *(float *)(param_5 + local_10 * 8) = *(float *)(param_5 + local_10 * 8) + _DAT_005a3660;
      }
      else if (param_4[local_10 + -1] == '4') {
        *(float *)(param_5 + local_10 * 8) = *(float *)(param_5 + local_10 * 8) - _DAT_005a3660;
      }
      else if (param_4[local_10 + -1] == '8') {
        *(float *)(param_5 + 4 + local_10 * 8) =
             *(float *)(param_5 + 4 + local_10 * 8) + _DAT_005a3660;
      }
      else if (param_4[local_10 + -1] == '2') {
        *(float *)(param_5 + 4 + local_10 * 8) =
             *(float *)(param_5 + 4 + local_10 * 8) - _DAT_005a3660;
      }
      else if (param_4[local_10 + -1] == '3') {
        *(float *)(param_5 + local_10 * 8) = *(float *)(param_5 + local_10 * 8) + _DAT_005a3660;
        *(float *)(param_5 + 4 + local_10 * 8) =
             *(float *)(param_5 + 4 + local_10 * 8) - _DAT_005a3660;
      }
      else if (param_4[local_10 + -1] == '1') {
        *(float *)(param_5 + local_10 * 8) = *(float *)(param_5 + local_10 * 8) - _DAT_005a3660;
        *(float *)(param_5 + 4 + local_10 * 8) =
             *(float *)(param_5 + 4 + local_10 * 8) - _DAT_005a3660;
      }
      else if (param_4[local_10 + -1] == '9') {
        *(float *)(param_5 + local_10 * 8) = *(float *)(param_5 + local_10 * 8) + _DAT_005a3660;
        *(float *)(param_5 + 4 + local_10 * 8) =
             *(float *)(param_5 + 4 + local_10 * 8) + _DAT_005a3660;
      }
      else if (param_4[local_10 + -1] == '7') {
        *(float *)(param_5 + local_10 * 8) = *(float *)(param_5 + local_10 * 8) - _DAT_005a3660;
        *(float *)(param_5 + 4 + local_10 * 8) =
             *(float *)(param_5 + 4 + local_10 * 8) + _DAT_005a3660;
      }
      FUN_0040bd30(param_5 + local_10 * 8);
    }
  }
  return;
}

