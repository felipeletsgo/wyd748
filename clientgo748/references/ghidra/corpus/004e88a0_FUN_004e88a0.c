// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e88a0 | Name: FUN_004e88a0


undefined4 * __thiscall FUN_004e88a0(undefined4 *param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 uVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a1289;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  *param_1 = &PTR_FUN_005a50b8;
  param_1[0x27] = 0;
  param_1[0x26] = 1000;
  param_1[0x28] = 0;
  param_1[0x29] = 0;
  param_1[0x24] = 0x3f800000;
  param_1[7] = param_2;
  if (param_1[7] == 0) {
    uVar1 = FUN_00430f50(0,0,0);
    FUN_004310a0(uVar1);
  }
  else {
    uVar1 = FUN_00430f50(*(undefined4 *)(param_1[7] + 0x28),*(undefined4 *)(param_1[7] + 0x30),
                         *(undefined4 *)(param_1[7] + 0x2c));
    FUN_004310a0(uVar1);
  }
  param_1[0x2a] = param_3;
  param_1[0x2b] = 0;
  FUN_004e89e9(0xffffffff);
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x25] = uVar1;
  ExceptionList = local_10;
  return param_1;
}

