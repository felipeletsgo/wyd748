// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004af854 | Name: FUN_004af854


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004af854(int param_1)

{
  undefined4 uVar1;
  
  if ((((DAT_005ccec0 != 0) && (_DAT_005a3534 < *(float *)(DAT_005ccec0 + 0x4c))) &&
      (*(float *)(DAT_005ccec0 + 0x4c) < (float)*(int *)(DAT_005ccf9c + 0x2a504))) &&
     ((_DAT_005a3534 < *(float *)(DAT_005ccec0 + 0x50) &&
      (*(float *)(DAT_005ccec0 + 0x50) < (float)*(int *)(DAT_005ccf9c + 0x2a508))))) {
    uVar1 = __ftol();
    uVar1 = __ftol(uVar1);
    (**(code **)(*DAT_0067cf38 + 8))(0x201,*(undefined4 *)(param_1 + 0x44c),uVar1);
  }
  return;
}

