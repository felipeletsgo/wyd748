// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004af91b | Name: FUN_004af91b


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004af91b(int param_1)

{
  undefined4 uVar1;
  
  if ((((DAT_005ccec0 != 0) && (_DAT_005a3534 < *(float *)(DAT_005ccec0 + 0x4c))) &&
      (*(float *)(DAT_005ccec0 + 0x4c) <
       (float)(*(int *)(DAT_005ccf9c + 0x2a504) - *(int *)(DAT_005ccf9c + 0x2a5e4)))) &&
     ((_DAT_005a3534 < *(float *)(DAT_005ccec0 + 0x50) &&
      (*(float *)(DAT_005ccec0 + 0x50) <
       (float)(*(int *)(DAT_005ccf9c + 0x2a508) - *(int *)(DAT_005ccf9c + 0x2a5e8)))))) {
    uVar1 = __ftol();
    uVar1 = __ftol(uVar1);
    (**(code **)(*DAT_0067cf38 + 8))(0x204,*(undefined4 *)(param_1 + 0x44c),uVar1);
  }
  return;
}

