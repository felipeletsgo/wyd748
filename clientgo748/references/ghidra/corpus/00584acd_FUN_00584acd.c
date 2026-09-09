// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00584acd | Name: FUN_00584acd


int FUN_00584acd(uint param_1,int param_2)

{
  uint *puVar1;
  int iVar2;
  code *pcVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  int local_1c;
  undefined4 *local_18;
  int local_14;
  uint local_10;
  int local_c;
  int local_8;
  
  iVar4 = param_1;
  iVar2 = *(int *)(param_1 + 0x118);
  iVar6 = *(int *)(param_1 + 0x184);
  while ((*(int *)(param_1 + 0x7c) < *(int *)(param_1 + 0x84) ||
         ((*(int *)(param_1 + 0x7c) == *(int *)(param_1 + 0x84) &&
          (*(uint *)(param_1 + 0x80) <= *(uint *)(param_1 + 0x88)))))) {
    iVar5 = (*(code *)**(undefined4 **)(param_1 + 0x18c))(param_1);
    if (iVar5 == 0) {
      return 0;
    }
  }
  local_8 = 0;
  iVar5 = *(int *)(param_1 + 0xc4);
  if (0 < *(int *)(param_1 + 0x20)) {
    local_18 = (undefined4 *)(iVar6 + 0x48);
    do {
      if (*(char *)(iVar5 + 0x30) != '\0') {
        iVar6 = (**(code **)(*(int *)(iVar4 + 4) + 0x20))
                          (iVar4,*local_18,*(int *)(iVar4 + 0x88) * *(int *)(iVar5 + 0xc),
                           *(int *)(iVar5 + 0xc),0);
        if (*(uint *)(iVar4 + 0x88) < iVar2 - 1U) {
          param_1 = *(uint *)(iVar5 + 0xc);
        }
        else {
          param_1 = *(uint *)(iVar5 + 0x20) % *(uint *)(iVar5 + 0xc);
          if (param_1 == 0) {
            param_1 = *(uint *)(iVar5 + 0xc);
          }
        }
        pcVar3 = *(code **)(*(int *)(iVar4 + 0x198) + 4 + local_8 * 4);
        local_14 = *(int *)(local_8 * 4 + param_2);
        local_1c = 0;
        if (0 < (int)param_1) {
          uVar7 = *(uint *)(iVar5 + 0x1c);
          do {
            local_c = *(int *)(iVar6 + local_1c * 4);
            iVar8 = 0;
            local_10 = 0;
            if (uVar7 != 0) {
              do {
                (*pcVar3)(iVar4,iVar5,local_c,local_14,iVar8);
                local_c = local_c + 0x80;
                iVar8 = iVar8 + *(int *)(iVar5 + 0x24);
                uVar7 = *(uint *)(iVar5 + 0x1c);
                local_10 = local_10 + 1;
              } while (local_10 < uVar7);
            }
            local_14 = local_14 + *(int *)(iVar5 + 0x24) * 4;
            local_1c = local_1c + 1;
          } while (local_1c < (int)param_1);
        }
      }
      local_8 = local_8 + 1;
      local_18 = local_18 + 1;
      iVar5 = iVar5 + 0x54;
    } while (local_8 < *(int *)(iVar4 + 0x20));
  }
  puVar1 = (uint *)(iVar4 + 0x88);
  *puVar1 = *puVar1 + 1;
  return 4 - (uint)(*puVar1 < *(uint *)(iVar4 + 0x118));
}

