// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004baee0 | Name: FUN_004baee0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 FUN_004baee0(void)

{
  DWORD DVar1;
  
  DVar1 = timeGetTime();
  return (float10)DVar1 * (float10)_DAT_005a46a0;
}

