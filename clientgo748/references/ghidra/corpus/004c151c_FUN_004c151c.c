// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c151c | Name: FUN_004c151c


undefined4 * __thiscall FUN_004c151c(undefined4 *param_1,undefined4 param_2)

{
  FUN_004c5b40();
  FUN_004c5b40();
  *param_1 = param_2;
  FUN_00430fc0(param_1 + 2);
  FUN_00430fc0(param_1 + 0x12);
  param_1[0x22] = 0;
  param_1[0x23] = 0;
  param_1[0x24] = 0;
  param_1[0x25] = 0;
  return param_1;
}

