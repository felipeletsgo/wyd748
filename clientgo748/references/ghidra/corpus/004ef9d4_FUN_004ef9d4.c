// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ef9d4 | Name: FUN_004ef9d4


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004ef9d4(int param_1,int param_2)

{
  if (((*(int *)(param_1 + 0x24) != 1) || (param_2 != 1)) &&
     ((*(int *)(param_1 + 0x24) != 0 || (param_2 != 0)))) {
    *(int *)(param_1 + 0x24) = param_2;
    DAT_005ccf9c[0xac0e] = 1;
    if (*(int *)(param_1 + 0x24) == 1) {
      *(undefined4 *)(param_1 + 0x2c) = 0xbf451e9b;
      *(float *)(param_1 + 0x34) = *(float *)(param_1 + 0xc0) - 0.0;
      *(float *)(param_1 + 0x38) = *(float *)(param_1 + 0xc0) - 0.0;
      *(undefined4 *)(param_1 + 0x3c) = 0x3ea8f5c3;
      *(undefined4 *)(param_1 + 0x28) = 0x3f490fdb;
      DAT_005ccf9c[0xac0e] = 1;
    }
    else if (*(int *)(param_1 + 0x24) == 0) {
      *(undefined4 *)(param_1 + 0x2c) = 0xbea0d97c;
      *(undefined4 *)(param_1 + 0x34) = 0x40e00000;
      *(undefined4 *)(param_1 + 0x38) = 0x40e00000;
      *(undefined4 *)(param_1 + 0x3c) = 0;
      *(undefined4 *)(param_1 + 0x28) = 0x3f490fdb;
    }
    else if (*(int *)(param_1 + 0x24) == 2) {
      *(undefined4 *)(param_1 + 0x2c) = 0xbf860a92;
      *(float *)(param_1 + 0x34) = *(float *)(param_1 + 0xc0) - _DAT_005a3660;
      *(float *)(param_1 + 0x38) = *(float *)(param_1 + 0xc0) - _DAT_005a3660;
      *(undefined4 *)(param_1 + 0x3c) = 0x3f000000;
      *(undefined4 *)(param_1 + 0x28) = 0x3f490fdb;
      DAT_005ccf9c[0xac0e] = 1;
    }
    else if (*(int *)(param_1 + 0x24) == 4) {
      *(undefined4 *)(param_1 + 0x2c) = 0xbf490fdb;
      *(undefined4 *)(param_1 + 0x28) = 0x3f490fdb;
      *(undefined4 *)(param_1 + 0x34) = 0x41780000;
      *(undefined4 *)(param_1 + 0x38) = 0x41780000;
      *(undefined4 *)(param_1 + 0x3c) = 0;
    }
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,DAT_005ccf9c[0xac0e]);
  }
  return;
}

