// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00409078 | Name: FUN_00409078


undefined4 __thiscall FUN_00409078(int param_1,int param_2)

{
  undefined4 *puVar1;
  int local_c;
  int local_8;
  
  local_8 = 0;
  while( true ) {
    if (*(int *)(param_1 + 0x210) <= local_8) {
      return 0;
    }
    if (*(int *)(param_1 + 0x214 + local_8 * 4) == param_2) break;
    local_8 = local_8 + 1;
  }
  local_c = local_8;
  if (*(int *)(param_1 + 0x214 + local_8 * 4) != 0) {
    puVar1 = *(undefined4 **)(param_1 + 0x214 + local_8 * 4);
    if (puVar1 != (undefined4 *)0x0) {
      (**(code **)*puVar1)(1);
    }
    *(undefined4 *)(param_1 + 0x214 + local_8 * 4) = 0;
  }
  while (local_c = local_c + 1, local_c < *(int *)(param_1 + 0x210)) {
    *(undefined4 *)(param_1 + 0x210 + local_c * 4) = *(undefined4 *)(param_1 + 0x214 + local_c * 4);
  }
  *(int *)(param_1 + 0x210) = *(int *)(param_1 + 0x210) + -1;
  if (*(char *)(param_1 + 0x1e8) != '\0') {
    (**(code **)(**(int **)(param_1 + 0x20c) + 0x88))(*(undefined4 *)(param_1 + 0x210));
  }
  return 1;
}

