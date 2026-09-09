// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004dcdc0 | Name: FUN_004dcdc0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __fastcall FUN_004dcdc0(undefined4 *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0cf9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  FUN_00421910(param_1 + 0x53,0xc,0x32,CSimpleArray<>);
  FUN_00421910(param_1 + 0xe9,0x18,4,FUN_004cb8e0);
  *param_1 = &PTR_FUN_005a4c34;
  param_1[0x20] = 0;
  uVar1 = FUN_00430f50(0xbc03126f,0xbe4ccccd,0);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(0x3c03126f,0xbe4ccccd,0);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(0x3c03126f,0x3e4ccccd,0);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(0xbc03126f,0x3e4ccccd,0);
  FUN_004310a0(uVar1);
  param_1[0xed] = 0x3ca3d70a;
  param_1[0xee] = 0x3ca3d70a;
  param_1[0xf3] = 0x3f7ae148;
  param_1[0xf4] = 0x3ca3d70a;
  param_1[0xf9] = 0x3f7ae148;
  param_1[0xfa] = 0x3f7ae148;
  param_1[0xff] = 0x3ca3d70a;
  param_1[0x100] = 0x3f7ae148;
  param_1[0xec] = 0x33333333;
  param_1[0xf2] = 0x33333333;
  param_1[0xf8] = 0x33333333;
  param_1[0xfe] = 0x33333333;
  for (local_14 = 0; local_14 < 0x32; local_14 = local_14 + 1) {
    iVar2 = _rand();
    param_1[local_14 * 3 + 0x53] = _DAT_005a3794 - (float)(iVar2 % 0x18) / _DAT_005a365c;
    iVar2 = _rand();
    param_1[local_14 * 3 + 0x55] = _DAT_005a3794 - (float)(iVar2 % 0x18) / _DAT_005a365c;
    iVar2 = _rand();
    param_1[local_14 * 3 + 0x54] = (float)(iVar2 % 10);
    iVar2 = _rand();
    param_1[local_14 + 0x21] = (float)(iVar2 % 3) * _DAT_005a4c30;
    if ((float)param_1[local_14 + 0x21] == _DAT_005a3534) {
      param_1[local_14 + 0x21] = 0x3e75c28f;
    }
  }
  ExceptionList = local_10;
  return param_1;
}

