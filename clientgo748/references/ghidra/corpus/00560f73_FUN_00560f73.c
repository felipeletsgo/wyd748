// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00560f73 | Name: FUN_00560f73


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_00560f73(float param_1,float param_2)

{
  param_1 = param_1 - param_2;
  if ((_DAT_005a67fc < param_1 != (_DAT_005a67fc == param_1)) &&
     (param_1 < _DAT_005a67f8 != (param_1 == _DAT_005a67f8))) {
    return 1;
  }
  return 0;
}

