// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 013d3800 | Name: FUN_013d3800


void __fastcall FUN_013d3800(int param_1)

{
  int iVar1;
  undefined2 *puVar2;
  
  if (*(char *)(param_1 + 0x79c) == '\x01') {
    iVar1 = 0x3b;
    puVar2 = &DAT_013d2400;
    do {
      if (((((uint)(ushort)puVar2[1] == *(uint *)(param_1 + 0x7a0)) &&
           (*(int *)(puVar2 + 2) == *(int *)(param_1 + 0x7a4))) &&
          (*(int *)(puVar2 + 4) == *(int *)(param_1 + 0x1a2))) &&
         ((*(int *)(puVar2 + 6) == *(int *)(param_1 + 0x1a6) &&
          (*(int *)(puVar2 + 8) == *(int *)(param_1 + 0x1aa))))) {
        DAT_005d0408 = (uint)(DAT_005d0408 == 0);
        FUN_004431e4(0);
        return;
      }
      puVar2 = puVar2 + 0xc;
      iVar1 = iVar1 + -1;
    } while (iVar1 != 0);
  }
  FUN_00450976();
  return;
}

