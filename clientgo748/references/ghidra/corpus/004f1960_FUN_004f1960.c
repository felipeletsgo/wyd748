// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f1960 | Name: FUN_004f1960


undefined4 * __fastcall FUN_004f1960(undefined4 *param_1)

{
  FUN_004f3a9b();
  *param_1 = &PTR_FUN_005a5460;
  *(undefined2 *)((int)param_1 + 0x302) = 0;
  *(undefined2 *)(param_1 + 0xc0) = 0;
  param_1[0xb8] = 0;
  _memset(param_1 + 0x58,0,8);
  return param_1;
}

