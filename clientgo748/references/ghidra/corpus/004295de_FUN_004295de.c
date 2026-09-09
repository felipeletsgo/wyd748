// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004295de | Name: FUN_004295de


void __fastcall FUN_004295de(int *param_1)

{
  int local_8;
  
  if (param_1[0x8403] != 0) {
    (**(code **)(*(int *)param_1[0x8403] + 8))(param_1[0x8403]);
    param_1[0x8403] = 0;
  }
  for (local_8 = 0; local_8 < 0x200; local_8 = local_8 + 1) {
    if (param_1[local_8 * 0x42 + 2] != 0) {
      if ((undefined4 *)param_1[local_8 * 0x42 + 2] != (undefined4 *)0x0) {
        (*(code *)**(undefined4 **)param_1[local_8 * 0x42 + 2])(1);
      }
      param_1[local_8 * 0x42 + 2] = 0;
    }
  }
  if (*param_1 != 0) {
    (**(code **)(*(int *)*param_1 + 8))(*param_1);
    *param_1 = 0;
  }
  DAT_005ccf98 = 0;
  return;
}

