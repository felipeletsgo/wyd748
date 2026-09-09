// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053d710 | Name: FUN_0053d710


undefined4 * __thiscall
FUN_0053d710(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a2689;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0054a9e0(0);
  local_8 = 0;
  FUN_00430f10();
  *param_1 = &PTR_FUN_005a59c0;
  param_1[0xb] = 1;
  param_1[10] = param_6;
  param_1[0xc] = DAT_005bdeec;
  _memset(param_1 + 0xd,0,0x68);
  param_1[0xd] = 1;
  param_1[0x22] = 0x3a83126f;
  param_1[0x23] = 0x3a83126f;
  param_1[0x24] = 0x3a83126f;
  param_1[0x16] = param_2;
  param_1[0x17] = param_3;
  param_1[0x18] = param_4;
  param_1[0x19] = param_5;
  param_1[0x12] = param_2;
  param_1[0x13] = param_3;
  param_1[0x14] = param_4;
  param_1[0x15] = param_5;
  param_1[0xe] = param_2;
  param_1[0xf] = param_3;
  param_1[0x10] = param_4;
  param_1[0x11] = param_5;
  if (*(int *)(DAT_0067cf38 + 0x24) == 0x7531) {
    if (param_1[0xc] == 6) {
      param_1[0x20] = 0x40e00000;
    }
    else {
      param_1[0x20] = 0x40a00000;
    }
  }
  else {
    param_1[0x20] = 0x40a00000;
  }
  ExceptionList = local_10;
  return param_1;
}

