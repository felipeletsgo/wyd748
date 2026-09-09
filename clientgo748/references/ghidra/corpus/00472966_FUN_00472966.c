// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00472966 | Name: FUN_00472966


void __fastcall FUN_00472966(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x28b44) != 0) {
    iVar1 = FUN_0040c0f0();
    if (iVar1 != 0) {
      (**(code **)(**(int **)(param_1 + 0x28) + 0x40))(0);
      (**(code **)(**(int **)(param_1 + 0x28b44) + 0x60))(0);
      *(undefined4 *)(param_1 + 0x28b40) = 0;
    }
    FUN_004481c5(1);
  }
  return;
}

