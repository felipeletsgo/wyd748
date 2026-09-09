// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00556cc7 | Name: FUN_00556cc7


void FUN_00556cc7(void)

{
  int iVar1;
  int local_10;
  
  for (local_10 = 0; local_10 < 0x300; local_10 = local_10 + 1) {
    *(byte *)((int)&DAT_00e22c00 + local_10) = *(byte *)((int)&DAT_00e22c00 + local_10) ^ 0xff;
  }
  iVar1 = FUN_0058f716(s_InitItem_bin_005c457c,&DAT_005c4578);
  if (iVar1 == 0) {
    MessageBoxA((HWND)0x0,s_Can_t_write_inititem_bin_005c4594,s_ERROR_005c458c,0x1000);
  }
  else {
    FUN_005904b6(&DAT_00e22c00,0x300,1,iVar1);
    FUN_0058f62c(iVar1);
    for (local_10 = 0; local_10 < 0x300; local_10 = local_10 + 1) {
      *(byte *)((int)&DAT_00e22c00 + local_10) = *(byte *)((int)&DAT_00e22c00 + local_10) ^ 0xff;
    }
  }
  return;
}

