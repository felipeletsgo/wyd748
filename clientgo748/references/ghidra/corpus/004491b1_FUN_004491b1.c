// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004491b1 | Name: FUN_004491b1


void __fastcall FUN_004491b1(int param_1)

{
  int *piVar1;
  int iVar2;
  undefined4 *local_1c;
  int local_14;
  int local_10;
  int local_c;
  
  piVar1 = *(int **)(param_1 + 0x2873c);
  for (local_c = 0; local_c < 7; local_c = local_c + 1) {
    for (local_14 = 0; local_14 < 9; local_14 = local_14 + 1) {
      iVar2 = (**(code **)(*piVar1 + 0xb4))(local_14,local_c);
      if ((iVar2 != 0) && (*(int *)(iVar2 + 0x94) == -0x10000)) {
        *(undefined4 *)(iVar2 + 0x94) = 0xffffffff;
      }
    }
  }
  _memset((void *)(DAT_013b71e8 + 0xea0),0,0x54);
  *(undefined2 *)(DAT_013b71e8 + 0xea6) = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
  *(undefined2 *)(DAT_013b71e8 + 0xea4) = 0x3c0;
  for (local_10 = 0; local_10 < 8; local_10 = local_10 + 1) {
    *(undefined1 *)(DAT_013b71e8 + local_10 + 0xeec) = 0xff;
  }
  for (local_10 = 0; local_10 < 3; local_10 = local_10 + 1) {
    local_1c = (undefined4 *)0x0;
    if (*(int *)(param_1 + 0x27c3c + local_10 * 4) != 0) {
      local_1c = (undefined4 *)
                 (**(code **)(**(int **)(param_1 + 0x27c3c + local_10 * 4) + 0xa4))(0,0);
    }
    if ((*(int *)(DAT_005ccec0 + 0x1e8) != 0) &&
       (*(undefined4 **)(DAT_005ccec0 + 0x1e8) == local_1c)) {
      *(undefined4 *)(DAT_005ccec0 + 0x1e8) = 0;
    }
    if ((local_1c != (undefined4 *)0x0) && (local_1c != (undefined4 *)0x0)) {
      (**(code **)*local_1c)(1);
    }
  }
  *(undefined4 *)(*(int *)(param_1 + 0x2873c) + 0x400) = 0;
  FUN_00447f6f(1);
  return;
}

