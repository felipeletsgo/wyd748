// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00494e68 | Name: FUN_00494e68


void __thiscall FUN_00494e68(int *param_1,undefined4 param_2)

{
  void *pvVar1;
  char *pcVar2;
  int local_a8;
  undefined4 local_9c;
  undefined4 local_98;
  char local_94 [128];
  size_t local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059fff2;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (param_1[10] == 0) {
    ExceptionList = &local_10;
    pvVar1 = operator_new(0x154);
    local_8 = 0;
    if (pvVar1 == (void *)0x0) {
      local_a8 = 0;
    }
    else {
      local_a8 = FUN_0040c2cd(param_1);
    }
    local_9c = local_a8;
    param_1[10] = local_a8;
  }
  local_8 = 0xffffffff;
  local_98 = 0;
  FUN_0058f078(local_94,&DAT_005b89d4,param_2);
  local_14 = _strlen(local_94);
  pcVar2 = _strchr(local_94,0x5f);
  if (pcVar2 == (char *)0x0) {
    FUN_0058f078(local_94 + (local_14 - 3),&PTR_DAT_005b89e0);
  }
  else {
    FUN_0058f078((int)&local_9c + local_14 + 1,&DAT_005b89d8);
  }
  (**(code **)(*param_1 + 0x44))(local_94);
  ExceptionList = local_10;
  return;
}

