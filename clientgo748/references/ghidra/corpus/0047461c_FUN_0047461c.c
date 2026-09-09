// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0047461c | Name: FUN_0047461c


void __fastcall FUN_0047461c(int param_1)

{
  undefined4 uVar1;
  void *pvVar2;
  int *piVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 local_2c;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f786;
  local_10 = ExceptionList;
  if ((((*(int *)(param_1 + 0x27d50) != 0) && (*(int *)(param_1 + 0x27d64) != 0)) &&
      (*(int *)(param_1 + 0x27d58) != 0)) && (*(int *)(param_1 + 0x27d60) != 0)) {
    ExceptionList = &local_10;
    FUN_004091ab();
    uVar1 = (**(code **)(**(int **)(param_1 + 0x27d64) + 0x88))();
    (**(code **)(**(int **)(param_1 + 0x27d60) + 0x90))(uVar1);
    for (local_14 = 0; local_14 < *(int *)(*(int *)(param_1 + 0x27d50) + 0x210) + -1;
        local_14 = local_14 + 1) {
      pvVar2 = operator_new(0xe50);
      local_8 = 0;
      if (pvVar2 == (void *)0x0) {
        local_2c = 0;
      }
      else {
        uVar12 = 0;
        uVar11 = 1;
        uVar10 = 0x77777777;
        uVar9 = 0;
        uVar8 = 0x41800000;
        uVar7 = 0x43960000;
        uVar6 = 0;
        uVar5 = 0;
        uVar4 = 0xffffffff;
        piVar3 = (int *)FUN_00409046(local_14);
        uVar1 = (**(code **)(*piVar3 + 0x88))
                          (uVar4,uVar5,uVar6,uVar7,uVar8,uVar9,uVar10,uVar11,uVar12);
        local_2c = FUN_00407203(uVar1,uVar4,uVar5,uVar6,uVar7,uVar8,uVar9,uVar10,uVar11,uVar12);
      }
      local_8 = 0xffffffff;
      FUN_00408d33(local_2c);
    }
    *(undefined2 *)(*(int *)(param_1 + 0x27d58) + 0x1ea) = (undefined2)local_14;
    piVar3 = (int *)FUN_00409046(local_14);
    uVar1 = (**(code **)(*piVar3 + 0x88))();
    (**(code **)(**(int **)(*(int *)(param_1 + 0x27d58) + 0x208) + 0x90))(uVar1);
    FUN_004091ab();
  }
  ExceptionList = local_10;
  return;
}

