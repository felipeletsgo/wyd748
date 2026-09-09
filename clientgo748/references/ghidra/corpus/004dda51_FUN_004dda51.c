// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004dda51 | Name: FUN_004dda51


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __fastcall FUN_004dda51(undefined4 *param_1)

{
  float fVar1;
  undefined4 uVar2;
  int iVar3;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0d21;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  FUN_00430f10();
  FUN_00421910(param_1 + 0x2d,0xc,10,CSimpleArray<>);
  FUN_00421910(param_1 + 0x4b,0x18,4,FUN_004cb8e0);
  *param_1 = &PTR_FUN_005a4c70;
  param_1[0x22] = 0;
  uVar2 = FUN_00430f50(0xbc75c28f,0xbe99999a,0);
  FUN_004310a0(uVar2);
  uVar2 = FUN_00430f50(0x3c75c28f,0xbe99999a,0);
  FUN_004310a0(uVar2);
  uVar2 = FUN_00430f50(0x3c75c28f,0x3e99999a,0);
  FUN_004310a0(uVar2);
  uVar2 = FUN_00430f50(0xbc75c28f,0x3e99999a,0);
  FUN_004310a0(uVar2);
  param_1[0x4f] = 0x3ca3d70a;
  param_1[0x50] = 0x3ca3d70a;
  param_1[0x55] = 0x3f7ae148;
  param_1[0x56] = 0x3ca3d70a;
  param_1[0x5b] = 0x3f7ae148;
  param_1[0x5c] = 0x3f7ae148;
  param_1[0x61] = 0x3ca3d70a;
  param_1[0x62] = 0x3f7ae148;
  param_1[0x4e] = 0x33333333;
  param_1[0x54] = 0x33333333;
  param_1[0x5a] = 0x33333333;
  param_1[0x60] = 0x33333333;
  FUN_0040bd30(&stack0x00000004);
  for (local_14 = 0; local_14 < 10; local_14 = local_14 + 1) {
    fVar1 = (float)param_1[0x20] + _DAT_005a3660;
    iVar3 = _rand();
    param_1[local_14 * 3 + 0x2d] = fVar1 - (float)(iVar3 % 5) / _DAT_005a3834;
    iVar3 = _rand();
    param_1[local_14 * 3 + 0x2f] = (float)param_1[0x21] - (float)(iVar3 % 5) / _DAT_005a3834;
    iVar3 = _rand();
    param_1[local_14 * 3 + 0x2e] = (float)(iVar3 % 10);
    iVar3 = _rand();
    param_1[local_14 + 0x23] = (float)(iVar3 % 3) * _DAT_005a4c30;
    if ((float)param_1[local_14 + 0x23] == _DAT_005a3534) {
      param_1[local_14 + 0x23] = 0x3e75c28f;
    }
  }
  ExceptionList = local_10;
  return param_1;
}

