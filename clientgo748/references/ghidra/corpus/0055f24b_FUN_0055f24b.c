// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055f24b | Name: FUN_0055f24b


undefined4 FUN_0055f24b(DWORD param_1,DWORD param_2,DWORD param_3)

{
  DWORD local_a8;
  int local_a4;
  DEVMODEA local_a0;
  
  local_a8 = 0;
  local_a4 = EnumDisplaySettingsA((LPCSTR)0x0,0,&local_a0);
  while( true ) {
    if (local_a4 == 0) {
      return 0;
    }
    if (((local_a0.dmPelsWidth == param_1) && (local_a0.dmPelsHeight == param_2)) &&
       (local_a0.dmBitsPerPel == param_3)) break;
    local_a8 = local_a8 + 1;
    local_a4 = EnumDisplaySettingsA((LPCSTR)0x0,local_a8,&local_a0);
  }
  return 1;
}

