// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005999c1 | Name: FUN_005999c1


void FUN_005999c1(void)

{
  int iVar1;
  undefined4 *puVar2;
  
  puVar2 = &DAT_013b84e0;
  for (iVar1 = 0x40; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  }
  *(undefined1 *)puVar2 = 0;
  DAT_013b83b4 = 0;
  DAT_013b83cc = 0;
  DAT_013b85e4 = 0;
  DAT_013b83c0 = 0;
  DAT_013b83c4 = 0;
  DAT_013b83c8 = 0;
  return;
}

