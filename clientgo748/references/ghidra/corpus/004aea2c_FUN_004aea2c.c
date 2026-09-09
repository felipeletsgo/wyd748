// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004aea2c | Name: FUN_004aea2c


bool __fastcall FUN_004aea2c(int param_1)

{
  DWORD local_10;
  DWORD local_c;
  HIMC local_8;
  
  local_c = 0;
  local_8 = ImmGetContext(*(HWND *)(param_1 + 0x444));
  if (local_8 != (HIMC)0x0) {
    ImmGetConversionStatus(local_8,&local_c,&local_10);
    ImmReleaseContext(*(HWND *)(param_1 + 0x444),local_8);
  }
  return (local_c & 1) == 1;
}

