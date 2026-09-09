// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00593dba | Name: FUN_00593dba


void FUN_00593dba(LPVOID param_1)

{
  if (DAT_005c9d60 != 0xffffffff) {
    if ((param_1 != (LPVOID)0x0) || (param_1 = TlsGetValue(DAT_005c9d60), param_1 != (LPVOID)0x0)) {
      if (*(int *)((int)param_1 + 0x24) != 0) {
        FUN_0058ffd8(*(int *)((int)param_1 + 0x24));
      }
      if (*(int *)((int)param_1 + 0x28) != 0) {
        FUN_0058ffd8(*(int *)((int)param_1 + 0x28));
      }
      if (*(int *)((int)param_1 + 0x30) != 0) {
        FUN_0058ffd8(*(int *)((int)param_1 + 0x30));
      }
      if (*(int *)((int)param_1 + 0x38) != 0) {
        FUN_0058ffd8(*(int *)((int)param_1 + 0x38));
      }
      if (*(int *)((int)param_1 + 0x40) != 0) {
        FUN_0058ffd8(*(int *)((int)param_1 + 0x40));
      }
      if (*(int *)((int)param_1 + 0x44) != 0) {
        FUN_0058ffd8(*(int *)((int)param_1 + 0x44));
      }
      if (*(undefined **)((int)param_1 + 0x50) != &DAT_005cc030) {
        FUN_0058ffd8(*(undefined **)((int)param_1 + 0x50));
      }
      FUN_0058ffd8(param_1);
    }
    TlsSetValue(DAT_005c9d60,(LPVOID)0x0);
    return;
  }
  return;
}

