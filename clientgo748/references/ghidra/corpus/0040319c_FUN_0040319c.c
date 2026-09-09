// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040319c | Name: FUN_0040319c


void __fastcall FUN_0040319c(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0059e996;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a3674;
  local_8 = 0;
  if (param_1[0x6c] != 0) {
    if ((undefined4 *)param_1[0x6c] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x6c])(1);
    }
    param_1[0x6c] = 0;
  }
  if (param_1[0x7c] != 0) {
    if ((undefined4 *)param_1[0x7c] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x7c])(1);
    }
    param_1[0x7c] = 0;
  }
  local_8 = 0xffffffff;
  FUN_0040151a();
  ExceptionList = local_10;
  return;
}

