// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004489c5 | Name: FUN_004489c5


void __fastcall FUN_004489c5(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *local_3c;
  int local_34;
  int local_30 [8];
  int local_10;
  int local_c;
  int *local_8;
  
  local_8 = *(int **)(param_1 + 0x2873c);
  for (local_c = 0; local_c < 7; local_c = local_c + 1) {
    for (local_34 = 0; local_34 < 9; local_34 = local_34 + 1) {
      iVar1 = (**(code **)(*local_8 + 0xb4))(local_34,local_c);
      if ((iVar1 != 0) && (*(int *)(iVar1 + 0x94) == -0x10000)) {
        *(undefined4 *)(iVar1 + 0x94) = 0xffffffff;
      }
    }
  }
  _memset((void *)(DAT_013b71e8 + 0xd50),0,0x54);
  *(undefined2 *)(DAT_013b71e8 + 0xd56) = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
  *(undefined2 *)(DAT_013b71e8 + 0xd54) = 0x3b5;
  for (local_10 = 0; local_10 < 8; local_10 = local_10 + 1) {
    *(undefined1 *)(DAT_013b71e8 + local_10 + 0xd9c) = 0xff;
  }
  _memset(local_30,0,0x20);
  for (local_10 = 0; local_10 < 8; local_10 = local_10 + 1) {
    local_3c = (undefined4 *)0x0;
    iVar1 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(local_10 + 0x17e0);
    local_30[local_10] = iVar1;
    if (local_30[local_10] != 0) {
      local_3c = (undefined4 *)(**(code **)(*(int *)local_30[local_10] + 0xa4))(0,0);
    }
    if ((*(int *)(DAT_005ccec0 + 0x1e8) != 0) &&
       (*(undefined4 **)(DAT_005ccec0 + 0x1e8) == local_3c)) {
      *(undefined4 *)(DAT_005ccec0 + 0x1e8) = 0;
    }
    if ((local_3c != (undefined4 *)0x0) && (local_3c != (undefined4 *)0x0)) {
      (**(code **)*local_3c)(1);
    }
  }
  for (local_10 = 0; local_10 < 4; local_10 = local_10 + 1) {
    puVar2 = (undefined4 *)(**(code **)(**(int **)(param_1 + 0x27c60 + local_10 * 4) + 0xa4))(0,0);
    if ((puVar2 != (undefined4 *)0x0) && (puVar2 != (undefined4 *)0x0)) {
      (**(code **)*puVar2)(1);
    }
  }
  *(undefined4 *)(*(int *)(param_1 + 0x2873c) + 0x400) = 0;
  FUN_00447f6f(1);
  return;
}

