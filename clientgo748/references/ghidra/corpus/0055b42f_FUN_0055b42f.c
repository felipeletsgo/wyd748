// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055b42f | Name: FUN_0055b42f


void FUN_0055b42f(void)

{
  int iVar1;
  size_t sVar2;
  char local_cc [64];
  undefined1 local_8c [128];
  int local_c;
  int local_8;
  
  local_8 = FUN_0058f716(s_UI_UIString_txt_005c4ae0,&DAT_005c4adc);
  if (local_8 != 0) {
    _memset(local_cc,0,0x40);
    local_c = 0;
    while (iVar1 = FUN_00590a06(local_8c,0x80,local_8), iVar1 != 0) {
      FUN_005909d2(local_8c,s__d__s_005c4af0,&local_c,local_cc);
      FUN_00431159(s__d__s_005c4af8,local_c,local_cc);
      sVar2 = _strlen(local_cc);
      if (0x1b7 < local_c) break;
      FUN_0058ee20(&DAT_00e38540 + local_c * 0x40,local_cc,iVar1,sVar2);
    }
    FUN_0058f62c(local_8);
  }
  return;
}

