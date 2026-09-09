// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f07d2 | Name: FUN_004f07d2


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall FUN_004f07d2(int param_1,undefined4 *param_2)

{
  float fVar1;
  float10 fVar2;
  float10 fVar3;
  
  fVar1 = _DAT_005a3534 + *(float *)(param_1 + 0x28);
  fVar2 = (float10)FUN_00493bf0(fVar1);
  fVar3 = (float10)FUN_00493bf0(*(undefined4 *)(param_1 + 0x2c));
  *(float *)(param_1 + 0x58) = (float)(fVar3 * (float10)(float)fVar2);
  fVar2 = (float10)FUN_00423b00(fVar1);
  fVar3 = (float10)FUN_00493bf0(*(undefined4 *)(param_1 + 0x2c));
  *(float *)(param_1 + 0x60) = (float)(fVar3 * (float10)(float)fVar2);
  fVar2 = (float10)FUN_00423b00(*(undefined4 *)(param_1 + 0x2c));
  *(float *)(param_1 + 0x5c) = (float)fVar2;
  *param_2 = *(undefined4 *)(param_1 + 0x58);
  param_2[1] = *(undefined4 *)(param_1 + 0x5c);
  param_2[2] = *(undefined4 *)(param_1 + 0x60);
  return param_2;
}

