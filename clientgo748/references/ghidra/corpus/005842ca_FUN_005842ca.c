// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005842ca | Name: FUN_005842ca


void FUN_005842ca(void)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  int *piVar6;
  undefined4 *puVar7;
  int iVar8;
  int unaff_EDI;
  int local_10;
  int local_c;
  undefined4 *local_8;
  
  iVar1 = *(int *)(unaff_EDI + 0x180);
  local_c = 0;
  iVar2 = *(int *)(unaff_EDI + 0x114);
  if (0 < *(int *)(unaff_EDI + 0x20)) {
    piVar6 = (int *)(*(int *)(unaff_EDI + 0xc4) + 0xc);
    do {
      local_10 = (piVar6[6] * *piVar6) / *(int *)(unaff_EDI + 0x114);
      local_8 = *(undefined4 **)(local_c * 4 + *(int *)(iVar1 + 0x3c));
      if (0 < local_10) {
        puVar7 = local_8 + (iVar2 + 2) * local_10;
        puVar5 = local_8 + -local_10;
        puVar4 = local_8 + (iVar2 + 1) * local_10;
        iVar8 = *(int *)(local_c * 4 + *(int *)(iVar1 + 0x38)) - (int)local_8;
        do {
          *(undefined4 *)(iVar8 + (int)puVar5) = *(undefined4 *)(iVar8 + (int)puVar4);
          *puVar5 = *puVar4;
          *(undefined4 *)(iVar8 + (int)puVar7) = *(undefined4 *)(iVar8 + (int)local_8);
          uVar3 = *local_8;
          local_8 = local_8 + 1;
          *puVar7 = uVar3;
          puVar4 = puVar4 + 1;
          puVar5 = puVar5 + 1;
          puVar7 = puVar7 + 1;
          local_10 = local_10 + -1;
        } while (local_10 != 0);
      }
      local_c = local_c + 1;
      piVar6 = piVar6 + 0x15;
    } while (local_c < *(int *)(unaff_EDI + 0x20));
  }
  return;
}

