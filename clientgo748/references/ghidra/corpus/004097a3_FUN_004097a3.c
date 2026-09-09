// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004097a3 | Name: FUN_004097a3


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004097a3(int param_1,undefined4 param_2,float param_3)

{
  FUN_00401275(param_2,param_3);
  *(undefined4 *)(param_1 + 0x200) = *(undefined4 *)(param_1 + 0x54);
  *(undefined4 *)(param_1 + 0x204) = *(undefined4 *)(param_1 + 0x58);
  if (*(int *)(param_1 + 0x20c) != 0) {
    (**(code **)(**(int **)(param_1 + 0x20c) + 0x74))
              (*(undefined4 *)(*(int *)(param_1 + 0x20c) + 0x54),param_3 - _DAT_005a3660);
  }
  return;
}

