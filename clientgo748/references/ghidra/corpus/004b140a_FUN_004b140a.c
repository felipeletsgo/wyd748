// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b140a | Name: FUN_004b140a


undefined4 __thiscall FUN_004b140a(int param_1,int param_2,int param_3)

{
  void *pvVar1;
  undefined4 local_24;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a041b;
  local_10 = ExceptionList;
  if (*(int *)(param_1 + 0x48 + param_2 * 0x58) == 0) {
    ExceptionList = &local_10;
    pvVar1 = operator_new(0x360);
    local_8 = 0;
    if (pvVar1 == (void *)0x0) {
      local_24 = 0;
    }
    else {
      local_24 = FUN_004baf10();
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x48 + param_2 * 0x58) = local_24;
    if (*(int *)(param_1 + 0x48 + param_2 * 0x58) != 0) {
      if (param_3 == 1) {
        *(undefined4 *)(*(int *)(param_1 + 0x48 + param_2 * 0x58) + 4) = 1;
      }
      FUN_004bc7c7(param_1 + 8 + param_2 * 0x58);
      ExceptionList = local_10;
      return 1;
    }
  }
  ExceptionList = local_10;
  return 0;
}

