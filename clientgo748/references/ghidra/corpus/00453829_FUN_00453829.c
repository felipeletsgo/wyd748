// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00453829 | Name: FUN_00453829


undefined4 __thiscall FUN_00453829(int param_1,char param_2)

{
  int iVar1;
  undefined4 uVar2;
  int *piVar3;
  
  if ((((param_2 == 'i') || (param_2 == 'I')) || (param_2 == 'g')) || (param_2 == 'G')) {
    iVar1 = FUN_0040c0f0();
    if (iVar1 == 0) {
      FUN_00447691();
      iVar1 = *(int *)(param_1 + 0x27b34);
      piVar3 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x126);
      if (piVar3 != (int *)0x0) {
        (**(code **)(*piVar3 + 0x8c))(*(undefined4 *)(iVar1 + 0x28));
      }
      uVar2 = 1;
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

