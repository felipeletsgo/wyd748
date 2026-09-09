// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f8ebb | Name: FUN_004f8ebb


void __fastcall FUN_004f8ebb(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_005a17a0;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a557c;
  local_8 = 0;
  DAT_0092eaf0 = DAT_0092eaf0 + -1;
  if (*(short *)(param_1 + 0x1ee) == 0) {
    DAT_0092eaf4 = DAT_0092eaf4 + 1;
  }
  if (param_1[0x10b] != 0) {
    if ((undefined4 *)param_1[0x10b] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x10b])(1);
    }
    param_1[0x10b] = 0;
  }
  if (param_1[0x106] != 0) {
    if ((undefined4 *)param_1[0x106] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x106])(1);
    }
    param_1[0x106] = 0;
  }
  if (param_1[0x1f9] != 0) {
    if ((undefined4 *)param_1[0x1f9] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x1f9])(1);
    }
    param_1[0x1f9] = 0;
  }
  if (param_1[0x109] != 0) {
    if ((undefined4 *)param_1[0x109] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x109])(1);
    }
    param_1[0x109] = 0;
  }
  if ((DAT_005b892c == 2) && (param_1[0x108] != 0)) {
    if ((undefined4 *)param_1[0x108] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x108])(1);
    }
    param_1[0x108] = 0;
  }
  if (param_1[0x10a] != 0) {
    if ((undefined4 *)param_1[0x10a] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x10a])(1);
    }
    param_1[0x10a] = 0;
  }
  if (param_1[0x10d] != 0) {
    if ((undefined4 *)param_1[0x10d] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x10d])(1);
    }
    param_1[0x10d] = 0;
  }
  if (param_1[0x10e] != 0) {
    if ((undefined4 *)param_1[0x10e] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x10e])(1);
    }
    param_1[0x10e] = 0;
  }
  if (param_1[0x10c] != 0) {
    if ((undefined4 *)param_1[0x10c] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x10c])(1);
    }
    param_1[0x10c] = 0;
  }
  if (param_1[0x10f] != 0) {
    if ((undefined4 *)param_1[0x10f] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x10f])(1);
    }
    param_1[0x10f] = 0;
  }
  if (param_1[0x18] != 0) {
    if (param_1[0x18] != 0) {
      FUN_00493d20(1);
    }
    param_1[0x18] = 0;
  }
  if (param_1[0x66] != 0) {
    if (param_1[0x66] != 0) {
      FUN_00493d20(1);
    }
    param_1[0x66] = 0;
  }
  if (param_1[0x67] != 0) {
    if (param_1[0x67] != 0) {
      FUN_00493d20(1);
    }
    param_1[0x67] = 0;
  }
  local_8 = 0xffffffff;
  FUN_0053e26b();
  ExceptionList = local_10;
  return;
}

