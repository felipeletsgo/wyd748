// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004538c6 | Name: FUN_004538c6


undefined4 __thiscall FUN_004538c6(int param_1,char param_2)

{
  int iVar1;
  undefined4 uVar2;
  int *piVar3;
  
  if ((param_2 == 'c') || (param_2 == 'C')) {
    iVar1 = FUN_0040c0f0();
    if (iVar1 == 0) {
      iVar1 = FUN_0040c0f0();
      if (iVar1 == 1) {
        uVar2 = 1;
      }
      else {
        iVar1 = FUN_0040c0f0();
        if (iVar1 == 1) {
          uVar2 = 1;
        }
        else {
          FUN_00447c73();
          iVar1 = *(int *)(param_1 + 0x27c7c);
          piVar3 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x125);
          if (piVar3 != (int *)0x0) {
            (**(code **)(*piVar3 + 0x8c))(*(undefined4 *)(iVar1 + 0x28));
          }
          uVar2 = 1;
        }
      }
    }
    else {
      uVar2 = 1;
    }
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}

