// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040f78d | Name: FUN_0040f78d


int * __thiscall FUN_0040f78d(int param_1,undefined4 param_2,undefined4 param_3)

{
  int *piVar1;
  int iVar2;
  int local_14;
  int *local_8;
  
  local_8 = (int *)0x0;
  if ((((((*(int *)(param_1 + 0x1e4) == 0) && (*(int *)(param_1 + 0x400) != 8)) &&
        (*(int *)(param_1 + 0x400) != 9)) &&
       ((*(int *)(param_1 + 0x400) != 6 && (*(int *)(param_1 + 0x400) != 0xb)))) &&
      ((*(int *)(param_1 + 0x400) != 0xc &&
       ((*(int *)(param_1 + 0x400) != 0xe && (*(int *)(param_1 + 0x400) != 0x10)))))) &&
     ((*(int *)(param_1 + 0x400) != 0x12 &&
      ((*(int *)(param_1 + 0x400) != 0x14 && (*(int *)(param_1 + 0x400) != 0x16)))))) {
    for (local_14 = 0; local_14 < *(int *)(param_1 + 0x1fc); local_14 = local_14 + 1) {
      piVar1 = *(int **)(param_1 + 0x200 + local_14 * 4);
      iVar2 = FUN_0040dee8(param_2,param_3);
      if (iVar2 == 1) {
        (**(code **)(*piVar1 + 0x88))(1);
        local_8 = piVar1;
      }
      else {
        (**(code **)(*piVar1 + 0x88))(0);
      }
    }
  }
  else if (*(int *)(param_1 + 0x200) != 0) {
    local_8 = *(int **)(param_1 + 0x200);
    (**(code **)(*local_8 + 0x88))(1);
  }
  if (local_8 == (int *)0x0) {
    local_8 = (int *)0x0;
  }
  return local_8;
}

