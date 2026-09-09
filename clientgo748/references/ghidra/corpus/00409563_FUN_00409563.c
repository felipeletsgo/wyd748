// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00409563 | Name: FUN_00409563


undefined4 __thiscall FUN_00409563(int param_1,int param_2,int param_3)

{
  void *pvVar1;
  int *piVar2;
  undefined4 uVar3;
  int local_34;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059ed32;
  local_10 = ExceptionList;
  if (param_2 == 1) {
    ExceptionList = &local_10;
    FUN_0040927a(param_3);
    uVar3 = 1;
  }
  else if (param_2 == 2) {
    if ((param_3 == 0) || (param_3 == 8)) {
      if (*(int *)(param_1 + 0xc) + -1 <= (int)*(short *)(param_1 + 6)) {
        return 1;
      }
      ExceptionList = &local_10;
      uVar3 = (**(code **)(**(int **)(param_1 + 0x24) + 0x88))();
      pvVar1 = operator_new(0xe50);
      local_8 = 0;
      if (pvVar1 == (void *)0x0) {
        local_34 = 0;
      }
      else {
        local_34 = FUN_00407203(uVar3,0xffffffff,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
      }
      local_8 = 0xffffffff;
      if (local_34 != 0) {
        FUN_00408d33(local_34);
      }
      (**(code **)(**(int **)(param_1 + 0x24) + 0x90))(&DAT_005ccefc);
      *(short *)(param_1 + 6) = *(short *)(param_1 + 6) + 1;
      if (param_3 == 8) {
        (**(code **)(**(int **)(param_1 + 0x24) + 0x14))
                  (*(undefined1 *)(*(int *)(param_1 + 0x24) + 0x104c),0);
      }
    }
    else if (param_3 == 7) {
      if (0 < *(short *)(param_1 + 6)) {
        ExceptionList = &local_10;
        *(short *)(param_1 + 6) = *(short *)(param_1 + 6) + -1;
        piVar2 = (int *)FUN_00409046((int)*(short *)(param_1 + 6));
        if (piVar2 == (int *)0x0) {
          ExceptionList = local_10;
          return 1;
        }
        uVar3 = (**(code **)(*piVar2 + 0x88))();
        (**(code **)(**(int **)(param_1 + 0x24) + 0x90))(uVar3);
        FUN_00409078(piVar2);
      }
    }
    else if (param_3 == 2) {
      ExceptionList = &local_10;
      (**(code **)(**(int **)(param_1 + 0x28) + 0x90))();
    }
    else if (param_3 == 4) {
      ExceptionList = &local_10;
      (**(code **)(**(int **)(param_1 + 0x28) + 0x94))();
    }
    uVar3 = 1;
  }
  else {
    uVar3 = 0;
  }
  ExceptionList = local_10;
  return uVar3;
}

