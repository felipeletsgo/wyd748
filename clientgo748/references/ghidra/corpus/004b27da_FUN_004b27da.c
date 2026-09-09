// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b27da | Name: FUN_004b27da


void FUN_004b27da(int param_1)

{
  int iVar1;
  int local_8;
  
  if ((param_1 != 0) && (*(int *)(*(int *)(param_1 + 0x7bc) + 0x24) == 30000)) {
    local_8 = *(int *)(*(int *)(param_1 + 0x7bc) + 0x2c);
    iVar1 = *(int *)(*(int *)(param_1 + 0x7bc) + 0x2c);
    do {
      if (local_8 == 0) {
        return;
      }
      if (*(int *)(local_8 + 0x1c) == param_1) {
        *(undefined4 *)(local_8 + 0x1c) = 0;
      }
      if (*(int *)(local_8 + 0x10) == 0) {
        while (*(int *)(local_8 + 0xc) == 0) {
          local_8 = *(int *)(local_8 + 4);
          if ((local_8 == iVar1) || (local_8 == 0)) goto LAB_004b287f;
        }
        local_8 = *(int *)(local_8 + 0xc);
      }
      else {
        local_8 = *(int *)(local_8 + 0x10);
      }
LAB_004b287f:
    } while (local_8 != iVar1);
  }
  return;
}

