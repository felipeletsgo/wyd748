// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00401466 | Name: FUN_00401466


undefined4 * __thiscall
FUN_00401466(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059e889;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0040104c(param_3,param_4,param_5,param_6);
  local_8 = 0;
  FUN_0040bea0(param_8,param_2,0,0,param_5,param_6,0,param_7);
  param_1[0x74] = 0;
  param_1[0x77] = 0;
  param_1[0x78] = 0;
  *param_1 = &PTR_FUN_005a34ac;
  param_1[0x10] = 1;
  ExceptionList = local_10;
  return param_1;
}

