// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00488267 | Name: FUN_00488267


undefined4 __thiscall FUN_00488267(int param_1,int param_2)

{
  int iVar1;
  void *pvVar2;
  int local_30;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059fb52;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = (**(code **)(*DAT_013b71e8 + 0x34))(*(undefined2 *)(param_2 + 0x12));
  if (iVar1 != 0) {
    if ((*(char *)(iVar1 + 0x248) == '\x01') || (*(int *)(param_1 + 0x4c) == iVar1)) {
      FUN_0048150a(*(undefined4 *)(param_2 + 0x14),*(undefined4 *)(param_2 + 0xc));
      if ((*(char *)(param_1 + 0x26eb4) == '\x01') && (*(int *)(param_1 + 0x4c) == iVar1)) {
        *(undefined4 *)(param_1 + 0x26e90) = 0;
      }
    }
    pvVar2 = operator_new(0xa4);
    local_8 = 0;
    if (pvVar2 == (void *)0x0) {
      local_30 = 0;
    }
    else {
      local_30 = FUN_004ce5e0(iVar1,0,0xffffffff);
    }
    local_8 = 0xffffffff;
    if ((*(int *)(param_1 + 0x2c) != 0) && (local_30 != 0)) {
      FUN_0054ac09(local_30);
    }
  }
  iVar1 = (**(code **)(*DAT_013b71e8 + 0x34))(*(undefined2 *)(param_2 + 0x10));
  if (iVar1 != 0) {
    *(undefined2 *)(iVar1 + 0x450) = 0;
    FUN_0052355c();
  }
  ExceptionList = local_10;
  return 1;
}

