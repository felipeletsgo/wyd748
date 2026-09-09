// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058492e | Name: FUN_0058492e


undefined4 FUN_0058492e(int param_1)

{
  uint *puVar1;
  int iVar2;
  int iVar3;
  char cVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  uint uVar8;
  undefined4 uVar9;
  int aiStack_38 [4];
  int *local_28;
  int local_24;
  uint local_20;
  int *local_1c;
  int local_18;
  int *local_14;
  int local_10;
  int local_c;
  int *local_8;
  
  iVar3 = param_1;
  param_1 = 0;
  iVar2 = *(int *)(iVar3 + 0x184);
  if (0 < *(int *)(iVar3 + 0x120)) {
    local_8 = (int *)(iVar3 + 0x124);
    do {
      iVar5 = *(int *)(*local_8 + 0xc);
      iVar5 = (**(code **)(*(int *)(iVar3 + 4) + 0x20))
                        (iVar3,*(undefined4 *)(iVar2 + 0x48 + *(int *)(*local_8 + 4) * 4),
                         *(int *)(iVar3 + 0x80) * iVar5,iVar5,1);
      local_8 = local_8 + 1;
      aiStack_38[param_1] = iVar5;
      param_1 = param_1 + 1;
    } while (param_1 < *(int *)(iVar3 + 0x120));
  }
  local_8 = *(int **)(iVar2 + 0x18);
  do {
    if (*(int *)(iVar2 + 0x1c) <= (int)local_8) {
      puVar1 = (uint *)(iVar3 + 0x80);
      *puVar1 = *puVar1 + 1;
      if (*puVar1 < *(uint *)(iVar3 + 0x118)) {
        FUN_00584702();
        uVar9 = 3;
      }
      else {
        (**(code **)(*(int *)(iVar3 + 0x18c) + 0xc))(iVar3);
        uVar9 = 4;
      }
      return uVar9;
    }
    uVar8 = *(uint *)(iVar2 + 0x14);
    local_20 = uVar8;
    if (uVar8 < *(uint *)(iVar3 + 0x134)) {
      do {
        local_c = 0;
        param_1 = 0;
        if (0 < *(int *)(iVar3 + 0x120)) {
          local_1c = (int *)(iVar3 + 0x124);
          do {
            iVar5 = *local_1c;
            iVar7 = *(int *)(iVar5 + 0x34);
            local_18 = 0;
            if (0 < *(int *)(iVar5 + 0x38)) {
              local_24 = iVar7 * uVar8 * 0x80;
              local_14 = (int *)(aiStack_38[param_1] + (int)local_8 * 4);
              do {
                iVar6 = *local_14 + local_24;
                local_10 = 0;
                if (0 < iVar7) {
                  local_28 = (int *)(iVar2 + 0x20 + local_c * 4);
                  do {
                    local_c = local_c + 1;
                    *local_28 = iVar6;
                    local_28 = local_28 + 1;
                    iVar6 = iVar6 + 0x80;
                    local_10 = local_10 + 1;
                    iVar7 = *(int *)(iVar5 + 0x34);
                  } while (local_10 < iVar7);
                }
                local_18 = local_18 + 1;
                local_14 = local_14 + 1;
              } while (local_18 < *(int *)(iVar5 + 0x38));
            }
            param_1 = param_1 + 1;
            local_1c = local_1c + 1;
          } while (param_1 < *(int *)(iVar3 + 0x120));
        }
        local_20 = uVar8;
        cVar4 = (**(code **)(*(int *)(iVar3 + 0x194) + 4))(iVar3,iVar2 + 0x20);
        if (cVar4 == '\0') {
          *(int **)(iVar2 + 0x18) = local_8;
          *(uint *)(iVar2 + 0x14) = uVar8;
          return 0;
        }
        uVar8 = uVar8 + 1;
        local_20 = uVar8;
      } while (uVar8 < *(uint *)(iVar3 + 0x134));
    }
    *(undefined4 *)(iVar2 + 0x14) = 0;
    local_8 = (int *)((int)local_8 + 1);
  } while( true );
}

