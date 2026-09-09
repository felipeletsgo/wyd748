// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f0d86 | Name: FUN_004f0d86


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004f0d86(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  float10 fVar1;
  
  FUN_0053e94b(param_2,param_3,param_4);
  fVar1 = (float10)FUN_00423b00(*(float *)(param_1 + 0x34) - _DAT_005a4380);
  *(float *)(param_1 + 0x2ec) = (float)fVar1;
  fVar1 = (float10)FUN_00493bf0(*(float *)(param_1 + 0x34) - _DAT_005a4380);
  *(float *)(param_1 + 0x2f0) = (float)fVar1;
  return;
}

