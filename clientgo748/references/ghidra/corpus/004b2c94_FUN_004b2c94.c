// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b2c94 | Name: FUN_004b2c94


int FUN_004b2c94(int param_1)

{
  int iVar1;
  int local_8;
  
  if (param_1 != 0) {
    iVar1 = *(int *)(DAT_0067cf38 + 0x38);
    local_8 = iVar1;
    do {
      if (local_8 == 0) {
        return 0;
      }
      if (*(char *)(local_8 + 0x14) == '\0') {
        if (*(int *)(local_8 + 0x20) == param_1) {
          return local_8;
        }
        if (*(int *)(local_8 + 0x10) == 0) goto LAB_004b2cf4;
        local_8 = *(int *)(local_8 + 0x10);
      }
      else {
LAB_004b2cf4:
        while (*(int *)(local_8 + 0xc) == 0) {
          local_8 = *(int *)(local_8 + 4);
          if ((local_8 == iVar1) || (local_8 == 0)) goto LAB_004b2d1f;
        }
        local_8 = *(int *)(local_8 + 0xc);
      }
LAB_004b2d1f:
    } while (local_8 != iVar1);
  }
  return 0;
}

