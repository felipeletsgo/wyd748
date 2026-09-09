// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040a9cc | Name: FUN_0040a9cc


void __thiscall FUN_0040a9cc(int param_1,int param_2)

{
  int local_20;
  float local_18 [2];
  int local_10;
  int local_c;
  int local_8;
  
  *(int *)(param_1 + 0x28) = param_2;
  for (local_8 = 0; local_8 < 3; local_8 = local_8 + 1) {
    for (local_c = 0; local_c < 0x16; local_c = local_c + 1) {
      if (param_2 == 0) {
        (**(code **)(**(int **)(param_1 + 0x1e4 + local_8 * 0x58 + local_c * 4) + 0x60))(0);
      }
      else {
        if (DAT_005b892c == 2) {
          local_10 = __ftol();
        }
        else {
          local_10 = __ftol();
        }
        FUN_0040c110(local_18);
        if (DAT_005b892c != 2) {
          local_18[0] = ((float)*(int *)(param_1 + 0x3ac) + *(float *)(param_1 + 0x3a4)) *
                        (float)local_8;
        }
        (**(code **)(**(int **)(param_1 + 0x1e4 + local_8 * 0x58 + local_c * 4) + 0x70))
                  (local_18[0],(float)((local_10 * local_c) % (local_10 * 0x16) - local_10));
        if ((local_c < 0) || (3 < local_c)) {
          (**(code **)(**(int **)(param_1 + 0x1e4 + local_8 * 0x58 + local_c * 4) + 0x60))(0);
        }
        else {
          (**(code **)(**(int **)(param_1 + 0x1e4 + local_8 * 0x58 + local_c * 4) + 0x60))(param_2);
        }
      }
    }
  }
  if (param_2 == 0) {
    for (local_20 = 0; local_20 < 9; local_20 = local_20 + 1) {
      (**(code **)(**(int **)(param_1 + 0x2ec + local_20 * 4) + 0x60))(0);
      (**(code **)(**(int **)(param_1 + 0x310 + local_20 * 4) + 0x60))(0);
      *(undefined1 *)(param_1 + local_20 + 0x360) = 0;
    }
  }
  return;
}

