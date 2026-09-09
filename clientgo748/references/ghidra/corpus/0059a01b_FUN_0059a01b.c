// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059a01b | Name: FUN_0059a01b


void FUN_0059a01b(void)

{
  if ((DAT_013b8030 == 1) || ((DAT_013b8030 == 0 && (DAT_005c9c04 == 1)))) {
    FUN_0059a054(0xfc);
    if (DAT_013b82a8 != (code *)0x0) {
      (*DAT_013b82a8)();
    }
    FUN_0059a054(0xff);
  }
  return;
}

