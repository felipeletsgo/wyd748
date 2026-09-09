// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d53b2 | Name: FUN_004d53b2


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004d53b2(int *param_1)

{
  int iVar1;
  
  iVar1 = (**(code **)(*DAT_0092e654 + 8))();
  (**(code **)(*param_1 + 0x38))();
  if ((param_1[0x21] != 0) &&
     (_DAT_005a3660 < (float)(uint)(iVar1 - param_1[0x20]) / (float)param_1[0x21])) {
    (**(code **)(*DAT_013b71e8 + 100))(param_1,param_1[0x21],0);
  }
  return 1;
}

