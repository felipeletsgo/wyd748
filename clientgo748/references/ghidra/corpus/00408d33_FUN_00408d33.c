// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00408d33 | Name: FUN_00408d33


undefined4 __thiscall FUN_00408d33(int param_1,undefined4 param_2)

{
  int local_8;
  
  if (*(int *)(param_1 + 0x210) < *(int *)(param_1 + 0x1f0) - (int)*(char *)(param_1 + 0x1e9)) {
    *(undefined4 *)(param_1 + 0x214 + *(int *)(param_1 + 0x210) * 4) = param_2;
    *(int *)(param_1 + 0x210) = *(int *)(param_1 + 0x210) + 1;
    if (((*(char *)(param_1 + 0x1e8) != '\0') && (*(int *)(param_1 + 0x20c) != 0)) &&
       ((**(code **)(**(int **)(param_1 + 0x20c) + 0x88))(*(undefined4 *)(param_1 + 0x210)),
       *(int *)(param_1 + 500) <= *(int *)(param_1 + 0x210))) {
      FUN_0040927a((*(int *)(param_1 + 0x210) - *(int *)(param_1 + 500)) +
                   (int)*(char *)(param_1 + 0x1e9));
      (**(code **)(**(int **)(param_1 + 0x20c) + 0x88))(*(undefined4 *)(param_1 + 0x210));
      (**(code **)(**(int **)(param_1 + 0x20c) + 0x80))
                ((*(int *)(param_1 + 0x210) - *(int *)(param_1 + 500)) +
                 (int)*(char *)(param_1 + 0x1e9));
    }
  }
  else {
    if (*(int *)(param_1 + 0x214) != 0) {
      if (*(undefined4 **)(param_1 + 0x214) != (undefined4 *)0x0) {
        (**(code **)**(undefined4 **)(param_1 + 0x214))(1);
      }
      *(undefined4 *)(param_1 + 0x214) = 0;
    }
    for (local_8 = 1; local_8 < *(int *)(param_1 + 0x210); local_8 = local_8 + 1) {
      *(undefined4 *)(param_1 + 0x210 + local_8 * 4) =
           *(undefined4 *)(param_1 + 0x214 + local_8 * 4);
    }
    *(undefined4 *)(param_1 + 0x210 + *(int *)(param_1 + 0x210) * 4) = param_2;
  }
  return 1;
}

