// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040827e | Name: FUN_0040827e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0040827e(int param_1,float param_2,undefined4 param_3)

{
  FUN_00401224(param_2,param_3);
  (**(code **)(**(int **)(param_1 + 0xe50) + 0x70))(param_2 + _DAT_005a3c64,param_3);
  (**(code **)(**(int **)(param_1 + 0xe54) + 0x70))(param_2 + _DAT_005a3c60,param_3);
  (**(code **)(**(int **)(param_1 + 0xe58) + 0x70))(param_2 + _DAT_005a3c5c,param_3);
  (**(code **)(**(int **)(param_1 + 0xe5c) + 0x70))(param_2 + _DAT_005a3c58,param_3);
  return;
}

