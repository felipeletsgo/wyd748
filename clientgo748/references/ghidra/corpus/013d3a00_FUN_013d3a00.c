// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 013d3a00 | Name: FUN_013d3a00


void __fastcall FUN_013d3a00(int param_1)

{
  int iVar1;
  int unaff_EBP;
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
        if (*(int *)(*(int *)(unaff_EBP + -0x5b0) + 0x287f8) != 0) {
          *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x5b0) + 0x287f8) + 0x2c) = 1;
          *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x5b0) + 0x287f8) + 0x1e8) = DAT_005d0408;
        }
        FUN_0044eca3();
        ExceptionList = *(void **)(unaff_EBP + -0xc);
        return;
      }
      puVar2 = puVar2 + 0xc;
      iVar1 = iVar1 + -1;
    } while (iVar1 != 0);
  }
  FUN_00446f91();
  return;
}

