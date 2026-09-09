// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055474f | Name: FUN_0055474f


void FUN_0055474f(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_84 [128];
  
  _memset(&DAT_00d44950,0,52000);
  iVar1 = FUN_005547e4(s_itemlist_csv_005c4340,0,1);
  if (iVar1 == 0) {
    MessageBoxA((HWND)0x0,s_There_is_no_file_005c4360,s_itemlist_csv_005c4350,0);
  }
  else {
    FUN_0058ee20(local_84,s_extraitem_csv_005c4374);
    uVar2 = FUN_005547e4(local_84,1);
    FUN_0054b140(uVar2);
  }
  return;
}

