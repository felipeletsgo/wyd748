// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00409cbc | Name: FUN_00409cbc


undefined4 * __thiscall
FUN_00409cbc(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059ed45;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00401466(param_2,param_3,param_4,param_5,param_6,0x77777777,4);
  local_8 = 0;
  param_1[0x79] = 0;
  FUN_0040bdb0();
  *param_1 = &PTR_FUN_005a3e00;
  param_1[0x10] = 0;
  param_1[0x29] = 0;
  param_1[0x13] = param_3;
  param_1[0x14] = param_4;
  DAT_005ccec0 = param_1;
  param_1[0x7a] = 0;
  ExceptionList = local_10;
  return param_1;
}

