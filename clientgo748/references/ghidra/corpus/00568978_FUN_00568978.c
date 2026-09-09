// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00568978 | Name: FUN_00568978


undefined4 __fastcall FUN_00568978(int *param_1)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  undefined4 *puVar7;
  undefined4 *puVar8;
  int *piVar9;
  undefined4 *puVar10;
  int *piVar11;
  undefined4 *puVar12;
  bool bVar13;
  uint local_14;
  uint local_10;
  uint local_8;
  
  iVar3 = param_1[1];
  iVar1 = *param_1;
  if (*(int *)(iVar3 + 4) != *(int *)(iVar1 + 4)) {
    return 0x80004005;
  }
  if ((((*(int *)(iVar1 + 0x10) == 0) && (*(int *)(iVar3 + 0x1058) == *(int *)(iVar1 + 0x1058))) &&
      (local_14 = *(uint *)(iVar3 + 0x105c), local_14 == *(uint *)(iVar1 + 0x105c))) &&
     (*(int *)(iVar3 + 0x1060) == *(int *)(iVar1 + 0x1060))) {
    if (*(int *)(iVar3 + 0xc) == 0) {
      uVar2 = FUN_00567815();
      return uVar2;
    }
    if (*(int *)(iVar3 + 0x14) != 0) {
      if ((int *)(iVar3 + 0x30) != (int *)(iVar1 + 0x30)) {
        iVar4 = 0x100;
        bVar13 = true;
        piVar9 = (int *)(iVar3 + 0x30);
        piVar11 = (int *)(iVar1 + 0x30);
        do {
          if (iVar4 == 0) break;
          iVar4 = iVar4 + -1;
          bVar13 = *piVar9 == *piVar11;
          piVar9 = piVar9 + 1;
          piVar11 = piVar11 + 1;
        } while (bVar13);
        if (!bVar13) goto LAB_00568aa6;
      }
    }
    local_8 = 0;
    if (*(int *)(iVar3 + 0x1060) != 0) {
      do {
        puVar7 = (undefined4 *)(*(int *)(iVar3 + 0x1054) * local_8 + *(int *)(iVar3 + 0x18));
        puVar8 = (undefined4 *)(*(int *)(*param_1 + 0x1054) * local_8 + *(int *)(*param_1 + 0x18));
        local_10 = 0;
        if (local_14 != 0) {
          do {
            uVar6 = *(uint *)(iVar3 + 0x1064);
            puVar10 = puVar8;
            puVar12 = puVar7;
            for (uVar5 = uVar6 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
              *puVar12 = *puVar10;
              puVar10 = puVar10 + 1;
              puVar12 = puVar12 + 1;
            }
            for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
              *(undefined1 *)puVar12 = *(undefined1 *)puVar10;
              puVar10 = (undefined4 *)((int)puVar10 + 1);
              puVar12 = (undefined4 *)((int)puVar12 + 1);
            }
            iVar3 = param_1[1];
            puVar8 = (undefined4 *)((int)puVar8 + *(int *)(*param_1 + 0x1050));
            puVar7 = (undefined4 *)((int)puVar7 + *(int *)(iVar3 + 0x1050));
            local_14 = *(uint *)(iVar3 + 0x105c);
            local_10 = local_10 + 1;
          } while (local_10 < local_14);
        }
        local_8 = local_8 + 1;
        iVar3 = param_1[1];
      } while (local_8 < *(uint *)(iVar3 + 0x1060));
    }
    uVar2 = 0;
  }
  else {
LAB_00568aa6:
    uVar2 = 0x80004005;
  }
  return uVar2;
}

