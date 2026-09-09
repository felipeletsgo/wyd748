// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004287c2 | Name: FUN_004287c2


undefined4 * __thiscall FUN_004287c2(undefined4 *param_1,int param_2,undefined4 param_3)

{
  int *local_24;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059efeb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  param_1[5] = 0;
  local_24 = operator_new(param_2 * 0x18 + 4);
  local_8 = 0;
  if (local_24 == (int *)0x0) {
    local_24 = (int *)0x0;
  }
  else {
    *local_24 = param_2;
    FUN_00590340(local_24 + 1,0x18,param_2,FUN_00428200,FUN_00428344);
    local_24 = local_24 + 1;
  }
  param_1[1] = local_24;
  if ((local_24 == (int *)0x0) || (param_2 < 1)) {
    *(undefined1 *)(param_1 + 2) = 0;
  }
  else {
    param_1[4] = param_2;
    param_1[3] = 1;
    *(undefined1 *)(param_1 + 2) = 1;
    *param_1 = param_3;
  }
  ExceptionList = local_10;
  return param_1;
}

