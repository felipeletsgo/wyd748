// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00447594 | Name: FUN_00447594


void __fastcall FUN_00447594(int param_1)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  
  piVar1 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x273);
  piVar2 = *(int **)(param_1 + 0x27c78);
  (**(code **)(**(int **)(param_1 + 0x28) + 0x40))(0);
  (**(code **)(*piVar2 + 0x60))(0);
  (**(code **)(*piVar1 + 0x90))(&DAT_005d0438);
  if (-1 < *(int *)(param_1 + 0x26ee4)) {
    piVar2 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x726);
    iVar3 = (**(code **)(*piVar2 + 0xb8))
                      (*(int *)(param_1 + 0x26ee4) % 9,*(int *)(param_1 + 0x26ee4) / 9);
    *(undefined4 *)(iVar3 + 0x94) = 0xffffffff;
    *(undefined4 *)(param_1 + 0x26ee4) = 0xffffffff;
  }
  if (DAT_013b722c == 1) {
    (**(code **)(**(int **)(param_1 + 0x28) + 0x40))(*(undefined4 *)(param_1 + 0x27b2c));
  }
  return;
}

