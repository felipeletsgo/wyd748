// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052ff60 | Name: FUN_0052ff60


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_0052ff60(undefined4 *param_1,int param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a24b9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0053e1d2();
  local_8 = 0;
  CSimpleArray<>();
  *param_1 = &PTR_FUN_005a56f4;
  iVar1 = _rand();
  iVar1 = iVar1 % 10;
  param_1[9] = param_2;
  param_1[100] = iVar1 % 3;
  param_1[0x6c] = 0;
  param_1[0x65] = param_3;
  param_1[0x18] = 0;
  param_1[0x17] = 0x45;
  if (param_2 == 7) {
    param_1[0x17] = 0x18;
    param_1[100] = 3;
  }
  param_1[7] = param_4;
  _memset(param_1 + 0x58,0,0x20);
  _memset(param_1 + 0x60,0,0x10);
  if (param_1[0x65] == 0) {
    iVar2 = _rand();
    *(short *)((int)param_1 + 0x162) = (short)(iVar2 % 3);
  }
  else if (param_1[0x65] == 1) {
    uVar3 = _rand();
    uVar3 = uVar3 & 0x80000001;
    if ((int)uVar3 < 0) {
      uVar3 = (uVar3 - 1 | 0xfffffffe) + 1;
    }
    *(short *)((int)param_1 + 0x162) = (short)uVar3 + 3;
  }
  else if (param_1[0x65] == 2) {
    *(undefined2 *)((int)param_1 + 0x162) = 2;
  }
  else if (param_1[0x65] == 3) {
    *(undefined2 *)((int)param_1 + 0x162) = 5;
  }
  else {
    *(undefined2 *)((int)param_1 + 0x162) = 0;
  }
  param_1[0x66] = (float)iVar1 * _DAT_005a4214 + _DAT_005a3660;
  param_1[0x68] = (float)iVar1 * _DAT_005a34a0 + _DAT_005a40f0;
  param_1[0x67] = (float)iVar1 * _DAT_005a439c + _DAT_005a4158;
  ExceptionList = local_10;
  return param_1;
}

