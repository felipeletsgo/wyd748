// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00407add | Name: FUN_00407add


void __fastcall FUN_00407add(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0059ec4b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a3b38;
  local_8 = 0;
  if (param_1[0x398] != 0) {
    if ((undefined4 *)param_1[0x398] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x398])(1);
    }
    param_1[0x398] = 0;
  }
  if (param_1[0x39a] != 0) {
    if ((undefined4 *)param_1[0x39a] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x39a])(1);
    }
    param_1[0x39a] = 0;
  }
  if (param_1[0x399] != 0) {
    if ((undefined4 *)param_1[0x399] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x399])(1);
    }
    param_1[0x399] = 0;
  }
  local_8 = 0xffffffff;
  FUN_00407265();
  ExceptionList = local_10;
  return;
}

