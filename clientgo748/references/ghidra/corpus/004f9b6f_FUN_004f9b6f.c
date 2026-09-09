// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f9b6f | Name: FUN_004f9b6f


void __fastcall FUN_004f9b6f(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int local_10;
  int local_8;
  
  uVar2 = (**(code **)(*DAT_0092e654 + 8))();
  *(undefined4 *)(param_1 + 0x214) = uVar2;
  (**(code **)(*DAT_013b71e8 + 0x68))(param_1);
  iVar1 = *(int *)(param_1 + 0x7bc);
  if (*(int *)(iVar1 + 0x24) == 30000) {
    *(undefined2 *)(param_1 + 0x7b8) = 1;
    if (*(int *)(iVar1 + 0x26e90) == param_1) {
      *(undefined4 *)(iVar1 + 0x26e90) = 0;
    }
    if ((*(int *)(iVar1 + 0x4c) != 0) && (*(int *)(*(int *)(iVar1 + 0x4c) + 0x768) == param_1)) {
      *(undefined4 *)(*(int *)(iVar1 + 0x4c) + 0x768) = 0;
    }
    if ((*(int *)(iVar1 + 0x4c) != 0) && (*(int *)(*(int *)(iVar1 + 0x4c) + 0x76c) == param_1)) {
      *(undefined4 *)(*(int *)(iVar1 + 0x4c) + 0x76c) = 0;
    }
    for (local_10 = 0; local_10 < 0xd; local_10 = local_10 + 1) {
      if (((*(short *)(param_1 + 0x652 + local_10 * 2) != 0) &&
          (iVar3 = (**(code **)(*DAT_013b71e8 + 0x34))
                             (*(undefined2 *)(param_1 + 0x652 + local_10 * 2)), iVar3 != 0)) &&
         (*(short *)(iVar3 + 0x650) == *(short *)(param_1 + 0x20))) {
        *(undefined2 *)(iVar3 + 0x650) = 0;
      }
    }
    if (*(int *)(iVar1 + 0x26e94) == param_1) {
      *(undefined4 *)(iVar1 + 0x26e94) = 0;
    }
    if (*(int *)(iVar1 + 0x4c) == param_1) {
      *(undefined4 *)(iVar1 + 0x4c) = 0;
    }
  }
  if (*(int *)(DAT_0067cf38 + 0x7c) == param_1) {
    *(undefined4 *)(DAT_0067cf38 + 0x7c) = 0;
  }
  if (*(int *)(DAT_013b71e8[0x6c21] + 0x54) == param_1) {
    *(undefined4 *)(DAT_013b71e8[0x6c21] + 0x54) = 0;
  }
  if (*(int *)(param_1 + 0x60) != 0) {
    *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2f4) = 0;
    if ((*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) &&
       (*(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x84) = 0,
       *(int *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x88) != 0)) {
      (**(code **)(*DAT_013b71e8 + 100))
                (*(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x88));
      *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x88) = 0;
    }
    if ((*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0) &&
       (*(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x84) = 0,
       *(int *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x88) != 0)) {
      (**(code **)(*DAT_013b71e8 + 100))
                (*(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x88));
      *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x88) = 0;
    }
  }
  if (*(int *)(param_1 + 0x548) != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x548));
    *(undefined4 *)(param_1 + 0x548) = 0;
  }
  if (*(int *)(param_1 + 0x544) != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x544));
    *(undefined4 *)(param_1 + 0x544) = 0;
  }
  if (*(int *)(param_1 + 0x55c) != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x55c));
    *(undefined4 *)(param_1 + 0x55c) = 0;
  }
  if (*(int *)(param_1 + 0x560) != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x560));
    *(undefined4 *)(param_1 + 0x560) = 0;
  }
  if (*(int *)(param_1 + 0x530) != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x530));
    *(undefined4 *)(param_1 + 0x530) = 0;
  }
  if (*(int *)(param_1 + 0x538) != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x538));
    *(undefined4 *)(param_1 + 0x538) = 0;
  }
  if (*(int *)(param_1 + 0x53c) != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x53c));
    *(undefined4 *)(param_1 + 0x53c) = 0;
  }
  if (*(int *)(param_1 + 0x540) != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x540));
    *(undefined4 *)(param_1 + 0x540) = 0;
  }
  if (*(int *)(param_1 + 0x550) != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x550));
    *(undefined4 *)(param_1 + 0x550) = 0;
  }
  for (local_8 = 0; local_8 < 2; local_8 = local_8 + 1) {
    if (*(int *)(param_1 + 0x554 + local_8 * 4) != 0) {
      (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x554 + local_8 * 4));
      *(undefined4 *)(param_1 + 0x554 + local_8 * 4) = 0;
    }
  }
  for (local_8 = 0; local_8 < 7; local_8 = local_8 + 1) {
    if (*(int *)(param_1 + 0x5a4 + local_8 * 4) != 0) {
      (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x5a4 + local_8 * 4));
      *(undefined4 *)(param_1 + 0x5a4 + local_8 * 4) = 0;
    }
  }
  for (local_8 = 0; local_8 < 7; local_8 = local_8 + 1) {
    if (*(int *)(param_1 + 0x56c + local_8 * 4) != 0) {
      (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x56c + local_8 * 4));
      *(undefined4 *)(param_1 + 0x56c + local_8 * 4) = 0;
    }
    if (*(int *)(param_1 + 0x588 + local_8 * 4) != 0) {
      (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x588 + local_8 * 4));
      *(undefined4 *)(param_1 + 0x588 + local_8 * 4) = 0;
    }
  }
  for (local_8 = 0; local_8 < 4; local_8 = local_8 + 1) {
    if (*(int *)(param_1 + 0x5c0 + local_8 * 4) != 0) {
      (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x5c0 + local_8 * 4));
      *(undefined4 *)(param_1 + 0x5c0 + local_8 * 4) = 0;
    }
  }
  for (local_8 = 0; local_8 < 5; local_8 = local_8 + 1) {
    if (*(int *)(param_1 + 0x5d0 + local_8 * 4) != 0) {
      (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x5d0 + local_8 * 4));
      *(undefined4 *)(param_1 + 0x5d0 + local_8 * 4) = 0;
    }
  }
  for (local_8 = 0; local_8 < 2; local_8 = local_8 + 1) {
    if (*(int *)(param_1 + 0x528 + local_8 * 4) != 0) {
      (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x528 + local_8 * 4));
      *(undefined4 *)(param_1 + 0x528 + local_8 * 4) = 0;
    }
  }
  if (*(int *)(param_1 + 0x514) != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x514));
    *(undefined4 *)(param_1 + 0x514) = 0;
  }
  if (*(int *)(param_1 + 0x518) != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x518));
    *(undefined4 *)(param_1 + 0x518) = 0;
  }
  if (*(int *)(param_1 + 0x51c) != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x51c));
    *(undefined4 *)(param_1 + 0x51c) = 0;
  }
  if (*(int *)(param_1 + 0x520) != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x520));
    *(undefined4 *)(param_1 + 0x520) = 0;
  }
  if (*(int *)(param_1 + 0x524) != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x524));
    *(undefined4 *)(param_1 + 0x524) = 0;
  }
  if (*(int *)(param_1 + 0x42c) != 0) {
    (**(code **)(**(int **)(param_1 + 0x42c) + 0x60))(0);
  }
  if (*(int *)(param_1 + 0x418) != 0) {
    (**(code **)(**(int **)(param_1 + 0x418) + 0x60))(0);
  }
  if (*(int *)(param_1 + 0x41c) != 0) {
    (**(code **)(**(int **)(param_1 + 0x41c) + 0x60))(0);
  }
  if (*(int *)(param_1 + 0x7e4) != 0) {
    (**(code **)(**(int **)(param_1 + 0x7e4) + 0x60))(0);
  }
  if (*(int *)(param_1 + 0x424) != 0) {
    (**(code **)(**(int **)(param_1 + 0x424) + 0x60))(0);
  }
  if ((DAT_005b892c == 2) && (*(int *)(param_1 + 0x420) != 0)) {
    (**(code **)(**(int **)(param_1 + 0x420) + 0x60))(0);
  }
  if (*(int *)(param_1 + 0x428) != 0) {
    (**(code **)(**(int **)(param_1 + 0x428) + 0x60))(0);
  }
  if (*(int *)(param_1 + 0x434) != 0) {
    (**(code **)(**(int **)(param_1 + 0x434) + 0x60))(0);
  }
  if (*(int *)(param_1 + 0x438) != 0) {
    (**(code **)(**(int **)(param_1 + 0x438) + 0x60))(0);
  }
  if (*(int *)(param_1 + 0x430) != 0) {
    (**(code **)(**(int **)(param_1 + 0x430) + 0x60))(0);
  }
  *(undefined4 *)(param_1 + 0x20) = 0xffffffff;
  *(undefined1 *)(param_1 + 0x248) = 0;
  return;
}

