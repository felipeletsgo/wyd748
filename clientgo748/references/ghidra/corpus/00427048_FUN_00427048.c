// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00427048 | Name: FUN_00427048


undefined4 FUN_00427048(DEVMODEA *param_1)

{
  HDC hdc;
  DWORD DVar1;
  DWORD DVar2;
  DWORD DVar3;
  DWORD local_14;
  int local_10;
  
  hdc = CreateDCA(s_DISPLAY_005b27ec,(LPCSTR)0x0,(LPCSTR)0x0,(DEVMODEA *)0x0);
  DVar1 = GetDeviceCaps(hdc,8);
  DVar2 = GetDeviceCaps(hdc,10);
  DVar3 = GetDeviceCaps(hdc,0xc);
  GetDeviceCaps(hdc,0x74);
  DeleteDC(hdc);
  local_14 = 0;
  local_10 = EnumDisplaySettingsA((LPCSTR)0x0,0,param_1);
  while( true ) {
    if (local_10 == 0) {
      return 0;
    }
    if (((param_1->dmPelsWidth == DVar1) && (param_1->dmPelsHeight == DVar2)) &&
       (param_1->dmBitsPerPel == DVar3)) break;
    local_14 = local_14 + 1;
    local_10 = EnumDisplaySettingsA((LPCSTR)0x0,local_14,param_1);
  }
  return 1;
}

