// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00465b05 | Name: FUN_00465b05


void __fastcall FUN_00465b05(int param_1)

{
  undefined2 uVar1;
  undefined2 uVar2;
  char cVar3;
  undefined2 uVar4;
  undefined4 uVar5;
  int *piVar6;
  char *pcVar7;
  size_t sVar8;
  undefined4 uVar9;
  undefined4 *puVar10;
  int local_20;
  
  uVar5 = (**(code **)(*DAT_0092e654 + 8))();
  piVar6 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x273);
  pcVar7 = (char *)(**(code **)(*piVar6 + 0x88))();
  sVar8 = _strlen(pcVar7);
  if (sVar8 < 4) {
    FUN_00403df2(&DAT_00a348f8,2000);
    (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
  }
  else {
    pcVar7 = (char *)(**(code **)(*piVar6 + 0x88))();
    sVar8 = _strlen(pcVar7);
    if (sVar8 < 0xd) {
      uVar9 = (**(code **)(*piVar6 + 0x88))();
      cVar3 = FUN_0054eee7(uVar9);
      if (cVar3 == '\0') {
        FUN_00403df2(&DAT_00a349f8,2000);
        (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
      }
      else {
        uVar9 = (**(code **)(*piVar6 + 0x88))();
        pcVar7 = (char *)FUN_004b17e2(uVar9);
        sVar8 = _strlen(pcVar7);
        for (local_20 = 0; local_20 < (int)(sVar8 - 1); local_20 = local_20 + 1) {
          if ((pcVar7[local_20] == -0x5f) && (pcVar7[local_20 + 1] == -0x5f)) {
            FUN_00403df2(&DAT_00a349f8,2000);
            (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
            return;
          }
        }
        uVar1 = *(undefined2 *)(param_1 + 0x271b8);
        uVar2 = *(undefined2 *)(param_1 + 0x271ba);
        uVar4 = __ftol();
        *(undefined2 *)(param_1 + 0x271b8) = uVar4;
        uVar4 = __ftol();
        *(undefined2 *)(param_1 + 0x271ba) = uVar4;
        uVar9 = (**(code **)(*piVar6 + 0x88))();
        FUN_0058f078(param_1 + 0x271be,uVar9);
        FUN_0055f2dd(param_1 + 0x2719c,0x34);
        *(undefined4 *)(param_1 + 0x27478) = uVar5;
        *(undefined4 *)(DAT_005d0504 + 0x43c) = 1;
        puVar10 = (undefined4 *)(**(code **)(**(int **)(param_1 + 0x2873c) + 0xa4))(uVar1,uVar2);
        if ((*(int *)(DAT_005ccec0 + 0x1e8) != 0) &&
           (*(undefined4 **)(DAT_005ccec0 + 0x1e8) == puVar10)) {
          *(undefined4 *)(DAT_005ccec0 + 0x1e8) = 0;
        }
        if ((puVar10 != (undefined4 *)0x0) && (puVar10 != (undefined4 *)0x0)) {
          (**(code **)*puVar10)(1);
        }
        FUN_004431e4(0);
        _memset((void *)(DAT_013b71e8 + 0x7c8 + *(int *)(param_1 + 0x271ac) * 8),0,8);
      }
    }
    else {
      FUN_00403df2(&DAT_00a34978,2000);
      (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
    }
  }
  return;
}

