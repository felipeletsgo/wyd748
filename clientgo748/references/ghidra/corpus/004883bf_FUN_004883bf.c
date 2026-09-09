// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004883bf | Name: FUN_004883bf


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_004883bf(int param_1,int param_2)

{
  int *piVar1;
  void *pvVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  int local_d8;
  int local_d4;
  int local_d0;
  undefined1 local_90 [128];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059fb86;
  local_10 = ExceptionList;
  iVar3 = *(int *)(param_1 + 0x2872c);
  ExceptionList = &local_10;
  *(undefined1 *)(param_2 + 0x25) = 0;
  pvVar2 = operator_new(0xe6c);
  local_8 = 0;
  if (pvVar2 == (void *)0x0) {
    local_d0 = 0;
  }
  else {
    local_d0 = FUN_004077ce(param_2 + 0x16,0xffffffff,0,0,0x42d00000,0x41a00000,
                            *(undefined2 *)(param_2 + 0x14),(int)*(char *)(param_2 + 0xc),
                            (int)*(short *)(param_2 + 0xe),(int)*(short *)(param_2 + 0x12),
                            (int)*(short *)(param_2 + 0x10));
  }
  local_8 = 0xffffffff;
  if (local_d0 != 0) {
    if (*(char *)(param_2 + 0xd) == '\0') {
      *(undefined4 *)(local_d0 + 0xe5c) = 1;
    }
    if (0 < *(int *)(iVar3 + 0x210)) {
      FUN_004091ab();
    }
    FUN_00408d33(local_d0);
    if (DAT_005b892c == 2) {
      if (*(int *)(param_1 + 0x27d8c) != 0) {
        (**(code **)(**(int **)(param_1 + 0x27d8c) + 0x60))(1);
      }
      if (*(int *)(param_1 + 0x27d90) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x27d90) + 0x1e8) = 0;
      }
    }
    else {
      iVar3 = *(int *)(iVar3 + 0x210);
      piVar1 = *(int **)(param_1 + 0x27d8c);
      (**(code **)(*piVar1 + 0x74))(0x42e40000,(float)iVar3 * _DAT_005a4374 + _DAT_005a4370);
      (**(code **)(*piVar1 + 0x70))(0,_DAT_005a436c - (float)iVar3 * _DAT_005a4374);
      FUN_0040981b(0x42e40000,(float)iVar3 * _DAT_005a4374);
    }
    iVar3 = (**(code **)(*DAT_013b71e8 + 0x34))(*(undefined2 *)(param_2 + 0x14));
    if (iVar3 != 0) {
      FUN_004f9946(0xaaffff00);
    }
    iVar3 = FUN_0040c0f0();
    if (iVar3 == 0) {
      FUN_0044da6f();
    }
    FUN_0058f078(local_90,&DAT_00a36078,param_2 + 0x16);
    pvVar2 = operator_new(0xe50);
    local_8 = 1;
    if (pvVar2 == (void *)0x0) {
      local_d4 = 0;
    }
    else {
      local_d4 = FUN_00407203(local_90,0xffccaaff,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
    }
    local_8 = 0xffffffff;
    if (local_d4 != 0) {
      FUN_00408d33(local_d4);
    }
    FUN_0058f078(local_90,&DAT_00a360f8,param_2 + 0x16);
    pvVar2 = operator_new(0xe50);
    local_8 = 2;
    if (pvVar2 == (void *)0x0) {
      local_d8 = 0;
    }
    else {
      local_d8 = FUN_00407203(local_90,0xffccaaff,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
    }
    local_8 = 0xffffffff;
    if (local_d8 != 0) {
      FUN_00408d33(local_d8);
      uVar4 = (**(code **)(*DAT_0092e654 + 8))();
      *(undefined4 *)(param_1 + 0x27464) = uVar4;
      if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(0x21), iVar3 != 0)) {
        uVar5 = 0;
        uVar4 = 0;
        FUN_00429a6d(0x21);
        FUN_0042ad2b(uVar4,uVar5);
      }
    }
  }
  ExceptionList = local_10;
  return 1;
}

