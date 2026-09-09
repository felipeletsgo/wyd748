// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f9946 | Name: FUN_004f9946


void __thiscall FUN_004f9946(int param_1,undefined4 param_2)

{
  void *pvVar1;
  undefined4 local_24;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a17b5;
  local_10 = ExceptionList;
  if (((*(int *)(param_1 + 0x430) == 0) && (*(int *)(DAT_0067cf38 + 0x24) == 30000)) &&
     (*(int *)(DAT_0067cf38 + 0x4c) != param_1)) {
    ExceptionList = &local_10;
    pvVar1 = operator_new(0x1e4);
    local_8 = 0;
    if (pvVar1 == (void *)0x0) {
      local_24 = 0;
    }
    else {
      local_24 = FUN_00401466(0xfffffffe,0,0,0x40800000,0x40800000,param_2,4);
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x430) = local_24;
    if (*(int *)(param_1 + 0x430) != 0) {
      *(undefined4 *)(*(int *)(param_1 + 0x430) + 0x3c) = 0;
      FUN_0054ac09(*(undefined4 *)(param_1 + 0x430));
    }
  }
  ExceptionList = local_10;
  return;
}

