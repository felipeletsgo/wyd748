// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00534a75 | Name: FUN_00534a75


undefined4 __thiscall FUN_00534a75(int param_1,int param_2)

{
  undefined4 uVar1;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  if (param_2 == 0) {
    uVar1 = 0;
  }
  else {
    *(undefined4 *)(param_1 + 0xac) = 0;
    *(undefined4 *)(param_1 + 0xb0) = 0;
    *(undefined4 *)(param_1 + 0xb4) = 0;
    *(undefined4 *)(param_1 + 0xb8) = 0;
    if (*(int *)(param_2 + 0x20a20) == *(int *)(param_1 + 0x20a20) + 1) {
      *(int *)(param_1 + 0xb0) = param_2;
      *(int *)(*(int *)(param_1 + 0xb0) + 0xac) = param_1;
      for (local_8 = 0; local_8 < 0x40; local_8 = local_8 + 1) {
        *(undefined1 *)(param_2 + 0xcc + local_8 * 0x300) =
             *(undefined1 *)(param_1 + 0xcc + (local_8 * 0x40 + 0x3f) * 0xc);
        *(undefined4 *)(param_2 + 0xd4 + local_8 * 0x300) =
             *(undefined4 *)(param_1 + 0xd4 + (local_8 * 0x40 + 0x3f) * 0xc);
        FUN_004310a0(param_1 + 0x140cc + (local_8 * 0x40 + 0x3f) * 0xc);
      }
      *(undefined4 *)(param_1 + 200) = 0;
      *(undefined4 *)(*(int *)(param_1 + 0xb0) + 200) = 1;
      uVar1 = 1;
    }
    else if (*(int *)(param_2 + 0x20a20) == *(int *)(param_1 + 0x20a20) + -1) {
      *(int *)(param_1 + 0xac) = param_2;
      *(int *)(*(int *)(param_1 + 0xac) + 0xb0) = param_1;
      for (local_c = 0; local_c < 0x40; local_c = local_c + 1) {
        *(undefined1 *)(param_1 + 0xcc + local_c * 0x300) =
             *(undefined1 *)(param_2 + 0xcc + (local_c * 0x40 + 0x3f) * 0xc);
        *(undefined4 *)(param_1 + 0xd4 + local_c * 0x300) =
             *(undefined4 *)(param_2 + 0xd4 + (local_c * 0x40 + 0x3f) * 0xc);
        FUN_004310a0(param_2 + 0x140cc + (local_c * 0x40 + 0x3f) * 0xc);
      }
      *(undefined4 *)(param_1 + 200) = 1;
      *(undefined4 *)(*(int *)(param_1 + 0xac) + 200) = 0;
      uVar1 = 1;
    }
    else if (*(int *)(param_2 + 0x20a24) == *(int *)(param_1 + 0x20a24) + 1) {
      *(int *)(param_1 + 0xb8) = param_2;
      *(int *)(*(int *)(param_1 + 0xb8) + 0xb4) = param_1;
      for (local_10 = 0; local_10 < 0x40; local_10 = local_10 + 1) {
        *(undefined1 *)(param_2 + 0xcc + local_10 * 0xc) =
             *(undefined1 *)(param_1 + 0xcc + (local_10 + 0xfc0) * 0xc);
        *(undefined4 *)(param_2 + 0xd4 + local_10 * 0xc) =
             *(undefined4 *)(param_1 + 0xd4 + (local_10 + 0xfc0) * 0xc);
        FUN_004310a0(param_1 + 0x140cc + (local_10 + 0xfc0) * 0xc);
      }
      *(undefined4 *)(param_1 + 200) = 0;
      *(undefined4 *)(*(int *)(param_1 + 0xb8) + 200) = 2;
      uVar1 = 1;
    }
    else if (*(int *)(param_2 + 0x20a24) == *(int *)(param_1 + 0x20a24) + -1) {
      *(int *)(param_1 + 0xb4) = param_2;
      *(int *)(*(int *)(param_1 + 0xb4) + 0xb8) = param_1;
      for (local_14 = 0; local_14 < 0x40; local_14 = local_14 + 1) {
        *(undefined1 *)(param_1 + 0xcc + local_14 * 0xc) =
             *(undefined1 *)(param_2 + 0xcc + (local_14 + 0xfc0) * 0xc);
        *(undefined4 *)(param_1 + 0xd4 + local_14 * 0xc) =
             *(undefined4 *)(param_2 + 0xd4 + (local_14 + 0xfc0) * 0xc);
        FUN_004310a0(param_2 + 0x140cc + (local_14 + 0xfc0) * 0xc);
      }
      *(undefined4 *)(param_1 + 200) = 2;
      *(undefined4 *)(*(int *)(param_1 + 0xb4) + 200) = 0;
      uVar1 = 1;
    }
    else {
      uVar1 = 0;
    }
  }
  return uVar1;
}

