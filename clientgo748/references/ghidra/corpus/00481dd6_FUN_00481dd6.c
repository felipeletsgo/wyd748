// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00481dd6 | Name: FUN_00481dd6


undefined4 __thiscall FUN_00481dd6(int param_1,int param_2)

{
  undefined4 uVar1;
  void *pvVar2;
  int local_bc;
  undefined1 local_a0 [128];
  int *local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f9df;
  local_10 = ExceptionList;
  if ((((*(int *)(param_1 + 0x27af8) != 0) && (*(int *)(param_1 + 0x2872c) != 0)) &&
      ((DAT_005b892c != 2 || (*(int *)(*(int *)(param_1 + 0x27af8) + 0x1e8) != 0)))) &&
     (ExceptionList = &local_10,
     local_14 = (**(code **)(*DAT_013b71e8 + 0x34))(*(undefined2 *)(param_2 + 6)), local_14 == 0)) {
    local_1c = *(int *)(param_1 + 0x2872c);
    for (local_18 = 0; local_18 < *(int *)(local_1c + 0x210); local_18 = local_18 + 1) {
      local_20 = *(int **)(local_1c + 0x214 + local_18 * 4);
      if (local_20[0x394] == (uint)*(ushort *)(param_2 + 6)) {
        *(undefined1 *)(param_2 + 0x6b) = 0;
        *(undefined1 *)(param_2 + 0x6a) = 0;
        uVar1 = (**(code **)(*local_20 + 0x88))(param_2 + 0xc);
        FUN_0058f078(local_a0,s___s_>__s_005b87f0,uVar1);
        pvVar2 = operator_new(0xe50);
        local_8 = 0;
        if (pvVar2 == (void *)0x0) {
          local_bc = 0;
        }
        else {
          local_bc = FUN_00407203(local_a0,0xffaaffaa,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
        }
        local_8 = 0xffffffff;
        if (local_bc != 0) {
          FUN_00408d33(local_bc);
        }
        uVar1 = (**(code **)(*DAT_0092e654 + 8))();
        *(undefined4 *)(param_1 + 0x27464) = uVar1;
        ExceptionList = local_10;
        return 0;
      }
    }
  }
  ExceptionList = local_10;
  return 0;
}

