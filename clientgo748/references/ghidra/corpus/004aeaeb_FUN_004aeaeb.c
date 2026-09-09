// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004aeaeb | Name: FUN_004aeaeb


void __fastcall FUN_004aeaeb(int param_1)

{
  DWORD local_10 [2];
  HIMC local_8;
  
  local_8 = ImmGetContext(*(HWND *)(param_1 + 0x444));
  if (local_8 != (HIMC)0x0) {
    ImmGetConversionStatus(local_8,local_10 + 1,local_10);
    local_10[1] = 0;
    ImmSetConversionStatus(local_8,0,local_10[0]);
    ImmReleaseContext(*(HWND *)(param_1 + 0x444),local_8);
  }
  return;
}

