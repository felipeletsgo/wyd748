// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004067e9 | Name: FUN_004067e9


undefined4 * __thiscall
FUN_004067e9(undefined4 *param_1,undefined4 param_2,char *param_3,size_t param_4,undefined4 param_5,
            undefined4 param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9,
            undefined4 param_10,undefined4 param_11,undefined4 param_12,undefined4 param_13,
            undefined4 param_14)

{
  int iVar1;
  size_t local_1c;
  size_t local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059eba9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00401d03(param_2,0,param_6,param_7,param_8,param_9,param_10,param_11,param_12,param_13,
               param_14);
  local_8 = 0;
  *(undefined1 *)(param_1 + 0x413) = 0;
  *(undefined1 *)((int)param_1 + 0x104d) = 0;
  param_1[0x414] = param_4;
  param_1[0x415] = 0;
  param_1[0x416] = param_5;
  param_1[0x417] = 0;
  param_1[0x418] = 0;
  *param_1 = &PTR_FUN_005a3974;
  param_1[0x10] = 0xd;
  param_1[0x414] = param_1[0x414];
  if (param_3 != (char *)0x0) {
    FUN_0054fc81(param_3);
    iVar1 = _strcmp(param_3,&DAT_005b10ac);
    if (iVar1 == 0) {
      param_3 = (LPCSTR)0x0;
    }
  }
  if (param_3 == (LPCSTR)0x0) {
    local_14 = 0;
  }
  else {
    iVar1 = lstrlenA(param_3);
    if ((int)param_4 < iVar1) {
      local_1c = param_4;
    }
    else {
      local_1c = lstrlenA(param_3);
    }
    local_14 = local_1c;
    _strncpy((char *)(param_1 + 0x393),param_3,local_1c);
  }
  *(undefined1 *)((int)param_1 + local_14 + 0xe4c) = 0;
  *(undefined1 *)(param_1 + 0x3d3) = 0;
  ExceptionList = local_10;
  return param_1;
}

