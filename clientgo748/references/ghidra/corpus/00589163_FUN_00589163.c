// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00589163 | Name: FUN_00589163


void FUN_00589163(int param_1)

{
  int *piVar1;
  int iVar2;
  undefined1 *puVar3;
  undefined4 uVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  undefined1 *puVar8;
  int iVar9;
  int local_1c;
  char local_18;
  int *local_14;
  int local_10;
  int local_c;
  int local_8;
  
  iVar2 = *(int *)(param_1 + 0x1a4);
  if (*(int *)(param_1 + 0x4c) == 1) {
    local_10 = 0x1fe;
    *(undefined1 *)(iVar2 + 0x1c) = 1;
  }
  else {
    local_10 = 0;
    *(undefined1 *)(iVar2 + 0x1c) = 0;
  }
  uVar4 = (**(code **)(*(int *)(param_1 + 4) + 8))
                    (param_1,1,local_10 + 0x100,*(undefined4 *)(param_1 + 100));
  *(undefined4 *)(iVar2 + 0x18) = uVar4;
  iVar5 = *(int *)(iVar2 + 0x14);
  local_1c = 0;
  if (0 < *(int *)(param_1 + 100)) {
    local_14 = (int *)(iVar2 + 0x20);
    do {
      iVar9 = *local_14;
      iVar5 = iVar5 / iVar9;
      if (local_10 != 0) {
        piVar1 = (int *)(*(int *)(iVar2 + 0x18) + local_1c * 4);
        *piVar1 = *piVar1 + 0xff;
      }
      puVar3 = *(undefined1 **)(*(int *)(iVar2 + 0x18) + local_1c * 4);
      iVar6 = (iVar9 + 0xfe) / (iVar9 * 2 + -2);
      local_c = 0;
      local_8 = 0;
      do {
        if (iVar6 < local_8) {
          iVar7 = local_c * 0x1fe;
          do {
            iVar6 = (iVar7 + 0x2fc + iVar9) / (iVar9 * 2 + -2);
            local_c = local_c + 1;
            iVar7 = iVar7 + 0x1fe;
          } while (iVar6 < local_8);
        }
        local_18 = (char)iVar5;
        iVar7 = local_8 + 1;
        puVar3[local_8] = (char)local_c * local_18;
        local_8 = iVar7;
      } while (iVar7 < 0x100);
      if (local_10 != 0) {
        iVar9 = 1;
        puVar8 = puVar3;
        do {
          puVar8 = puVar8 + -1;
          *puVar8 = *puVar3;
          (puVar3 + 0xff)[iVar9] = puVar3[0xff];
          iVar9 = iVar9 + 1;
        } while (iVar9 < 0x100);
      }
      local_14 = local_14 + 1;
      local_1c = local_1c + 1;
    } while (local_1c < *(int *)(param_1 + 100));
  }
  return;
}

