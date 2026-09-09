// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 013d3c00 | Name: FUN_013d3c00


void FUN_013d3c00(void)

{
  int iVar1;
  int iVar2;
  int unaff_EBP;
  undefined2 *puVar3;
  
  iVar1 = *(int *)(unaff_EBP + -4);
  if (*(char *)(iVar1 + 0x79c) == '\x01') {
    iVar2 = 0x3b;
    puVar3 = &DAT_013d2400;
    do {
      if (((((uint)(ushort)puVar3[1] == *(uint *)(iVar1 + 0x7a0)) &&
           (*(int *)(puVar3 + 2) == *(int *)(iVar1 + 0x7a4))) &&
          (*(int *)(puVar3 + 4) == *(int *)(iVar1 + 0x1a2))) &&
         ((*(int *)(puVar3 + 6) == *(int *)(iVar1 + 0x1a6) &&
          (*(int *)(puVar3 + 8) == *(int *)(iVar1 + 0x1aa))))) {
        if (*(char *)(*(int *)(unaff_EBP + -4) + 0x77f) == '\x01') {
          if (DAT_005d0408 == 1) {
            *(undefined4 *)(*(int *)(unaff_EBP + -4) + 0x220) = 0x40a00000;
          }
          else {
            *(undefined4 *)(*(int *)(unaff_EBP + -4) + 0x220) = 0x40000000;
          }
        }
        else if (DAT_005d0408 == 1) {
          *(undefined4 *)(*(int *)(unaff_EBP + -4) + 0x220) = 0x40c00000;
        }
        else {
          *(undefined4 *)(*(int *)(unaff_EBP + -4) + 0x220) = 0x40400000;
        }
        DAT_005bb418 = __ftol();
        return;
      }
      puVar3 = puVar3 + 0xc;
      iVar2 = iVar2 + -1;
    } while (iVar2 != 0);
  }
  FUN_004fa8e3();
  return;
}

