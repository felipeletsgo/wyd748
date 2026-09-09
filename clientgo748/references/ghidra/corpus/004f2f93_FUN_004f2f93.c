// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f2f93 | Name: FUN_004f2f93


void __thiscall FUN_004f2f93(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  
  if ((((*(int *)(param_1 + 0x2e8) != 1) || (param_2 != 4)) &&
      ((*(int *)(param_1 + 0x2e8) != 2 || (param_2 != 5)))) &&
     ((*(int *)(param_1 + 0x2e8) != 3 || (param_2 != 6)))) {
    if (((param_2 == 5) || (param_2 == 4)) &&
       (((*(int *)(param_1 + 0x2f8) == 0x8d && (DAT_005ccf98 != 0)) &&
        (iVar1 = FUN_00429a6d(0x39), iVar1 != 0)))) {
      uVar3 = 0;
      uVar2 = 0;
      FUN_00429a6d(0x39);
      FUN_0042ad2b(uVar2,uVar3);
    }
    uVar2 = (**(code **)(*DAT_0092e654 + 8))();
    *(undefined4 *)(param_1 + 0x2ec) = uVar2;
    *(int *)(param_1 + 0x2e8) = param_2;
  }
  return;
}

