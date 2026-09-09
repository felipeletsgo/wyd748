// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055ee59 | Name: FUN_0055ee59


void __thiscall FUN_0055ee59(int param_1,undefined4 param_2)

{
  int iVar1;
  size_t sVar2;
  undefined4 uVar3;
  int local_20c;
  undefined1 local_208 [256];
  char local_108 [256];
  undefined4 local_8;
  
  if (DAT_013b736c == 0) {
    local_8 = param_2;
    DAT_013b736c = 1;
    DAT_013b7374 = 0;
    ShowWindow(DAT_013b7368,5);
    if (DAT_013b7364 != 0) {
      iVar1 = FUN_0059e150(DAT_013b7364);
      if (iVar1 == 0) {
        _memset(local_208,0,0x100);
        _memset(local_108,0,0x100);
        FUN_0058f078(local_108,DAT_013b71e8 + 0x1af54);
        sVar2 = _strlen(local_108);
        for (local_20c = 0; local_20c < (int)sVar2; local_20c = local_20c + 1) {
          local_108[local_20c] = local_108[local_20c] - (char)local_20c;
        }
        FUN_0058f078(local_208,s__s_id__s_server__d_Empty__d_005c5004,&DAT_00a3c7f8,local_108,
                     *(undefined4 *)(DAT_013b71e8 + 0x1af4c),local_8);
        FUN_0059e010(DAT_013b7364,local_208);
        FUN_0059e100(DAT_013b7364,800,0x230);
        iVar1 = DAT_0067cf38;
        if (DAT_0067cf38 != 0) {
          uVar3 = (**(code **)(*DAT_0092e654 + 8))();
          *(undefined4 *)(iVar1 + 0x28e80) = uVar3;
        }
        SetFocus(*(HWND *)(param_1 + 4));
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
  }
  return;
}

