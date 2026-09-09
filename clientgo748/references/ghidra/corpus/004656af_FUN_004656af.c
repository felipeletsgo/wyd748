// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004656af | Name: FUN_004656af


void __fastcall FUN_004656af(int param_1)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  
  piVar3 = *(int **)(param_1 + 0x27c78);
  piVar1 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x276);
  piVar2 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x273);
  if ((piVar1 != (int *)0x0) && (piVar2 != (int *)0x0)) {
    *(undefined4 *)(param_1 + 0x26ee0) = 3;
    (**(code **)(*piVar1 + 0x80))(&DAT_00a387f8,0);
    (**(code **)(**(int **)(param_1 + 0x28) + 0x40))(piVar2);
    (**(code **)(*piVar3 + 0x60))(1);
    piVar3 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x23e);
    if (piVar3 != (int *)0x0) {
      (**(code **)(*piVar3 + 0x60))(1);
    }
    if (DAT_005b892c != 2) {
      (**(code **)(*piVar2 + 0x70))(0x41c80000,0x42240000);
    }
    piVar2[0x414] = 0x14;
  }
  return;
}

