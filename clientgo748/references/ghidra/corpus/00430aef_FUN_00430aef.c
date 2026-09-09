// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00430aef | Name: FUN_00430aef


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00430aef(int param_1,undefined4 *param_2,float *param_3)

{
  int iVar1;
  int iVar2;
  float local_50;
  float local_4c;
  float local_48;
  float local_40;
  float local_3c;
  float local_38;
  float local_30;
  float local_2c;
  float local_28;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  float local_10;
  float local_c;
  float local_8;
  
  FUN_00430f10();
  if (DAT_005ccec0 != 0) {
    iVar1 = *(int *)(param_1 + 0x2a504) - *(int *)(param_1 + 0x2a5e4);
    local_10 = ((_DAT_005a365c * *(float *)(DAT_005ccec0 + 0x4c)) / (float)iVar1 - _DAT_005a3660) /
               *(float *)(param_1 + 0x2a5ec);
    iVar2 = *(int *)(param_1 + 0x2a508) - *(int *)(param_1 + 0x2a5e8);
    local_c = -((_DAT_005a365c * *(float *)(DAT_005ccec0 + 0x50)) / (float)iVar2 - _DAT_005a3660) /
              *(float *)(param_1 + 0x2a600);
    local_8 = 1.0;
    FUN_00430f10(iVar2,0,iVar1,0);
    thunk_FUN_00561f6d(&local_50,0,param_1 + 0x2a62c);
    *param_3 = local_8 * local_30 + local_c * local_40 + local_10 * local_50;
    param_3[1] = local_8 * local_2c + local_c * local_3c + local_10 * local_4c;
    param_3[2] = local_8 * local_28 + local_c * local_38 + local_10 * local_48;
    *param_2 = local_20;
    param_2[1] = local_1c;
    param_2[2] = local_18;
  }
  return;
}

