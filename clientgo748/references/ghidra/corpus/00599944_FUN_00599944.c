// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00599944 | Name: FUN_00599944


int FUN_00599944(int param_1)

{
  int iVar1;
  bool bVar2;
  
  if (param_1 == -2) {
    DAT_013b819c = 1;
                    /* WARNING: Could not recover jumptable at 0x0059995e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    iVar1 = GetOEMCP();
    return iVar1;
  }
  if (param_1 == -3) {
    DAT_013b819c = 1;
                    /* WARNING: Could not recover jumptable at 0x00599973. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    iVar1 = GetACP();
    return iVar1;
  }
  bVar2 = param_1 == -4;
  if (bVar2) {
    param_1 = DAT_013b82f4;
  }
  DAT_013b819c = (uint)bVar2;
  return param_1;
}

