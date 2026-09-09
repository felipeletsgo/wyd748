// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e0924 | Name: FUN_004e0924


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004e0924(int param_1)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x24) == 0) {
    uVar1 = 0;
  }
  else {
    *(float *)(param_1 + 0x70) = (float)*(int *)(param_1 + 0x94) / _DAT_005a365c;
    uVar1 = FUN_004c8daa();
  }
  return uVar1;
}

