// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00594eee | Name: FUN_00594eee


void FUN_00594eee(uint param_1)

{
  LeaveCriticalSection
            ((LPCRITICAL_SECTION)
             ((&DAT_013b8620)[(int)param_1 >> 5] + 0xc + (param_1 & 0x1f) * 0x24));
  return;
}

