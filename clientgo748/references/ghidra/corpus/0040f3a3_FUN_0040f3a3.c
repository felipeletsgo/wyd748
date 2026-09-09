// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040f3a3 | Name: FUN_0040f3a3


int __thiscall FUN_0040f3a3(int param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  
  local_14 = -1;
  local_c = 0;
  do {
    local_10 = 0;
    if (*(int *)(param_1 + 0x1fc) <= local_c) {
LAB_0040f417:
      if (local_10 == 0) {
        local_10 = 0;
      }
      else {
        for (local_18 = *(int *)(local_10 + 0x1d4);
            local_18 < *(int *)(local_10 + 0x1d4) + *(int *)(local_10 + 0x1dc);
            local_18 = local_18 + 1) {
          for (local_1c = *(int *)(local_10 + 0x1d0);
              local_1c < *(int *)(local_10 + 0x1d0) + *(int *)(local_10 + 0x1d8);
              local_1c = local_1c + 1) {
            if ((local_1c < *(int *)(param_1 + 0x1ec)) && (local_18 < *(int *)(param_1 + 0x1e8))) {
              *(undefined4 *)
               (*(int *)(param_1 + 0x1f0) + (local_18 * *(int *)(param_1 + 0x1ec) + local_1c) * 4) =
                   0;
            }
          }
        }
        if (((local_14 != -1) && (local_14 < *(int *)(param_1 + 0x1fc))) &&
           (local_20 = local_14, -1 < local_14)) {
          while (local_20 = local_20 + 1, local_20 < *(int *)(param_1 + 0x1fc)) {
            *(undefined4 *)(param_1 + 0x1fc + local_20 * 4) =
                 *(undefined4 *)(param_1 + 0x200 + local_20 * 4);
          }
          *(undefined4 *)(param_1 + 0x1fc + *(int *)(param_1 + 0x1fc) * 4) = 0;
          *(int *)(param_1 + 0x1fc) = *(int *)(param_1 + 0x1fc) + -1;
        }
      }
      return local_10;
    }
    local_10 = *(int *)(param_1 + 0x200 + local_c * 4);
    if ((local_10 != 0) && (iVar1 = FUN_0040dee8(param_2,param_3), iVar1 == 1)) {
      local_14 = local_c;
      goto LAB_0040f417;
    }
    local_c = local_c + 1;
  } while( true );
}

