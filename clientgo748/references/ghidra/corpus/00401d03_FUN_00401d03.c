// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00401d03 | Name: FUN_00401d03


undefined4 * __thiscall
FUN_00401d03(undefined4 *param_1,undefined4 param_2,char *param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,
            undefined1 param_9,undefined4 param_10,undefined4 param_11,undefined4 param_12)

{
  int iVar1;
  size_t sVar2;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059e90e;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0040104c(param_5,param_6,param_7,param_8);
  local_8 = 0;
  FUN_004219a0();
  local_8._0_1_ = 1;
  FUN_004219a0();
  local_8._0_1_ = 2;
  FUN_004219a0();
  local_8._0_1_ = 3;
  FUN_004219a0();
  local_8 = CONCAT31(local_8._1_3_,4);
  FUN_0040bea0(0,param_2,0,0,0,0,0,param_4);
  FUN_0040bea0(0,param_2,0,0,0,0,0,param_4);
  FUN_0040bea0(0,param_2,0,0,0,0,0,param_4);
  FUN_0040bea0(0,param_2,0,0,0,0,0,param_4);
  param_1[0x335] = param_12;
  param_1[0x336] = param_11;
  *(undefined1 *)(param_1 + 0x337) = param_9;
  FUN_0040bea0(4,param_2,0,0,0,0,0,param_10);
  *param_1 = &PTR_FUN_005a35c8;
  *(undefined1 *)((int)param_1 + 0xcdd) = 0;
  param_1[0x10] = 0xc;
  FUN_004021f5(param_1[0x336]);
  *(undefined1 *)((int)param_1 + 0x76a) = 0;
  param_1[0x21c] = param_1 + 0x19;
  *(undefined1 *)((int)param_1 + 0x8d6) = 0;
  param_1[0x277] = param_1 + 0x85;
  *(undefined1 *)((int)param_1 + 0xa42) = 0;
  param_1[0x2d2] = param_1 + 0xf1;
  *(undefined1 *)((int)param_1 + 0xbae) = 0;
  param_1[0x32d] = param_1 + 0x15d;
  if (param_3 != (char *)0x0) {
    FUN_0054fc81(param_3);
    iVar1 = _strcmp(param_3,&DAT_005b1098);
    if (iVar1 == 0) {
      param_3 = (char *)0x0;
    }
  }
  if (param_3 != (char *)0x0) {
    sVar2 = _strlen(param_3);
    if (sVar2 != 0) {
      FUN_0058ee20((int)param_1 + 0x76a,param_3);
      FUN_00421b8f((int)param_1 + 0x76a,param_1[0x1d5],0);
    }
  }
  ExceptionList = local_10;
  return param_1;
}

