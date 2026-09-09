// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055cf8f | Name: FUN_0055cf8f


void FUN_0055cf8f(void)

{
  int iVar1;
  undefined4 extraout_var;
  undefined4 uVar2;
  int local_40;
  undefined1 local_34 [44];
  undefined *local_8;
  
  local_8 = &DAT_005c4e68;
  _memset(&DAT_013b714c,0,99);
  uVar2 = extraout_var;
  _memset(&DAT_013b71b0,0,0x2c);
  _memset(local_34,0,0x2c);
  iVar1 = FUN_0058f716(s_sn_bin_005c4f18,&DAT_005c4f14,uVar2);
  if (iVar1 != 0) {
    FUN_00590d68(&DAT_013b714c,1,99,iVar1);
    for (local_40 = 0; local_40 < 99; local_40 = local_40 + 1) {
      DAT_013b714c = (DAT_013b714c - local_8[local_40]) - (char)local_40;
    }
    FUN_00590d68(&DAT_013b71b0,1,0x2c,iVar1);
    FUN_0058f62c(iVar1);
  }
  return;
}

