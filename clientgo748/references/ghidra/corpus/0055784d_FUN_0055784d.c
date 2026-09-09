// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055784d | Name: FUN_0055784d


undefined4 FUN_0055784d(void)

{
  int iVar1;
  char *pcVar2;
  char *pcVar3;
  int local_58;
  int local_54;
  int local_50;
  char local_4c [68];
  int local_8;
  
  local_8 = FUN_0058f716(s___serverlist_bin_005c4778,&DAT_005c4774);
  if (local_8 != 0) {
    pcVar2 = (char *)&DAT_005c478c;
    pcVar3 = local_4c;
    for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
      *(undefined4 *)pcVar3 = *(undefined4 *)pcVar2;
      pcVar2 = pcVar2 + 4;
      pcVar3 = pcVar3 + 4;
    }
    *pcVar3 = *pcVar2;
    FUN_00590d68(&DAT_009325f8,0x6e,0x40,local_8);
    FUN_0058f62c(local_8);
    for (local_50 = 0; local_50 < 10; local_50 = local_50 + 1) {
      for (local_54 = 0; local_54 < 0xb; local_54 = local_54 + 1) {
        for (local_58 = 0; local_58 < 0x40; local_58 = local_58 + 1) {
          (&DAT_009325f8)[local_58 + local_50 * 0x2c0 + local_54 * 0x40] =
               (&DAT_009325f8)[local_58 + local_50 * 0x2c0 + local_54 * 0x40] -
               local_4c[0x3f - local_58];
        }
      }
    }
  }
  return 1;
}

