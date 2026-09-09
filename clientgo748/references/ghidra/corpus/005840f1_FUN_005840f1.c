// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005840f1 | Name: FUN_005840f1


void FUN_005840f1(void)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int *piVar5;
  int iVar6;
  int unaff_ESI;
  int local_8;
  
  iVar1 = *(int *)(unaff_ESI + 0x180);
  iVar2 = *(int *)(unaff_ESI + 0x114);
  iVar3 = (*(code *)**(undefined4 **)(unaff_ESI + 4))();
  local_8 = 0;
  *(int *)(iVar1 + 0x38) = iVar3;
  *(int *)(iVar1 + 0x3c) = iVar3 + *(int *)(unaff_ESI + 0x20) * 4;
  if (0 < *(int *)(unaff_ESI + 0x20)) {
    piVar5 = (int *)(*(int *)(unaff_ESI + 0xc4) + 0xc);
    do {
      iVar4 = (piVar5[6] * *piVar5) / *(int *)(unaff_ESI + 0x114);
      iVar3 = (*(code *)**(undefined4 **)(unaff_ESI + 4))();
      piVar5 = piVar5 + 0x15;
      iVar3 = iVar3 + iVar4 * 4;
      iVar6 = local_8 * 4;
      *(int *)(iVar6 + *(int *)(iVar1 + 0x38)) = iVar3;
      local_8 = local_8 + 1;
      *(int *)(iVar6 + *(int *)(iVar1 + 0x3c)) = iVar3 + (iVar2 + 4) * iVar4 * 4;
    } while (local_8 < *(int *)(unaff_ESI + 0x20));
  }
  return;
}

