// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040c1b0 | Name: FUN_0040c1b0


void FUN_0040c1b0(int param_1,int param_2,int param_3)

{
  int local_c;
  int local_8;
  
  if ((param_3 < 0x1e) && (local_8 = *(int *)(param_1 + param_3 * 8), local_8 != 0)) {
    if (local_8 == param_2) {
      *(undefined4 *)(param_1 + param_3 * 8) = *(undefined4 *)(local_8 + 0x150);
    }
    else {
      local_c = 0;
      do {
        if (local_8 == 0) {
          return;
        }
        if (*(int *)(local_8 + 0x150) == 0) {
          return;
        }
        if (*(int *)(local_8 + 0x150) == param_2) {
          *(undefined4 *)(local_8 + 0x150) = *(undefined4 *)(*(int *)(local_8 + 0x150) + 0x150);
          return;
        }
        local_8 = *(int *)(local_8 + 0x150);
        local_c = local_c + 1;
        if (DAT_005ccf00 < local_c) {
          DAT_005ccf00 = local_c;
        }
      } while (local_c < 0x65);
    }
  }
  return;
}

