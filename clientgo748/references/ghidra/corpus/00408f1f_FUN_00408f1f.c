// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00408f1f | Name: FUN_00408f1f


undefined4 __thiscall FUN_00408f1f(int param_1,int param_2)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  int local_8;
  
  if ((param_2 < *(int *)(param_1 + 0x210)) && (-1 < param_2)) {
    local_8 = param_2;
    if (*(int *)(param_1 + 0x214 + param_2 * 4) != 0) {
      puVar1 = *(undefined4 **)(param_1 + 0x214 + param_2 * 4);
      if (puVar1 != (undefined4 *)0x0) {
        (**(code **)*puVar1)(1);
      }
      *(undefined4 *)(param_1 + 0x214 + param_2 * 4) = 0;
    }
    while (local_8 = local_8 + 1, local_8 < *(int *)(param_1 + 0x210)) {
      *(undefined4 *)(param_1 + 0x210 + local_8 * 4) =
           *(undefined4 *)(param_1 + 0x214 + local_8 * 4);
    }
    *(undefined4 *)(param_1 + 0x210 + *(int *)(param_1 + 0x210) * 4) = 0;
    *(int *)(param_1 + 0x210) = *(int *)(param_1 + 0x210) + -1;
    if (*(char *)(param_1 + 0x1e8) != '\0') {
      (**(code **)(**(int **)(param_1 + 0x20c) + 0x88))(*(undefined4 *)(param_1 + 0x210));
    }
    uVar2 = 1;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}

