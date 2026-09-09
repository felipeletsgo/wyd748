// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00401637 | Name: FUN_00401637


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
FUN_00401637(int param_1,undefined4 param_2,float param_3,float param_4,undefined4 param_5)

{
  if ((-1 < *(int *)(param_1 + 0xa0)) || ((*(uint *)(param_1 + 0x94) & 0xff000000) != 0)) {
    *(float *)(param_1 + 0x68) = param_3 + *(float *)(param_1 + 0x4c);
    *(float *)(param_1 + 0x6c) = param_4 + *(float *)(param_1 + 0x50);
    *(undefined4 *)(param_1 + 0x70) = *(undefined4 *)(param_1 + 0x54);
    *(undefined4 *)(param_1 + 0x74) = *(undefined4 *)(param_1 + 0x58);
    *(undefined4 *)(param_1 + 0x1ac) = param_5;
    if ((*(float *)(param_1 + 0x68) <= _DAT_005a34a8 * DAT_005b490c) &&
       (((*(float *)(param_1 + 0x6c) <= _DAT_005a34a4 * DAT_005b4910 &&
         (_DAT_005a3534 <= *(float *)(param_1 + 0x68) + *(float *)(param_1 + 0x70))) &&
        (_DAT_005a3534 <= *(float *)(param_1 + 0x6c) + *(float *)(param_1 + 0x74))))) {
      FUN_0040c26d(param_2,param_1 + 100,param_5);
    }
  }
  return;
}

