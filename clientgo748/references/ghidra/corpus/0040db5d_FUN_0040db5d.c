// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040db5d | Name: FUN_0040db5d


void __fastcall FUN_0040db5d(undefined4 *param_1)

{
  int iVar1;
  size_t sVar2;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  puStack_c = &LAB_0059ee4a;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a3f88;
  local_8 = 1;
  if (((param_1[0x19c] != 0) && (*(short *)param_1[0x19c] < 0x1965)) &&
     (-1 < *(short *)param_1[0x19c])) {
    iVar1 = *(int *)(DAT_0067cf38 + 0x28);
    if (*(short *)(&DAT_00d44990 + *(short *)param_1[0x19c] * 0x8c) < 0) {
      if ((iVar1 != 0) && (-1 < (int)param_1[0x6b])) {
        FUN_0040c1b0(iVar1 + 0x3c,param_1 + 0x19,param_1[0x6b]);
      }
    }
    else if (((iVar1 != 0) && (-1 < (int)param_1[0x127])) &&
            (sVar2 = _strlen((char *)((int)param_1 + 0x39a)), sVar2 != 0)) {
      FUN_0040c1b0(iVar1 + 0x3c,param_1 + 0xd5,param_1[0x127]);
    }
    if ((iVar1 != 0) && (-1 < (int)param_1[0xcc])) {
      FUN_0040c1b0(iVar1 + 0x3c,param_1 + 0x7a,param_1[0xcc]);
    }
    if (param_1[0x19c] != 0) {
      FUN_0058ea3a(param_1[0x19c]);
      param_1[0x19c] = 0;
    }
  }
  local_8 = local_8 & 0xffffff00;
  FUN_00421a4b();
  local_8 = 0xffffffff;
  FUN_00401b53();
  ExceptionList = local_10;
  return;
}

