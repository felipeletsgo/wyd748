// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054b02f | Name: FUN_0054b02f


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_0054b02f(int param_1,int param_2)

{
  float fVar1;
  float10 fVar2;
  
  if (*(short *)(param_1 + 0x160) == 0) {
    fVar1 = ((float)((uint)(param_2 + param_1 * 100) % 1000) / _DAT_005a43c0) * _DAT_005a4148;
    fVar1 = fVar1 + fVar1;
    *(float *)(param_1 + 0x168) = *(float *)(param_1 + 0x174) - _DAT_005a3660;
    fVar2 = (float10)FUN_00493bf0(fVar1);
    *(float *)(param_1 + 0x164) =
         (float)(fVar2 * (float10)0.5 + (float10)*(float *)(param_1 + 0x170));
    fVar2 = (float10)FUN_00423b00(fVar1);
    *(float *)(param_1 + 0x16c) =
         (float)(fVar2 * (float10)0.5 + (float10)*(float *)(param_1 + 0x178));
    FUN_004310a0(param_1 + 0x164);
  }
  return 1;
}

