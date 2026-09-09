// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00567815 | Name: FUN_00567815


undefined4 __fastcall FUN_00567815(int *param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  undefined4 uVar4;
  uint uVar5;
  int iVar6;
  undefined4 *puVar7;
  undefined4 *puVar8;
  undefined4 *puVar9;
  undefined4 *puVar10;
  int iStack_2c;
  uint local_14;
  undefined4 *local_10;
  uint local_c;
  undefined4 *local_8;
  
  iVar6 = *param_1;
  if (((*(uint *)(iVar6 + 0x103c) | *(uint *)(iVar6 + 0x1038) | *(uint *)(iVar6 + 0x1030) |
       *(uint *)(iVar6 + 0x1034)) & 3) != 0) {
    return 0x80004005;
  }
  iVar1 = param_1[1];
  if (((*(uint *)(iVar1 + 0x103c) | *(uint *)(iVar1 + 0x1038) | *(uint *)(iVar1 + 0x1030) |
       *(uint *)(iVar1 + 0x1034)) & 3) == 0) {
    iVar2 = *(int *)(iVar1 + 4);
    if (iVar2 == 0x31545844) {
      iStack_2c = 8;
    }
    else {
      if ((((iVar2 != 0x32545844) && (iVar2 != 0x33545844)) && (iVar2 != 0x34545844)) &&
         (iVar2 != 0x35545844)) goto LAB_00567878;
      iStack_2c = 0x10;
    }
    uVar3 = *(uint *)(iVar1 + 0x1058);
    local_10 = (undefined4 *)
               ((*(uint *)(iVar1 + 0x1034) >> 2) * *(int *)(iVar1 + 0x1050) +
                *(int *)(iVar1 + 0x1054) * *(int *)(iVar1 + 0x1040) +
                (*(uint *)(iVar1 + 0x1030) >> 2) * iStack_2c + *(int *)(iVar1 + 0x18));
    local_14 = 0;
    puVar8 = (undefined4 *)
             ((*(uint *)(iVar6 + 0x1034) >> 2) * *(int *)(iVar6 + 0x1050) +
              *(int *)(iVar6 + 0x1054) * *(int *)(iVar6 + 0x1040) +
              (*(uint *)(iVar6 + 0x1030) >> 2) * iStack_2c + *(int *)(iVar6 + 0x18));
    if (*(int *)(iVar1 + 0x1060) != 0) {
      uVar5 = *(uint *)(iVar1 + 0x105c);
      do {
        local_c = 0;
        puVar7 = local_10;
        local_8 = puVar8;
        if (uVar5 != 0) {
          do {
            local_c = local_c + 4;
            puVar9 = local_8;
            puVar10 = puVar7;
            for (uVar5 = (uVar3 >> 2) * iStack_2c >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
              *puVar10 = *puVar9;
              puVar9 = puVar9 + 1;
              puVar10 = puVar10 + 1;
            }
            for (iVar6 = 0; iVar6 != 0; iVar6 = iVar6 + -1) {
              *(undefined1 *)puVar10 = *(undefined1 *)puVar9;
              puVar9 = (undefined4 *)((int)puVar9 + 1);
              puVar10 = (undefined4 *)((int)puVar10 + 1);
            }
            puVar7 = (undefined4 *)((int)puVar7 + *(int *)(param_1[1] + 0x1050));
            uVar5 = *(uint *)(param_1[1] + 0x105c);
            local_8 = (undefined4 *)((int)local_8 + *(int *)(*param_1 + 0x1050));
          } while (local_c < uVar5);
        }
        local_10 = (undefined4 *)((int)local_10 + *(int *)(param_1[1] + 0x1054));
        puVar8 = (undefined4 *)((int)puVar8 + *(int *)(*param_1 + 0x1054));
        local_14 = local_14 + 1;
      } while (local_14 < *(uint *)(param_1[1] + 0x1060));
    }
    uVar4 = 0;
  }
  else {
LAB_00567878:
    uVar4 = 0x80004005;
  }
  return uVar4;
}

