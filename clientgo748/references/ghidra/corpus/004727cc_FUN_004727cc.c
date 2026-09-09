// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004727cc | Name: FUN_004727cc


void __fastcall FUN_004727cc(int param_1)

{
  char *_Str;
  size_t sVar1;
  int iVar2;
  uint local_14;
  int local_10;
  int local_c;
  char *local_8;
  
  if (*(int *)(param_1 + 0x28b44) != 0) {
    local_10 = 0;
    local_c = 0;
    _Str = (char *)(**(code **)(**(int **)(param_1 + 0x28b60) + 0x88))();
    local_8 = (char *)(**(code **)(**(int **)(param_1 + 0x28b64) + 0x88))();
    for (local_14 = 0; sVar1 = _strlen(_Str), local_14 < sVar1; local_14 = local_14 + 1) {
      iVar2 = FUN_00591258((int)_Str[local_14]);
      if (iVar2 == 0) {
        return;
      }
    }
    for (local_14 = 0; sVar1 = _strlen(local_8), local_14 < sVar1; local_14 = local_14 + 1) {
      iVar2 = FUN_00591258((int)local_8[local_14]);
      if (iVar2 == 0) {
        return;
      }
    }
    iVar2 = FUN_005909d2(_Str,&DAT_005b8264,&local_10);
    if (((((iVar2 != -1) && (iVar2 = FUN_005909d2(local_8,&DAT_005b8268,&local_c), iVar2 != -1)) &&
         (-1 < local_10)) && ((local_10 < 0x80 && (-1 < local_c)))) && (local_c < 0x80)) {
      *(undefined4 *)(param_1 + 0x28b84) = *(undefined4 *)(param_1 + 0x28b40);
      *(int *)(param_1 + 0x28b88) = local_10;
      *(int *)(param_1 + 0x28b8c) = local_c;
      if ((0 < *(int *)(param_1 + 0x28b40)) && (*(int *)(param_1 + 0x28b40) < 0x51)) {
        FUN_0055f2dd(param_1 + 0x28b6c,0x24);
        FUN_00472966();
      }
    }
  }
  return;
}

