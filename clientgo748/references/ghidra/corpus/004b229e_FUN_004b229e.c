// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b229e | Name: FUN_004b229e


void FUN_004b229e(void)

{
  int iVar1;
  undefined4 extraout_var;
  uint local_10;
  
  _memset(&DAT_006703b0,0,0x2000);
  iVar1 = FUN_0058f716(s_curse_bin_005b9a80,&DAT_005b9a7c,extraout_var);
  if (iVar1 != 0) {
    FUN_00590d68(&DAT_006703b0,0x2000,1,iVar1);
    FUN_0058f62c(iVar1);
    for (local_10 = 0; local_10 < 0x2000; local_10 = local_10 + 1) {
      (&DAT_006703b0)[local_10] = (&DAT_006703b0)[local_10] ^ 0xff;
    }
  }
  return;
}

