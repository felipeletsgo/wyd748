// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054746a | Name: FUN_0054746a


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0054746a(int param_1,undefined4 param_2,float param_3,undefined4 param_4)

{
  *(float *)(param_1 + 0x34) = param_3;
  if ((*(int *)(param_1 + 0x60) != 0) && (*(int *)(param_1 + 0x60) != 0)) {
    FUN_004be1a3(param_2,param_3 + _DAT_005a4380,param_4);
  }
  *(undefined4 *)(param_1 + 0x198) = *(undefined4 *)(param_1 + 0x34);
  return;
}

