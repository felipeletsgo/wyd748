// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00524a51 | Name: FUN_00524a51


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00524a51(int param_1)

{
  int iVar1;
  int iVar2;
  int local_44;
  int local_40;
  undefined1 local_34 [48];
  
  if (*(int *)(param_1 + 0x214) == 0) {
    _memset(local_34,0,0x30);
    local_44 = *(int *)(param_1 + 0x414) + 1;
    if ((float)_DAT_005a45c8 < *(float *)(param_1 + 0x230)) {
      local_44 = *(int *)(param_1 + 0x414) + 2;
    }
    iVar1 = __ftol();
    iVar2 = __ftol();
    for (local_40 = local_44; local_40 < 0x30; local_40 = local_40 + 1) {
      *(float *)(param_1 + 0x290 + local_40 * 8) = (float)iVar1;
      *(float *)(param_1 + 0x294 + local_40 * 8) = (float)iVar2;
    }
  }
  return;
}

