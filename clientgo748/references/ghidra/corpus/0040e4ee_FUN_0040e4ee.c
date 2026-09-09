// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040e4ee | Name: FUN_0040e4ee


void __fastcall FUN_0040e4ee(int param_1)

{
  undefined4 *puVar1;
  int local_8;
  
  _memset(*(void **)(param_1 + 0x1f0),0,*(int *)(param_1 + 0x1e8) * 4 * *(int *)(param_1 + 0x1ec));
  for (local_8 = 0; local_8 < *(int *)(param_1 + 0x1fc); local_8 = local_8 + 1) {
    if ((*(int *)(DAT_005ccec0 + 0x1e8) != 0) &&
       (*(int *)(DAT_005ccec0 + 0x1e8) == *(int *)(param_1 + 0x200 + local_8 * 4))) {
      *(undefined4 *)(DAT_005ccec0 + 0x1e8) = 0;
    }
    if (*(int *)(param_1 + 0x200 + local_8 * 4) != 0) {
      puVar1 = *(undefined4 **)(param_1 + 0x200 + local_8 * 4);
      if (puVar1 != (undefined4 *)0x0) {
        (**(code **)*puVar1)(1);
      }
      *(undefined4 *)(param_1 + 0x200 + local_8 * 4) = 0;
    }
  }
  *(undefined4 *)(param_1 + 0x1fc) = 0;
  return;
}

