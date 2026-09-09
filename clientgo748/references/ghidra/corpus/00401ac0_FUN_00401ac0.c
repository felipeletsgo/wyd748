// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00401ac0 | Name: FUN_00401ac0


undefined4 * __thiscall
FUN_00401ac0(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059e8af;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0040104c(param_3,param_4,param_5,param_6);
  local_8 = 0;
  FUN_0040bea0(5,0,0,0,param_5,param_6,0,0xffffffff);
  *param_1 = &PTR_FUN_005a3538;
  param_1[0x10] = 0xf;
  param_1[0x27] = param_2;
  ExceptionList = local_10;
  return param_1;
}

