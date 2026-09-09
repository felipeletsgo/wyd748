// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d5480 | Name: FUN_004d5480


undefined4 * __thiscall FUN_004d5480(undefined4 *param_1,undefined4 param_2)

{
  int iVar1;
  float10 fVar2;
  undefined4 in_stack_00000020;
  undefined4 in_stack_00000024;
  undefined1 local_20 [12];
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0a69;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0053e1d2();
  local_8 = 0;
  CSimpleArray<>();
  CSimpleArray<>();
  *param_1 = &PTR_FUN_005a4a60;
  param_1[9] = 0x14b;
  param_1[0x18] = 0;
  param_1[0x87] = 0;
  param_1[0x17] = param_2;
  param_1[0x88] = 0xffffffff;
  param_1[0x89] = 0x3f800000;
  param_1[100] = 0;
  param_1[0x65] = 0;
  param_1[0x67] = 0;
  param_1[0x7d] = 0;
  param_1[0x66] = 0;
  param_1[0x69] = 0x3f800000;
  param_1[0x6a] = 0x3f800000;
  param_1[0x6b] = 0x3f800000;
  param_1[0x6d] = 0x3f800000;
  param_1[0x6e] = 0x3f800000;
  param_1[0x6f] = 0x3f800000;
  param_1[0x71] = 0x3f800000;
  param_1[0x72] = 0x3f800000;
  param_1[0x73] = 0x3f800000;
  param_1[0x75] = 0;
  param_1[0x7c] = in_stack_00000020;
  param_1[7] = in_stack_00000024;
  FUN_004310a0(&stack0x00000008);
  FUN_004310a0(&stack0x00000014);
  param_1[0x7e] = 0;
  FUN_00493db0(local_20,&stack0x00000008);
  fVar2 = (float10)FUN_004b3aa0();
  local_14 = (float)fVar2;
  iVar1 = __ftol();
  param_1[0x65] = iVar1 * 200;
  if (param_1[0x65] == 0) {
    param_1[0x65] = 1;
  }
  if (5000 < (uint)param_1[0x65]) {
    param_1[0x65] = 5000;
  }
  _memset(param_1 + 0x58,0,0x20);
  _memset(param_1 + 0x60,0,0x10);
  _memset(param_1 + 0x7f,0,0x20);
  param_1[0x8a] = 0;
  ExceptionList = local_10;
  return param_1;
}

