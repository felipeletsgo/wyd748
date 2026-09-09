// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004091ab | Name: FUN_004091ab


void __fastcall FUN_004091ab(int param_1)

{
  undefined4 *puVar1;
  int local_8;
  
  for (local_8 = 0; local_8 < *(int *)(param_1 + 0x210); local_8 = local_8 + 1) {
    if (*(int *)(param_1 + 0x214 + local_8 * 4) != 0) {
      puVar1 = *(undefined4 **)(param_1 + 0x214 + local_8 * 4);
      if (puVar1 != (undefined4 *)0x0) {
        (**(code **)*puVar1)(1);
      }
      *(undefined4 *)(param_1 + 0x214 + local_8 * 4) = 0;
    }
  }
  *(undefined4 *)(param_1 + 0x210) = 0;
  if ((*(char *)(param_1 + 0x1e8) != '\0') && (*(int *)(param_1 + 0x20c) != 0)) {
    (**(code **)(**(int **)(param_1 + 0x20c) + 0x88))(*(undefined4 *)(param_1 + 0x210));
  }
  return;
}

