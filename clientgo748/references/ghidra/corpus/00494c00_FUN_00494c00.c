// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00494c00 | Name: FUN_00494c00


void __fastcall FUN_00494c00(undefined4 *param_1)

{
  undefined4 uVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0059ffda;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a443c;
  local_8 = 0;
  DAT_0092e654[0x18] = 20000;
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  (**(code **)(*DAT_013b71e8 + 0x2c))(param_1[0xb],uVar1);
  if (param_1[0xd] != 0) {
    if ((undefined4 *)param_1[0xd] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0xd])(1);
    }
    param_1[0xd] = 0;
  }
  if (param_1[0xe] != 0) {
    if ((undefined4 *)param_1[0xe] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0xe])(1);
    }
    param_1[0xe] = 0;
  }
  if (param_1[10] != 0) {
    if ((undefined4 *)param_1[10] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[10])(1);
    }
    param_1[10] = 0;
  }
  DAT_013b71e8[0x6c22] = 0;
  FUN_004b16c0();
  param_1[0x10] = 0;
  DAT_0092eaf4 = 0;
  DAT_0092e654[0x18] = 0;
  local_8 = 0xffffffff;
  FUN_0054aa45();
  ExceptionList = local_10;
  return;
}

