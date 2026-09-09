// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b2e46 | Name: FUN_004b2e46


void FUN_004b2e46(int param_1)

{
  int local_8;
  
  DAT_0067cf40 = 0;
  DAT_0067cf44 = 0;
  local_8 = param_1;
  do {
    if (local_8 == 0) {
      return;
    }
    if ((*(char *)(local_8 + 0x14) == '\0') &&
       (DAT_0067cf40 = DAT_0067cf40 + 1, *(int *)(local_8 + 0x10) != 0)) {
      local_8 = *(int *)(local_8 + 0x10);
    }
    else {
      while (*(int *)(local_8 + 0xc) == 0) {
        local_8 = *(int *)(local_8 + 4);
        if ((local_8 == param_1) || (local_8 == 0)) goto LAB_004b2ef1;
      }
      local_8 = *(int *)(local_8 + 0xc);
    }
LAB_004b2ef1:
    DAT_0067cf44 = DAT_0067cf44 + 1;
    if (local_8 == param_1) {
      return;
    }
  } while( true );
}

