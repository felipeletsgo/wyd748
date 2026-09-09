// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004a2aef | Name: FUN_004a2aef


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_004a2aef(int param_1,undefined4 param_2)

{
  int *piVar1;
  int iVar2;
  float10 fVar3;
  undefined1 local_10 [4];
  undefined2 local_c;
  undefined2 local_a;
  
  FUN_0049a73a(param_2);
  iVar2 = (**(code **)(*DAT_0092e654 + 8))();
  if (((3000 < (uint)(iVar2 - *(int *)(param_1 + 0x26e84))) && (*(int *)(param_1 + 0x26e50) != 0))
     && (*(int *)(*(int *)(param_1 + 0x26e50) + 0x2c) == 0)) {
    (**(code **)(**(int **)(param_1 + 0x26e50) + 100))(1);
    (**(code **)(**(int **)(param_1 + 0x26e54) + 100))(1);
    (**(code **)(**(int **)(param_1 + 0x26e58) + 100))(1);
  }
  if (((2000 < (uint)(iVar2 - *(int *)(param_1 + 0x26e88))) && (*(int *)(param_1 + 0x26e5c) != 0))
     && (*(int *)(*(int *)(param_1 + 0x26e5c) + 0x2c) == 0)) {
    (**(code **)(**(int **)(param_1 + 0x26e5c) + 100))(1);
  }
  if (*(int *)(param_1 + 0x26e1c) == 1) {
    (**(code **)(**(int **)(param_1 + 0x26e5c) + 100))(0);
    (**(code **)(**(int **)(param_1 + 0x26e50) + 100))(0);
    (**(code **)(**(int **)(param_1 + 0x26e54) + 100))(0);
    (**(code **)(**(int **)(param_1 + 0x26e58) + 100))(0);
  }
  if (300000 < (uint)(iVar2 - DAT_0092e65c)) {
    _memset(local_10,0,0xc);
    local_a = 0;
    local_c = 0x3a0;
    FUN_0042550e(local_10,0xc);
  }
  if (*(int *)(param_1 + 0x54) == 1) {
    FUN_00403df2(s_Critical_Data_Error_In_Client_005b93b4,0);
    (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,0);
  }
  else if (DAT_005b892c != 2) {
    if (*(int *)(param_1 + 0x26e74) != 0) {
      piVar1 = *(int **)(param_1 + 0x26e60);
      if ((uint)(iVar2 - *(int *)(param_1 + 0x26e74)) < 700) {
        fVar3 = (float10)FUN_00423b00(((float)(uint)(iVar2 - *(int *)(param_1 + 0x26e74)) *
                                      _DAT_005a4380) / _DAT_005a4540);
        (**(code **)(*piVar1 + 0x70))
                  (0x435a0000,(float)((float10)_DAT_005a4538 - fVar3 * (float10)_DAT_005a453c));
      }
      else {
        (**(code **)(*piVar1 + 0x70))(0x435a0000,0x43998000);
        *(undefined4 *)(param_1 + 0x26e74) = 0;
      }
    }
    if (*(int *)(param_1 + 0x26e78) != 0) {
      piVar1 = *(int **)(param_1 + 0x26e60);
      if ((uint)(iVar2 - *(int *)(param_1 + 0x26e78)) < 400) {
        fVar3 = (float10)FUN_00423b00(((float)(uint)(iVar2 - *(int *)(param_1 + 0x26e78)) *
                                      _DAT_005a4380) / _DAT_005a4534);
        (**(code **)(*piVar1 + 0x70))
                  (0x435a0000,(float)(fVar3 * (float10)_DAT_005a453c + (float10)_DAT_005a4530));
      }
      else {
        (**(code **)(*piVar1 + 0x70))(0x435a0000,0x4408c000);
        *(undefined4 *)(param_1 + 0x26e78) = 0;
      }
    }
    if (*(int *)(param_1 + 0x26e7c) != 0) {
      piVar1 = *(int **)(param_1 + 0x26e64);
      if ((uint)(iVar2 - *(int *)(param_1 + 0x26e7c)) < 500) {
        fVar3 = (float10)FUN_00423b00(((float)(uint)(iVar2 - *(int *)(param_1 + 0x26e7c)) *
                                      _DAT_005a4380) / _DAT_005a43cc);
        (**(code **)(*piVar1 + 0x70))
                  (0x435a0000,(float)((float10)_DAT_005a34a4 - fVar3 * (float10)_DAT_005a453c));
      }
      else {
        (**(code **)(*piVar1 + 0x70))(0x435a0000,0x43b40000);
        *(undefined4 *)(param_1 + 0x26e7c) = 0;
      }
    }
    if (*(int *)(param_1 + 0x26e80) != 0) {
      piVar1 = *(int **)(param_1 + 0x26e64);
      if ((uint)(iVar2 - *(int *)(param_1 + 0x26e80)) < 300) {
        fVar3 = (float10)FUN_00423b00(((float)(uint)(iVar2 - *(int *)(param_1 + 0x26e80)) *
                                      _DAT_005a4380) / _DAT_005a43c4);
        (**(code **)(*piVar1 + 0x70))
                  (0x435a0000,(float)(fVar3 * (float10)_DAT_005a453c + (float10)_DAT_005a428c));
      }
      else {
        (**(code **)(*piVar1 + 0x70))(0x435a0000,0x44160000);
        *(undefined4 *)(param_1 + 0x26e80) = 0;
      }
    }
  }
  return 1;
}

