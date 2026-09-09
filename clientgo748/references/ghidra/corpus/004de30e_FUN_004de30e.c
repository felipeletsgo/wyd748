// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004de30e | Name: FUN_004de30e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall FUN_004de30e(undefined4 *param_1,float param_2)

{
  undefined4 uVar1;
  int iVar2;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0d49;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  FUN_00421910(param_1 + 0xe9,0xc,200,CSimpleArray<>);
  FUN_00421910(param_1 + 0x341,0x18,4,FUN_004cb8e0);
  *param_1 = &PTR_FUN_005a4cb0;
  param_1[0x359] = param_2 * _DAT_005a439c;
  param_1[0x20] = 0;
  uVar1 = FUN_00430f50(_DAT_005a4cac * param_2,_DAT_005a4cac * param_2,0);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(_DAT_005a439c * param_2,_DAT_005a4cac * param_2,0);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(_DAT_005a439c * param_2,_DAT_005a439c * param_2,0);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(_DAT_005a4cac * param_2,_DAT_005a439c * param_2,0);
  FUN_004310a0(uVar1);
  param_1[0x345] = 0x3ca3d70a;
  param_1[0x346] = 0x3ca3d70a;
  param_1[0x34b] = 0x3f7ae148;
  param_1[0x34c] = 0x3ca3d70a;
  param_1[0x351] = 0x3f7ae148;
  param_1[0x352] = 0x3f7ae148;
  param_1[0x357] = 0x3ca3d70a;
  param_1[0x358] = 0x3f7ae148;
  param_1[0x344] = 0xaaaaaaaa;
  param_1[0x34a] = 0xaaaaaaaa;
  param_1[0x350] = 0xaaaaaaaa;
  param_1[0x356] = 0xaaaaaaaa;
  for (local_14 = 0; local_14 < 200; local_14 = local_14 + 1) {
    iVar2 = _rand();
    param_1[local_14 * 3 + 0xe9] = _DAT_005a3794 - (float)(iVar2 % 0x18) / _DAT_005a365c;
    iVar2 = _rand();
    param_1[local_14 * 3 + 0xeb] = _DAT_005a3794 - (float)(iVar2 % 0x18) / _DAT_005a365c;
    iVar2 = _rand();
    param_1[local_14 * 3 + 0xea] = (float)(iVar2 % 10);
    iVar2 = _rand();
    param_1[local_14 + 0x21] = (float)(iVar2 % 3) * _DAT_005a4740;
    if ((float)param_1[local_14 + 0x21] == _DAT_005a3534) {
      param_1[local_14 + 0x21] = 0x3e75c28f;
    }
  }
  ExceptionList = local_10;
  return param_1;
}

