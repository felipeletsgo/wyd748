// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00570627 | Name: FUN_00570627


undefined4 __thiscall FUN_00570627(int param_1,int *param_2)

{
  undefined4 uVar1;
  undefined1 local_e8 [33];
  byte local_c7;
  byte local_bc;
  byte local_b8;
  undefined1 local_14 [12];
  byte local_8;
  
  if (param_2 == (int *)0x0) {
    uVar1 = 0x8876086c;
  }
  else {
    *(int **)(param_1 + 8) = param_2;
    (**(code **)(*param_2 + 4))(param_2);
    (**(code **)(**(int **)(param_1 + 8) + 0x1c))(*(int **)(param_1 + 8),local_e8);
    (**(code **)(**(int **)(param_1 + 8) + 0x24))(*(int **)(param_1 + 8),local_14);
    if (((local_c7 & 2) == 0) && ((local_8 & 0x10) == 0)) {
      uVar1 = 0;
    }
    else {
      uVar1 = 1;
    }
    *(undefined4 *)(param_1 + 0x1c) = uVar1;
    if (((local_bc & 0x10) == 0) || ((local_b8 & 0x20) == 0)) {
      uVar1 = 0;
    }
    else {
      uVar1 = 1;
    }
    *(undefined4 *)(param_1 + 0x18) = uVar1;
    uVar1 = 0;
  }
  return uVar1;
}

