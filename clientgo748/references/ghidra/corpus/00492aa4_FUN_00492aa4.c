// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00492aa4 | Name: FUN_00492aa4


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_00492aa4(int param_1)

{
  _DAT_00d52b20 = *(undefined4 *)(param_1 + 0xc);
  _DAT_00d52bac = *(undefined4 *)(param_1 + 0x10);
  return 1;
}

