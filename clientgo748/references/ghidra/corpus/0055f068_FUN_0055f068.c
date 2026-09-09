// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055f068 | Name: FUN_0055f068


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0055f068(int param_1)

{
  int iVar1;
  size_t sVar2;
  int local_208;
  undefined1 local_204 [256];
  char local_104 [256];
  
  if (DAT_013b736c == 0) {
    DAT_013b736c = 1;
    DAT_013b7374 = 0;
    ShowWindow(DAT_013b7368,5);
    if (DAT_013b7364 != 0) {
      iVar1 = FUN_0059e150(DAT_013b7364);
      if (iVar1 == 0) {
        _memset(local_204,0,0x100);
        _memset(local_104,0,0x100);
        FUN_0058f078(local_104,DAT_013b71e8 + 0x1af54);
        sVar2 = _strlen(local_104);
        for (local_208 = 0; local_208 < (int)sVar2; local_208 = local_208 + 1) {
          local_104[local_208] = local_104[local_208] - (char)local_208;
        }
        FUN_0058f078(local_204,s__s_id__s_server__d_005c5020,&DAT_00a3ebf8,local_104,
                     *(undefined4 *)(DAT_013b71e8 + 0x1af4c));
        FUN_0059e010(DAT_013b7364,local_204);
        FUN_0059e100(DAT_013b7364,800,0x230);
        SetFocus(*(HWND *)(param_1 + 4));
        _DAT_013b7370 = 1;
      }
      else {
        DAT_013b736c = 0;
      }
    }
  }
  else {
    SetFocus(*(HWND *)(param_1 + 4));
    DAT_013b736c = 0;
    DAT_013b7374 = 0;
    ShowWindow(DAT_013b7368,0);
    if (DAT_013b7364 != 0) {
      FUN_0059dfe0(DAT_013b7364);
    }
    _DAT_013b7370 = 0;
  }
  return;
}

