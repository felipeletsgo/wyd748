// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d97d0 | Name: FUN_004d97d0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __fastcall FUN_004d97d0(undefined4 *param_1)

{
  float fVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  int local_18;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0be9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  FUN_00421910(param_1 + 0x23,0x18,0x20,FUN_004cb8e0);
  FUN_00421910(param_1 + 0xe3,0x40,0x30,FUN_00430f10);
  FUN_00421910(param_1 + 0x3e3,0x40,0x30,FUN_00430f10);
  FUN_00421910(param_1 + 0x714,0x40,0x30,FUN_00430f10);
  FUN_00430f10();
  FUN_00430f10();
  *param_1 = &PTR_FUN_005a4b98;
  param_1[0xa45] = 0;
  param_1[0xa46] = 0;
  param_1[0x20] = 0;
  param_1[0x21] = 0;
  param_1[0x22] = 0;
  param_1[0xa39] = 0;
  param_1[0x1b] = 1;
  param_1[0xa3a] = 0;
  param_1[0xa3b] = 0;
  *(undefined1 *)((int)param_1 + 0x28f5) = 0;
  *(undefined1 *)(param_1 + 0xa3c) = 0;
  *(undefined1 *)((int)param_1 + 0x28f1) = 0;
  *(undefined1 *)((int)param_1 + 0x28f2) = 0;
  *(undefined1 *)((int)param_1 + 0x28f3) = 0;
  *(undefined1 *)(param_1 + 0xa3d) = 0;
  *(undefined1 *)((int)param_1 + 0x28f7) = 0;
  *(undefined1 *)((int)param_1 + 0x28f6) = 0;
  param_1[0xa3e] = 0;
  param_1[0xa42] = 0xff;
  param_1[0xa43] = 0xff;
  param_1[0xa44] = 0xff;
  for (local_18 = 0; local_18 < 0x20; local_18 = local_18 + 2) {
    fVar1 = (float)(local_18 / 2) / _DAT_005a40fc;
    param_1[local_18 * 6 + 0x27] = _DAT_005a3660 - fVar1;
    param_1[local_18 * 6 + 0x28] = 0x3f800000;
    param_1[(local_18 + 1) * 6 + 0x27] = _DAT_005a3660 - fVar1;
    param_1[(local_18 + 1) * 6 + 0x28] = 0;
    uVar2 = __ftol();
    uVar3 = __ftol();
    uVar4 = __ftol();
    param_1[local_18 * 6 + 0x26] = (uVar2 & 0xff) << 0x10 | (uVar3 & 0xff) << 8 | uVar4 & 0xff;
    uVar2 = __ftol();
    uVar3 = __ftol();
    uVar4 = __ftol();
    param_1[(local_18 + 1) * 6 + 0x26] = (uVar2 & 0xff) << 0x10 | (uVar3 & 0xff) << 8 | uVar4 & 0xff
    ;
  }
  for (local_18 = 0; local_18 < 0x30; local_18 = local_18 + 1) {
    FUN_00430fc0(param_1 + local_18 * 0x10 + 0x3e3);
    FUN_00430fc0(param_1 + local_18 * 0x10 + 0xe3);
    FUN_00430fc0(param_1 + local_18 * 0x10 + 0x714);
  }
  param_1[0xa34] = 0xf;
  param_1[0xa36] = 0;
  param_1[0xa3f] = 0;
  param_1[0xa40] = 0xdd;
  param_1[0xa41] = 0;
  _memset(param_1 + 0x6e3,0,0xc0);
  ExceptionList = local_10;
  return param_1;
}

