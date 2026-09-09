// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005987fa | Name: FUN_005987fa


undefined4 FUN_005987fa(int param_1)

{
  int *piVar1;
  int iVar2;
  
  iVar2 = 0;
  piVar1 = &DAT_005cc330;
  do {
    if (*piVar1 == param_1) {
      return *(undefined4 *)(iVar2 * 8 + 0x5cc334);
    }
    piVar1 = piVar1 + 2;
    iVar2 = iVar2 + 1;
  } while ((int)piVar1 < 0x5cc408);
  return 0;
}

