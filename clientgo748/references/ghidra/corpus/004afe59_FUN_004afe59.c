// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004afe59 | Name: FUN_004afe59


void __fastcall FUN_004afe59(int param_1)

{
  undefined4 *puVar1;
  int local_8;
  
  for (local_8 = 0; local_8 < 0xbe8; local_8 = local_8 + 1) {
    if (*(int *)(param_1 + 0x48 + local_8 * 0x58) != 0) {
      puVar1 = *(undefined4 **)(param_1 + 0x48 + local_8 * 0x58);
      if (puVar1 != (undefined4 *)0x0) {
        (**(code **)*puVar1)(1);
      }
      *(undefined4 *)(param_1 + 0x48 + local_8 * 0x58) = 0;
    }
  }
  for (local_8 = 0; local_8 < 0x400; local_8 = local_8 + 1) {
    _memset((void *)(param_1 + 0x417c8 + local_8 * 0x58),0,0x12);
    if (*(int *)(param_1 + 0x41808 + local_8 * 0x58) != 0) {
      puVar1 = *(undefined4 **)(param_1 + 0x41808 + local_8 * 0x58);
      if (puVar1 != (undefined4 *)0x0) {
        (**(code **)*puVar1)(1);
      }
      *(undefined4 *)(param_1 + 0x41808 + local_8 * 0x58) = 0;
    }
    if (*(int *)(param_1 + 0x4180c + local_8 * 0x58) != 0) {
      FUN_0058ffd8(*(undefined4 *)(param_1 + 0x4180c + local_8 * 0x58));
      *(undefined4 *)(param_1 + 0x4180c + local_8 * 0x58) = 0;
    }
    if (*(int *)(param_1 + 0x41810 + local_8 * 0x58) != 0) {
      FUN_0058ffd8(*(undefined4 *)(param_1 + 0x41810 + local_8 * 0x58));
      *(undefined4 *)(param_1 + 0x41810 + local_8 * 0x58) = 0;
    }
  }
  return;
}

