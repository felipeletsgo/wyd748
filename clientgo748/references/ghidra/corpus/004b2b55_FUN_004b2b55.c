// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b2b55 | Name: FUN_004b2b55


int FUN_004b2b55(int param_1)

{
  int local_8;
  
  local_8 = DAT_0067cf38;
  if (param_1 != 0) {
    do {
      if (local_8 == 0) {
        return 0;
      }
      if (*(char *)(local_8 + 0x14) == '\0') {
        if (*(int *)(local_8 + 0x20) == param_1) {
          return local_8;
        }
        if (*(int *)(local_8 + 0x10) == 0) goto LAB_004b2bb2;
        local_8 = *(int *)(local_8 + 0x10);
      }
      else {
LAB_004b2bb2:
        while (*(int *)(local_8 + 0xc) == 0) {
          local_8 = *(int *)(local_8 + 4);
          if ((local_8 == DAT_0067cf38) || (local_8 == 0)) goto LAB_004b2bdd;
        }
        local_8 = *(int *)(local_8 + 0xc);
      }
LAB_004b2bdd:
    } while (local_8 != DAT_0067cf38);
  }
  return 0;
}

