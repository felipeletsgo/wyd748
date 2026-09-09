// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044c9d6 | Name: FUN_0044c9d6


void __fastcall FUN_0044c9d6(int param_1)

{
  int *piVar1;
  undefined4 uVar2;
  int *piVar3;
  
  piVar1 = *(int **)(param_1 + 0x27c78);
  uVar2 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x273);
  piVar3 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x276);
  *(undefined4 *)(param_1 + 0x26ee0) = 10;
  (**(code **)(*piVar3 + 0x80))(&DAT_00a3fcf8,0);
  (**(code **)(**(int **)(param_1 + 0x28) + 0x40))(uVar2);
  (**(code **)(*piVar1 + 0x60))(1);
  return;
}

