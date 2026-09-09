// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00408bed | Name: FUN_00408bed


void __fastcall FUN_00408bed(int param_1)

{
  void *pvVar1;
  int local_24;
  undefined4 local_20;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059ed1d;
  local_10 = ExceptionList;
  if (*(int *)(param_1 + 0x208) == 0) {
    ExceptionList = &local_10;
    *(undefined1 *)(param_1 + 0x1e9) = 1;
    *(undefined2 *)(param_1 + 0x1ea) = 0;
    pvVar1 = operator_new(0x1064);
    local_8 = 0;
    if (pvVar1 == (void *)0x0) {
      local_20 = 0;
    }
    else {
      local_20 = FUN_004067e9(0xfffffffe,&DAT_005ccef8,0x4f,0,0xffffffff,0,0,
                              *(undefined4 *)(param_1 + 0x54),0x41800000,0,0,1,0);
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x208) = local_20;
    if (*(int *)(param_1 + 0x208) != 0) {
      (**(code **)(**(int **)(param_1 + 0x208) + 0x60))(1);
      (**(code **)(**(int **)(param_1 + 0x208) + 0x44))(2);
      if (param_1 == 0) {
        local_24 = 0;
      }
      else {
        local_24 = param_1 + 0x1e4;
      }
      (**(code **)(**(int **)(param_1 + 0x208) + 0x50))(local_24);
      FUN_0054ac09(*(undefined4 *)(param_1 + 0x208));
    }
  }
  ExceptionList = local_10;
  return;
}

