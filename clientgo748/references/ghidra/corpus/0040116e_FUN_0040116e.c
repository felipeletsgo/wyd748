// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040116e | Name: FUN_0040116e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
FUN_0040116e(int param_1,int param_2,undefined4 param_3,undefined4 param_4,float param_5)

{
  int local_8;
  
  if ((DAT_005b892c == 2) && (DAT_005ccf9c != 0)) {
    for (local_8 = 0; local_8 < 5; local_8 = local_8 + 1) {
      if (param_2 == *(int *)(&DAT_005b1080 + local_8 * 4)) {
        *(float *)(param_1 + 0x4c) =
             (float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 - param_5 * _DAT_005a34a0;
      }
    }
  }
  return;
}

