// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057058e | Name: FUN_0057058e


undefined4 __fastcall FUN_0057058e(undefined4 *param_1)

{
  if ((LPCVOID)param_1[2] != (LPCVOID)0x0) {
    UnmapViewOfFile((LPCVOID)param_1[2]);
    param_1[2] = 0;
    param_1[3] = 0;
  }
  if ((HANDLE)param_1[1] != (HANDLE)0xffffffff) {
    CloseHandle((HANDLE)param_1[1]);
    param_1[1] = 0xffffffff;
  }
  if ((HANDLE)*param_1 != (HANDLE)0xffffffff) {
    CloseHandle((HANDLE)*param_1);
    *param_1 = 0xffffffff;
  }
  return 0;
}

