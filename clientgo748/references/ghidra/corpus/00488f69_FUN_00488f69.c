// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00488f69 | Name: FUN_00488f69


undefined4 __fastcall FUN_00488f69(int param_1)

{
  int *piVar1;
  int *piVar2;
  undefined4 uVar3;
  
  piVar1 = *(int **)(param_1 + 0x27c78);
  piVar2 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x276);
  uVar3 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x273);
  *(undefined4 *)(param_1 + 0x26ee0) = 5;
  (**(code **)(*piVar2 + 0x80))(&DAT_00a38af8,0);
  (**(code **)(**(int **)(param_1 + 0x28) + 0x40))(uVar3);
  (**(code **)(*piVar1 + 0x60))(1);
  return 1;
}

