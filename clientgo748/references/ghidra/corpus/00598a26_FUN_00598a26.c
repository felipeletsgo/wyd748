// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00598a26 | Name: FUN_00598a26


void FUN_00598a26(void)

{
  if (DAT_013b8170 == 0) {
    FUN_00596aad(0xb);
    if (DAT_013b8170 == 0) {
      FUN_00598a54();
      DAT_013b8170 = DAT_013b8170 + 1;
    }
    FUN_00596b0e(0xb);
  }
  return;
}

