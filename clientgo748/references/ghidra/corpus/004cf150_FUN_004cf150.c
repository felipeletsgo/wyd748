// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004cf150 | Name: FUN_004cf150


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_004cf150(undefined4 *param_1,float param_2,float param_3,float param_4,float param_5,
            undefined4 param_6)

{
  undefined4 uVar1;
  uint uVar2;
  int iVar3;
  void *pvVar4;
  int local_34;
  int local_20;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a08d4;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  FUN_00421910(param_1 + 0x27,0xc,4,CSimpleArray<>);
  *param_1 = &PTR_FUN_005a48e8;
  param_1[0x22] = 500;
  param_1[0x20] = param_6;
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x21] = uVar1;
  FUN_004310a0(&param_2);
  uVar2 = _rand();
  uVar2 = uVar2 & 0x80000007;
  if ((int)uVar2 < 0) {
    uVar2 = (uVar2 - 1 | 0xfffffff8) + 1;
  }
  param_1[0x1c] = (float)(int)uVar2 * _DAT_005a4158;
  param_3 = param_3 + _DAT_005a3794;
  _memset(param_1 + 0x23,0,0x10);
  if (param_1[0x20] == 0) {
    for (local_14 = 0; local_14 < 4; local_14 = local_14 + 1) {
      iVar3 = _rand();
      iVar3 = iVar3 % 5;
      pvVar4 = operator_new(0x180);
      local_8._0_1_ = 1;
      if (pvVar4 == (void *)0x0) {
        local_34 = 0;
      }
      else {
        local_34 = FUN_004c8f70(0x77,local_14 * 100 + 1000,
                                (float)iVar3 * _DAT_005a4214 + (float)param_1[0x1c],
                                (float)iVar3 * _DAT_005a40ec + param_5,
                                (float)iVar3 * _DAT_005a4214 + (float)param_1[0x1c],0x38d1b717,1,
                                0x50);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      if (local_34 != 0) {
        *(float *)(local_34 + 0x74) = (float)(iVar3 + -3) * _DAT_005a439c + param_2;
        *(float *)(local_34 + 0x7c) = param_4 - (float)(local_14 + 3) * _DAT_005a41e8;
        *(float *)(local_34 + 0x78) = param_3;
        FUN_004310a0(local_34 + 0x74);
        *(undefined4 *)(local_34 + 0x100) = 0;
        *(undefined4 *)(local_34 + 0x148) = 1;
        *(float *)(local_34 + 0x140) = (float)local_14 * _DAT_005a4214 + _DAT_005a48e4;
        FUN_0054ac09(local_34);
      }
    }
  }
  else if (param_1[0x20] == 1) {
    param_1[0x22] = 0xa8c;
    for (local_20 = 0; local_20 < 4; local_20 = local_20 + 1) {
      *(undefined1 *)((int)param_1 + local_20 + 0xcc) = 0;
      uVar1 = FUN_004b14fd(local_20 + 0x28,0,180000);
      param_1[local_20 + 0x23] = uVar1;
      if (param_1[local_20 + 0x23] != 0) {
        *(undefined4 *)(param_1[local_20 + 0x23] + 0x32c) = 0x3fc00000;
        *(undefined4 *)(param_1[local_20 + 0x23] + 0x330) = 0x40133333;
      }
      FUN_004310a0(&param_2);
      iVar3 = _rand();
      param_1[local_20 * 3 + 0x27] =
           (float)(local_20 + -2 + iVar3 % 3) * _DAT_005a34a0 + (float)param_1[local_20 * 3 + 0x27];
      iVar3 = _rand();
      param_1[local_20 * 3 + 0x29] =
           (float)(local_20 + -2 + iVar3 % 3) * _DAT_005a34a0 + (float)param_1[local_20 * 3 + 0x29];
      param_1[local_20 * 3 + 0x28] = param_3 + _DAT_005a3794 + (float)(local_20 * 5);
    }
  }
  ExceptionList = local_10;
  return param_1;
}

