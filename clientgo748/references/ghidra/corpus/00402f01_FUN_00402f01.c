// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00402f01 | Name: FUN_00402f01


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_00402f01(undefined4 *param_1,int param_2,float param_3,float param_4,float param_5,
            undefined4 param_6,undefined4 param_7,undefined4 param_8,char *param_9)

{
  int iVar1;
  void *pvVar2;
  size_t sVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 local_34;
  undefined4 local_28;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059e983;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00401466(param_2,param_3 + _DAT_005a3660,param_4 - _DAT_005a3660,param_5,param_6,param_7,4);
  local_8 = 0;
  param_1[0x79] = 0;
  param_1[0x7a] = 0;
  *(undefined1 *)(param_1 + 0x7b) = 0;
  *(undefined1 *)((int)param_1 + 0x1ed) = 0;
  param_1[0x7c] = 0;
  param_1[0x7f] = param_7;
  param_1[0x80] = param_8;
  *param_1 = &PTR_FUN_005a3674;
  param_1[0x81] = 0;
  param_1[0x10] = 2;
  if (param_9 != (char *)0x0) {
    FUN_0054fc81(param_9);
    iVar1 = _strcmp(param_9,&DAT_005b10a0);
    if (iVar1 == 0) {
      param_9 = (char *)0x0;
    }
  }
  if (param_9 != (char *)0x0) {
    if ((param_2 < 0) && (-3 < param_2)) {
      param_1[0x6c] = 0;
      pvVar2 = operator_new(0x1b0);
      local_8._0_1_ = 1;
      if (pvVar2 == (void *)0x0) {
        local_28 = 0;
      }
      else {
        local_28 = FUN_004219a0();
      }
      local_8 = (uint)local_8._1_3_ << 8;
      param_1[0x6c] = local_28;
      FUN_0058f078((int)param_1 + 0xaa,&DAT_005b10a4,param_9);
      if (param_1[0x6c] != 0) {
        FUN_00421b8f((int)param_1 + 0xaa,0xffffffff,0);
      }
    }
    pvVar2 = operator_new(0xe4c);
    local_8._0_1_ = 2;
    if (pvVar2 == (void *)0x0) {
      local_34 = 0;
    }
    else {
      uVar8 = 0;
      uVar7 = 1;
      uVar6 = 0xff333333;
      uVar5 = 0;
      uVar4 = 0x41400000;
      sVar3 = _strlen(param_9);
      local_34 = FUN_00401d03(0xffffffff,param_9,0xffffffff,param_5 - _DAT_005a3670,0,
                              (float)sVar3 * _DAT_005a366c,uVar4,uVar5,uVar6,uVar7,uVar8);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x7c] = local_34;
    (**(code **)(*(int *)param_1[0x7c] + 0x60))(0);
    (**(code **)(*(int *)param_1[0x7c] + 0x5c))(1);
  }
  param_1[0x7e] = 0;
  FUN_00403853();
  ExceptionList = local_10;
  return param_1;
}

