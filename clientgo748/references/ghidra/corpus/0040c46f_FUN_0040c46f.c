// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040c46f | Name: FUN_0040c46f


void __fastcall FUN_0040c46f(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0059ee02;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a3f34;
  param_1[9] = &PTR_FUN_005a3f30;
  local_8 = 0;
  if (param_1[10] != 0) {
    if ((undefined4 *)param_1[10] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[10])(1);
    }
    param_1[10] = 0;
  }
  if (param_1[0xc] != 0) {
    if ((undefined4 *)param_1[0xc] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0xc])(1);
    }
    param_1[0xc] = 0;
  }
  local_8 = 0xffffffff;
  FUN_0054aa45();
  ExceptionList = local_10;
  return;
}

