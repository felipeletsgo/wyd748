// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052433d | Name: FUN_0052433d


void __fastcall FUN_0052433d(int param_1)

{
  int iVar1;
  int local_c;
  int local_8;
  
  if ((*(int *)(param_1 + 0x214) == 0) && (*(int *)(param_1 + 0x60) != 0)) {
    *(undefined1 *)(param_1 + 0x77c) = 0;
    *(undefined1 *)(param_1 + 0x77d) = 0;
    *(undefined1 *)(param_1 + 0x77e) = 0;
    *(undefined1 *)(param_1 + 0x77f) = 0;
    *(undefined1 *)(param_1 + 0x781) = 0;
    *(undefined1 *)(param_1 + 0x782) = 0;
    *(undefined1 *)(param_1 + 0x783) = 0;
    *(undefined1 *)(param_1 + 0x784) = 0;
    *(undefined1 *)(param_1 + 0x785) = 0;
    *(undefined1 *)(param_1 + 0x788) = 0;
    *(undefined1 *)(param_1 + 0x786) = 0;
    *(undefined1 *)(param_1 + 0x787) = 0;
    *(undefined1 *)(param_1 + 0x789) = 0;
    *(undefined1 *)(param_1 + 0x78a) = 0;
    *(undefined1 *)(param_1 + 0x78b) = 0;
    *(undefined1 *)(param_1 + 0x78c) = 0;
    *(undefined1 *)(param_1 + 0x790) = 0;
    *(undefined1 *)(param_1 + 0x78d) = 0;
    *(undefined1 *)(param_1 + 0x78f) = 0;
    if ((*(int *)(param_1 + 0x24c) != 0x22) &&
       ((*(int *)(param_1 + 0x24c) != 0x15 || (*(short *)(param_1 + 0x1d2) != 10)))) {
      *(undefined1 *)(param_1 + 0x78e) = 0;
    }
    *(undefined1 *)(param_1 + 0x791) = 0;
    *(undefined1 *)(param_1 + 0x793) = 0;
    *(undefined1 *)(param_1 + 0x792) = 0;
    *(undefined1 *)(param_1 + 0x794) = 0;
    *(undefined1 *)(param_1 + 0x796) = 0;
    *(undefined1 *)(param_1 + 0x797) = 0;
    *(undefined1 *)(param_1 + 0x798) = 0;
    *(undefined1 *)(param_1 + 0x799) = 0;
    FUN_00524324(0);
    for (local_8 = 0; local_8 < 0x10; local_8 = local_8 + 1) {
      iVar1 = (int)(uint)*(ushort *)(param_1 + 0x684 + local_8 * 2) >> 8;
      if (iVar1 == 1) {
        *(undefined1 *)(param_1 + 0x77f) = 1;
      }
      else if (iVar1 == 2) {
        *(undefined1 *)(param_1 + 0x77d) = 1;
      }
      else if (iVar1 == 3) {
        *(undefined1 *)(param_1 + 0x781) = 1;
      }
      else if (iVar1 == 4) {
        *(undefined1 *)(param_1 + 0x782) = 1;
      }
      else if (iVar1 == 7) {
        *(undefined1 *)(param_1 + 0x784) = 1;
      }
      else if (iVar1 == 8) {
        *(undefined1 *)(param_1 + 0x783) = 1;
      }
      else if (iVar1 == 9) {
        *(undefined1 *)(param_1 + 0x787) = 1;
      }
      else if (iVar1 == 10) {
        *(undefined1 *)(param_1 + 0x78b) = 1;
      }
      else if (iVar1 == 0xb) {
        *(undefined1 *)(param_1 + 0x785) = 1;
      }
      else if (iVar1 == 0xd) {
        *(undefined1 *)(param_1 + 0x77e) = 1;
      }
      else if (iVar1 == 0xf) {
        *(undefined1 *)(param_1 + 0x789) = 1;
      }
      else if (iVar1 == 0x11) {
        *(undefined1 *)(param_1 + 0x786) = 1;
      }
      else if (iVar1 == 0x12) {
        *(undefined1 *)(param_1 + 0x793) = 1;
      }
      else if (iVar1 == 0x13) {
        *(undefined1 *)(param_1 + 0x792) = 1;
      }
      else if (iVar1 == 0x14) {
        *(undefined1 *)(param_1 + 0x77c) = 1;
      }
      else if (iVar1 == 0x15) {
        *(undefined1 *)(param_1 + 0x794) = 1;
      }
      else if (iVar1 == 0x16) {
        *(undefined1 *)(param_1 + 0x78a) = 1;
      }
      else if (iVar1 == 0x17) {
        *(undefined1 *)(param_1 + 0x797) = 1;
      }
      else if (iVar1 == 0x19) {
        *(undefined1 *)(param_1 + 0x78c) = 1;
      }
      else if (iVar1 == 0x1a) {
        *(undefined1 *)(param_1 + 0x78e) = 1;
      }
      else if (iVar1 == 0x1b) {
        *(undefined1 *)(param_1 + 0x790) = 1;
      }
      else if (iVar1 == 0x1c) {
        *(undefined1 *)(param_1 + 0x78d) = 1;
      }
      else if (iVar1 == 0x1d) {
        FUN_00524324(1);
      }
      else if (iVar1 == 0x1e) {
        *(undefined1 *)(param_1 + 0x791) = 1;
      }
      else if (iVar1 == 0x1f) {
        *(undefined1 *)(param_1 + 0x796) = 1;
      }
      else if (iVar1 == 0x20) {
        *(undefined1 *)(param_1 + 0x788) = 1;
      }
      else if (iVar1 == 0x24) {
        *(undefined1 *)(param_1 + 0x798) = 1;
      }
      else if (iVar1 == 0x25) {
        *(undefined1 *)(param_1 + 0x799) = 1;
      }
    }
    if (*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) {
      *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f5) =
           *(undefined1 *)(param_1 + 0x794);
    }
    if (*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0) {
      *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f5) =
           *(undefined1 *)(param_1 + 0x794);
    }
    if (*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) {
      *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f3) =
           *(undefined1 *)(param_1 + 0x77e);
    }
    if (*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0) {
      *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f3) =
           *(undefined1 *)(param_1 + 0x77e);
    }
    if (*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) {
      *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f2) =
           *(undefined1 *)(param_1 + 0x790);
    }
    if (*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0) {
      *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f2) =
           *(undefined1 *)(param_1 + 0x790);
    }
    if ((*(char *)(param_1 + 0x783) == '\x01') && (*(char *)(param_1 + 0x784) == '\0')) {
      if (*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) {
        *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f8) = 1;
      }
      if (*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0) {
        *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f8) = 1;
      }
    }
    else {
      if (*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) {
        *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f8) = 0;
      }
      if (*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0) {
        *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f8) = 0;
      }
    }
    if (*(char *)(param_1 + 0x787) == '\x01') {
      if (((*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) == 0) || (*(short *)(param_1 + 0x66e) == 0))
         || (*(char *)(param_1 + 0x7b2) != '\0')) {
        if (*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) {
          *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f1) = 0;
        }
      }
      else {
        *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f1) = 1;
      }
      if ((*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) == 0) || (*(short *)(param_1 + 0x66c) == 0)) {
        if (*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0) {
          *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f1) = 0;
        }
      }
      else {
        *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f1) = 1;
      }
    }
    else {
      if (*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) {
        *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x28f1) = 0;
      }
      if (*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0) {
        *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x28f1) = 0;
      }
    }
    local_c = 0;
    if (*(int *)(DAT_0067cf38 + 0x24) == 30000) {
      local_c = DAT_0067cf38;
    }
    if ((param_1 == *(int *)(DAT_0067cf38 + 0x4c)) && (local_c != 0)) {
      FUN_004431e4(0);
      if (*(char *)(param_1 + 0x246) == '\0') {
        FUN_004fa8a1(*(undefined4 *)(local_c + 0x26e7c));
      }
    }
    else if (*(char *)(param_1 + 0x246) == '\0') {
      FUN_004fa8a1(0);
    }
  }
  return;
}

