// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00584199 | Name: FUN_00584199


void FUN_00584199(void)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  int iVar4;
  undefined4 uVar5;
  int iVar6;
  undefined4 *puVar7;
  int *piVar8;
  undefined4 *puVar9;
  int unaff_EDI;
  int local_24;
  int *local_18;
  int local_14;
  int local_10;
  undefined4 *local_c;
  
  iVar1 = *(int *)(unaff_EDI + 0x114);
  local_10 = 0;
  iVar2 = *(int *)(unaff_EDI + 0x180);
  if (0 < *(int *)(unaff_EDI + 0x20)) {
    piVar8 = (int *)(*(int *)(unaff_EDI + 0xc4) + 0xc);
    local_18 = (int *)(iVar2 + 8);
    do {
      iVar6 = (piVar8[6] * *piVar8) / *(int *)(unaff_EDI + 0x114);
      puVar3 = *(undefined4 **)(local_10 * 4 + *(int *)(iVar2 + 0x38));
      puVar9 = *(undefined4 **)(local_10 * 4 + *(int *)(iVar2 + 0x3c));
      iVar4 = *local_18;
      local_14 = (iVar1 + 2) * iVar6;
      if (0 < local_14) {
        puVar7 = puVar9;
        do {
          uVar5 = *(undefined4 *)((iVar4 - (int)puVar9) + (int)puVar7);
          *puVar7 = uVar5;
          *(undefined4 *)(((int)puVar3 - (int)puVar9) + (int)puVar7) = uVar5;
          puVar7 = puVar7 + 1;
          local_14 = local_14 + -1;
        } while (local_14 != 0);
      }
      if (0 < iVar6 * 2) {
        puVar7 = (undefined4 *)(iVar4 + (iVar1 + -2) * iVar6 * 4);
        local_24 = iVar6 * 2;
        local_c = puVar9 + iVar6 * iVar1;
        do {
          *(undefined4 *)(((int)puVar9 - iVar4) + (int)puVar7) =
               *(undefined4 *)((int)local_c + (iVar4 - (int)puVar9));
          uVar5 = *puVar7;
          puVar7 = puVar7 + 1;
          local_24 = local_24 + -1;
          *local_c = uVar5;
          local_c = local_c + 1;
        } while (local_24 != 0);
      }
      if (0 < iVar6) {
        puVar9 = puVar3 + -iVar6;
        do {
          *puVar9 = *puVar3;
          puVar9 = puVar9 + 1;
          iVar6 = iVar6 + -1;
        } while (iVar6 != 0);
      }
      local_10 = local_10 + 1;
      local_18 = local_18 + 1;
      piVar8 = piVar8 + 0x15;
    } while (local_10 < *(int *)(unaff_EDI + 0x20));
  }
  return;
}

