// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b6490 | Name: FUN_004b6490


undefined1 * __thiscall FUN_004b6490(undefined1 *param_1,undefined1 *param_2)

{
  *param_1 = *param_2;
  *(undefined4 *)(param_1 + 4) = 0;
  *(undefined4 *)(param_1 + 8) = 0;
  *(undefined4 *)(param_1 + 0xc) = 0;
  return param_1;
}

