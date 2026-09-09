// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005477aa | Name: FUN_005477aa


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_005477aa(int param_1)

{
  float fVar1;
  float10 fVar2;
  undefined4 local_c;
  undefined4 local_8;
  
  FUN_0040bd30(&stack0x00000004);
  FUN_00493c60(&local_c,param_1 + 0x28);
  fVar2 = (float10)FUN_00493c30(local_c,local_8);
  *(float *)(param_1 + 0x198) = (float)fVar2;
  fVar1 = *(float *)(param_1 + 0x198) - *(float *)(param_1 + 0x34);
  if (fVar1 <= _DAT_005a5c00) {
    if (fVar1 < _DAT_005a5bfc) {
      *(float *)(param_1 + 0x198) = *(float *)(param_1 + 0x198) + _DAT_005a35c4;
    }
  }
  else {
    *(float *)(param_1 + 0x34) = *(float *)(param_1 + 0x34) + _DAT_005a35c4;
  }
  return;
}

