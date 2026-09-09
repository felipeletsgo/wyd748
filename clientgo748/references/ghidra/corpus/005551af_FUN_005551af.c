// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005551af | Name: FUN_005551af


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_005551af(void)

{
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  _memset(&DAT_00e33b14,0,0x1000);
  local_10 = 0x200;
  local_8 = 0;
  local_c = 0;
  do {
    for (local_14 = 0; local_14 < 0x400; local_14 = local_14 + local_10) {
      if (*(int *)(&DAT_00e33b14 + local_14 * 4) == 0) {
        if (local_c == 0) {
          *(int *)(&DAT_00e33b14 + local_14 * 4) = local_8;
        }
        else if (local_c == 1) {
          *(int *)(&DAT_00e33b14 + local_14 * 4) = 0x200 - local_8;
        }
        else if (local_c == 2) {
          *(int *)(&DAT_00e33b14 + local_14 * 4) = local_8 + 0x200;
        }
        else {
          *(int *)(&DAT_00e33b14 + local_14 * 4) = 0x400 - local_8;
        }
        if (999 < *(int *)(&DAT_00e33b14 + local_14 * 4)) {
          *(undefined4 *)(&DAT_00e33b14 + local_14 * 4) = 999;
        }
        local_c = local_c + 1;
        if (3 < local_c) {
          local_c = 0;
        }
        if (local_c == 0) {
          local_8 = local_8 + 1;
        }
      }
    }
    local_10 = local_10 / 2;
  } while (local_10 != 0);
  _DAT_00e33b14 = 0x200;
  return;
}

