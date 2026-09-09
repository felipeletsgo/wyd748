// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040cc51 | Name: FUN_0040cc51


undefined4 __fastcall FUN_0040cc51(int param_1)

{
  int *piVar1;
  float local_18;
  float local_14;
  int local_c;
  int *local_8;
  
  FUN_00430f20(0,0);
  local_c = 0;
  piVar1 = *(int **)(param_1 + 0x28);
  if ((piVar1 != (int *)0x0) && (local_8 = piVar1, *(int *)(param_1 + 0x34) != 1)) {
    do {
      if ((char)local_8[5] == '\0') {
        if ((local_8[10] != 1) ||
           ((**(code **)(*local_8 + 0x58))(param_1 + 0x3c,local_18,local_14,local_c,0),
           local_8[4] == 0)) goto LAB_0040cd14;
        local_18 = local_18 + (float)local_8[0x13];
        local_14 = local_14 + (float)local_8[0x14];
        local_c = local_c + 1;
        local_8 = (int *)local_8[4];
      }
      else {
        *(undefined4 *)(param_1 + 0x38) = 1;
LAB_0040cd14:
        while (local_8[3] == 0) {
          local_8 = (int *)local_8[1];
          local_18 = local_18 - (float)local_8[0x13];
          local_14 = local_14 - (float)local_8[0x14];
          if ((local_8 == piVar1) || (local_8 == (int *)0x0)) goto LAB_0040cd57;
        }
        local_8 = (int *)local_8[3];
      }
LAB_0040cd57:
    } while ((local_8 != piVar1) && (local_8 != (int *)0x0));
    if (*(int *)(*(int *)(param_1 + 0x30) + 0x28) != 0) {
      (**(code **)(**(int **)(param_1 + 0x30) + 0x58))(param_1 + 0x3c,local_18,local_14,0x1d,0);
    }
  }
  return 1;
}

