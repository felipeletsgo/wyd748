// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00452737 | Name: FUN_00452737


undefined4 __thiscall FUN_00452737(int param_1,char param_2)

{
  undefined4 uVar1;
  
  if ((param_2 == ']') || (param_2 == '}')) {
    if ((((*(int *)(*(int *)(param_1 + 0x40) + 0x20a20) == 0xd) &&
         (*(int *)(*(int *)(param_1 + 0x40) + 0x20a24) == 0x1f)) ||
        ((*(int *)(*(int *)(param_1 + 0x40) + 0x20a20) == 0xe &&
         (*(int *)(*(int *)(param_1 + 0x40) + 0x20a24) == 0x1e)))) ||
       ((((*(int *)(*(int *)(param_1 + 0x40) + 0x20a20) == 0xf &&
          (*(int *)(*(int *)(param_1 + 0x40) + 0x20a24) == 0x1f)) ||
         ((*(int *)(*(int *)(param_1 + 0x40) + 0x20a20) == 9 &&
          (*(int *)(*(int *)(param_1 + 0x40) + 0x20a24) == 0x1c)))) ||
        (((*(int *)(*(int *)(param_1 + 0x40) + 0x20a20) == 8 &&
          (*(int *)(*(int *)(param_1 + 0x40) + 0x20a24) == 0x1b)) ||
         ((*(int *)(*(int *)(param_1 + 0x40) + 0x20a20) == 10 &&
          (*(int *)(*(int *)(param_1 + 0x40) + 0x20a24) == 0x1b)))))))) {
      uVar1 = 1;
    }
    else {
      *(uint *)(param_1 + 0x200a0) = (uint)(*(int *)(param_1 + 0x200a0) == 0);
      (**(code **)(**(int **)(param_1 + 0x2791c) + 0x60))(*(undefined4 *)(param_1 + 0x200a0));
      if (*(int *)(param_1 + 0x27c98) != 0) {
        (**(code **)(**(int **)(param_1 + 0x27c98) + 0x8c))(*(undefined4 *)(param_1 + 0x200a0));
      }
      uVar1 = 1;
    }
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

