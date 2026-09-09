// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ce5e0 | Name: FUN_004ce5e0


undefined4 * __thiscall
FUN_004ce5e0(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  undefined4 uVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a08a9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  *param_1 = &PTR_FUN_005a48a4;
  param_1[0x21] = 3000;
  param_1[0x27] = 0;
  param_1[0x26] = 0;
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
  param_1[0x28] = param_3;
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x20] = uVar1;
  if (param_1[0x28] == 2) {
    param_1[0x20] = param_1[0x20] + (int)param_1 * 300;
  }
  FUN_004ce77d(param_4);
  ExceptionList = local_10;
  return param_1;
}

