// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00530fa0 | Name: FUN_00530fa0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall FUN_00530fa0(undefined4 *param_1,undefined4 param_2,int param_3)

{
  int iVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a24f9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0053e1d2();
  local_8 = 0;
  CSimpleArray<>();
  *param_1 = &PTR_FUN_005a5774;
  param_1[9] = param_2;
  param_1[0x58] = param_3;
  iVar1 = _rand();
  param_1[0x65] = iVar1 % 3;
  iVar1 = _rand();
  param_1[0xe] = (float)(iVar1 % 10) * _DAT_005a4214 + _DAT_005a3660;
  param_1[0x6b] = 0;
  param_1[0x18] = 0;
  param_1[0x17] = 0x46;
  _memset(param_1 + 0x59,0,0x20);
  _memset(param_1 + 0x61,0,0x10);
  if (param_3 == 0) {
    iVar1 = _rand();
    *(short *)(param_1 + 0x59) = (short)(iVar1 % 3);
    iVar1 = _rand();
    *(char *)(param_1 + 0x61) = (char)(iVar1 % 3) + '\a';
  }
  else {
    *(short *)(param_1 + 0x59) = (short)(param_3 % 3);
    *(char *)(param_1 + 0x61) = (char)param_3 + '\x01';
  }
  iVar1 = _rand();
  param_1[0x66] = (float)(iVar1 % 10) * _DAT_005a4214 + _DAT_005a4160;
  iVar1 = _rand();
  param_1[0x67] = (float)(iVar1 % 7) * _DAT_005a34a0 + _DAT_005a40f0;
  ExceptionList = local_10;
  return param_1;
}

