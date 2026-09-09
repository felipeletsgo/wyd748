// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00455d02 | Name: FUN_00455d02


undefined4 __thiscall FUN_00455d02(int param_1,int param_2)

{
  int iVar1;
  void *pvVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  int local_38;
  int local_20;
  uint local_1c;
  int local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f60f;
  local_10 = ExceptionList;
  local_14 = *(int **)(param_1 + 0x2873c);
  ExceptionList = &local_10;
  local_18 = FUN_0054cd07(param_2 + 0x160,0x21);
  if ((7 < local_18) || (local_18 < 0)) {
    local_18 = 0;
  }
  (**(code **)(*local_14 + 0x98))
            (&local_20,*(undefined4 *)(&DAT_005bec0c + local_18 * 8),
             *(undefined4 *)(&DAT_005bec10 + local_18 * 8));
  if (((local_20 < 0) || (0x7fffffff < local_1c)) &&
     (iVar1 = FUN_0054cd07(param_2 + 0x160,0x26), iVar1 != 2)) {
    pvVar2 = operator_new(0xe50);
    local_8 = 0;
    if (pvVar2 == (void *)0x0) {
      local_38 = 0;
    }
    else {
      local_38 = FUN_00407203(&DAT_00a341f8,0xffffaaaa,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
    }
    local_8 = 0xffffffff;
    if (local_38 != 0) {
      FUN_00408d33(local_38);
    }
    if ((DAT_005ccf98 != 0) && (iVar1 = FUN_00429a6d(0x21), iVar1 != 0)) {
      uVar4 = 0;
      uVar3 = 0;
      FUN_00429a6d(0x21);
      FUN_0042ad2b(uVar3,uVar4);
    }
    uVar3 = (**(code **)(*DAT_0092e654 + 8))();
    *(undefined4 *)(param_1 + 0x27474) = uVar3;
    ExceptionList = local_10;
    return 0;
  }
  FUN_0051a56c(param_2);
  ExceptionList = local_10;
  return 1;
}

