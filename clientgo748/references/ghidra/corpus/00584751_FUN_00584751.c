// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00584751 | Name: FUN_00584751


undefined4 FUN_00584751(int param_1,int param_2)

{
  uint *puVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  code *pcVar6;
  int iVar7;
  char cVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  uint uVar12;
  undefined4 uVar13;
  int local_30;
  int *local_28;
  int local_20;
  int local_1c;
  undefined4 *local_18;
  int local_14;
  int local_10;
  int local_c;
  
  iVar2 = *(int *)(param_1 + 0x118);
  iVar3 = *(int *)(param_1 + 0x184);
  uVar12 = *(int *)(param_1 + 0x134) - 1;
  iVar10 = *(int *)(iVar3 + 0x18);
  do {
    if (*(int *)(iVar3 + 0x1c) <= iVar10) {
      *(int *)(param_1 + 0x88) = *(int *)(param_1 + 0x88) + 1;
      puVar1 = (uint *)(param_1 + 0x80);
      *puVar1 = *puVar1 + 1;
      if (*puVar1 < *(uint *)(param_1 + 0x118)) {
        FUN_00584702();
        uVar13 = 3;
      }
      else {
        (**(code **)(*(int *)(param_1 + 0x18c) + 0xc))(param_1);
        uVar13 = 4;
      }
      return uVar13;
    }
    for (uVar4 = *(uint *)(iVar3 + 0x14); uVar4 <= uVar12; uVar4 = uVar4 + 1) {
      FUN_00577ec2(*(undefined4 *)(iVar3 + 0x20),*(int *)(param_1 + 0x13c) << 7);
      cVar8 = (**(code **)(*(int *)(param_1 + 0x194) + 4))(param_1,iVar3 + 0x20);
      if (cVar8 == '\0') {
        *(int *)(iVar3 + 0x18) = iVar10;
        *(uint *)(iVar3 + 0x14) = uVar4;
        return 0;
      }
      iVar9 = 0;
      local_1c = 0;
      local_10 = 0;
      if (0 < *(int *)(param_1 + 0x120)) {
        local_28 = (int *)(param_1 + 0x124);
        do {
          iVar5 = *local_28;
          if (*(char *)(iVar5 + 0x30) == '\0') {
            iVar9 = iVar9 + *(int *)(iVar5 + 0x3c);
            local_1c = iVar9;
          }
          else {
            pcVar6 = *(code **)(*(int *)(param_1 + 0x198) + 4 + *(int *)(iVar5 + 4) * 4);
            if (uVar4 < uVar12) {
              iVar11 = *(int *)(iVar5 + 0x34);
            }
            else {
              iVar11 = *(int *)(iVar5 + 0x44);
            }
            iVar7 = *(int *)(iVar5 + 0x40);
            local_c = 0;
            local_20 = *(int *)(param_2 + local_10 * 4) + *(int *)(iVar5 + 0x24) * iVar10 * 4;
            if (0 < *(int *)(iVar5 + 0x38)) {
              do {
                if (((*(uint *)(param_1 + 0x80) < iVar2 - 1U) ||
                    (local_c + iVar10 < *(int *)(iVar5 + 0x48))) && (0 < iVar11)) {
                  local_18 = (undefined4 *)(iVar3 + 0x20 + iVar9 * 4);
                  local_30 = iVar11;
                  local_14 = iVar7 * uVar4;
                  do {
                    (*pcVar6)(param_1,iVar5,*local_18,local_20,local_14);
                    local_14 = local_14 + *(int *)(iVar5 + 0x24);
                    local_18 = local_18 + 1;
                    local_30 = local_30 + -1;
                    iVar9 = local_1c;
                  } while (local_30 != 0);
                }
                iVar9 = iVar9 + *(int *)(iVar5 + 0x34);
                local_c = local_c + 1;
                local_20 = local_20 + *(int *)(iVar5 + 0x24) * 4;
                local_1c = iVar9;
              } while (local_c < *(int *)(iVar5 + 0x38));
            }
          }
          local_10 = local_10 + 1;
          local_28 = local_28 + 1;
        } while (local_10 < *(int *)(param_1 + 0x120));
      }
    }
    *(undefined4 *)(iVar3 + 0x14) = 0;
    iVar10 = iVar10 + 1;
  } while( true );
}

