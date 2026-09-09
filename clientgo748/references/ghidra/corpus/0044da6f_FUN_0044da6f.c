// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044da6f | Name: FUN_0044da6f


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0044da6f(int param_1)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  int iVar4;
  bool bVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  
  if (DAT_005b892c == 2) {
    if (*(int *)(param_1 + 0x27d8c) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27d8c) + 0x60))
                (*(int *)(*(int *)(param_1 + 0x27d8c) + 0x28) == 0);
    }
  }
  else {
    piVar1 = *(int **)(param_1 + 0x27d8c);
    piVar2 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x74a);
    piVar3 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x129);
    bVar5 = piVar1[10] == 0;
    (**(code **)(*piVar1 + 0x60))(bVar5);
    (**(code **)(*piVar2 + 0x60))(bVar5);
    (**(code **)(*piVar3 + 0x8c))(bVar5);
    if (bVar5) {
      iVar4 = *(int *)(*(int *)(param_1 + 0x2872c) + 0x210);
      (**(code **)(*piVar1 + 0x74))(0x42e40000,(float)iVar4 * _DAT_005a4374 + _DAT_005a4370);
      (**(code **)(*piVar1 + 0x70))(0,_DAT_005a436c - (float)iVar4 * _DAT_005a4374);
      FUN_0040981b(0x42e40000,(float)iVar4 * _DAT_005a4374);
    }
    else if (*(int *)(*(int *)(param_1 + 0x2872c) + 0x210) == 1) {
      *(undefined1 *)(*(int *)(param_1 + 0x4c) + 0x248) = 0;
      FUN_004091ab();
    }
    if ((DAT_005ccf98 != 0) && (iVar4 = FUN_00429a6d(0x33), iVar4 != 0)) {
      uVar7 = 0;
      uVar6 = 0;
      FUN_00429a6d(0x33);
      FUN_0042ad2b(uVar6,uVar7);
    }
  }
  return;
}

