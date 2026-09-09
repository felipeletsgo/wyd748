// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00405065 | Name: FUN_00405065


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_00405065(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,float param_6,undefined4 param_7,float param_8,uint param_9,
            undefined4 param_10,undefined4 param_11)

{
  int iVar1;
  int local_24;
  int local_20;
  uint local_1c;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059eadd;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00401466(param_2,param_5,param_6,param_7,param_8,param_10,4);
  local_8 = 0;
  param_1[0x7a] = param_3;
  param_1[0x7b] = param_4;
  FUN_0040bea0(4,param_2,param_5,param_6,0,param_8,0,param_9);
  *param_1 = &PTR_FUN_005a3838;
  param_1[0x19] = 4;
  param_1[0x29] = 1;
  param_1[0x79] = param_11;
  if ((param_1[0x79] == 1) || (param_1[0x79] == 2)) {
    param_1[0x80] = param_6 + _DAT_005a365c;
    param_1[0x82] = param_8 - _DAT_005a3834;
    param_1[0x7e] = 4;
  }
  param_1[0x10] = 10;
  if (0x3a < DAT_005b491c) {
    iVar1 = DAT_005b491c + -0x28;
    local_20 = ((param_9 & 0xff0000) >> 0x10) - iVar1;
    local_24 = ((param_9 & 0xff00) >> 8) - iVar1;
    local_1c = (param_9 & 0xff) - iVar1;
    if (local_20 < 0) {
      local_20 = 0;
    }
    if (local_24 < 0) {
      local_24 = 0;
    }
    if ((int)local_1c < 0) {
      local_1c = 0;
    }
    param_1[0x8a] = param_9 & 0xff000000 | local_20 << 0x10 | local_24 << 8 | local_1c;
  }
  local_14 = param_1[0x8d];
  if (local_14 < -2) {
    local_14 = -param_1[0x8d];
  }
  iVar1 = FUN_004b8c45(local_14);
  if (iVar1 != 0) {
    param_1[0xd9] = (float)*(int *)(*(int *)(iVar1 + 4) + 0x10 + param_1[0x8e] * 0x1c);
    param_1[0xda] = (float)*(int *)(*(int *)(iVar1 + 4) + 8 + param_1[0x8e] * 0x1c);
    param_1[0xdb] = (float)*(int *)(*(int *)(iVar1 + 4) + 0xc + param_1[0x8e] * 0x1c);
    param_1[0xdc] = (float)*(int *)(*(int *)(iVar1 + 4) + 4 + param_1[0x8e] * 0x1c);
  }
  FUN_004054f2();
  ExceptionList = local_10;
  return param_1;
}

