// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053ac91 | Name: FUN_0053ac91


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0053ac91(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  float10 fVar1;
  
  FUN_0053e98a(param_2,param_3,param_4);
  if (*(char *)(param_1 + 0x161) == '\x05') {
    fVar1 = (float10)FUN_00493bf0(-*(float *)(param_1 + 0x34) + _DAT_005a5934);
    *(float *)(param_1 + 0x170) = (float)(fVar1 * (float10)_DAT_005a4418);
    fVar1 = (float10)FUN_00423b00(-*(float *)(param_1 + 0x34) + _DAT_005a5934);
    *(float *)(param_1 + 0x174) = (float)(fVar1 * (float10)_DAT_005a4418);
    fVar1 = (float10)FUN_00493bf0(-*(float *)(param_1 + 0x34) - _DAT_005a5934);
    *(float *)(param_1 + 0x178) = (float)(-fVar1 * (float10)_DAT_005a4418);
    fVar1 = (float10)FUN_00423b00(-*(float *)(param_1 + 0x34) - _DAT_005a5934);
    *(float *)(param_1 + 0x17c) = (float)(-fVar1 * (float10)_DAT_005a4418);
  }
  return;
}

