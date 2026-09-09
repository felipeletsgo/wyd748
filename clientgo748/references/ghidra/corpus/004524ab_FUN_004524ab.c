// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004524ab | Name: FUN_004524ab


undefined4 __thiscall FUN_004524ab(int param_1,char param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  
  if ((param_2 == 'h') || (param_2 == 'H')) {
    if (*(int *)(param_1 + 0x27ca8) != 0) {
      iVar1 = FUN_0040c0f0();
      (**(code **)(**(int **)(param_1 + 0x27ca8) + 0x60))(iVar1 == 0);
      (**(code **)(**(int **)(param_1 + 0x27ca4) + 0x8c))(iVar1 == 0);
      if (DAT_005ccf98 != 0) {
        iVar1 = FUN_00429a6d(0x33);
        if (iVar1 != 0) {
          uVar3 = 0;
          uVar2 = 0;
          FUN_00429a6d(0x33);
          FUN_0042ad2b(uVar2,uVar3);
        }
      }
    }
    uVar2 = 1;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}

