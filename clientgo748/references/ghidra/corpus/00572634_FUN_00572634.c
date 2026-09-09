// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00572634 | Name: FUN_00572634


undefined4 FUN_00572634(void)

{
  byte bVar1;
  uint *puVar2;
  char cVar3;
  uint uVar4;
  undefined4 uVar5;
  int *unaff_EBX;
  byte *pbVar6;
  int *piVar7;
  byte *pbVar8;
  int iVar9;
  undefined4 *puVar10;
  byte local_134 [256];
  uint local_34;
  int *local_30;
  uint local_2c;
  uint local_28;
  uint local_24;
  uint local_20;
  byte local_1c;
  int local_18;
  int *local_14;
  int local_10;
  byte *local_c;
  int local_8;
  
  piVar7 = (int *)unaff_EBX[5];
  local_8 = piVar7[1];
  pbVar8 = (byte *)*piVar7;
  local_30 = piVar7;
  if (local_8 == 0) {
    cVar3 = (*(code *)piVar7[3])();
    if (cVar3 != '\0') {
      pbVar8 = (byte *)*piVar7;
      local_8 = piVar7[1];
      local_c = pbVar8;
      goto LAB_0057266c;
    }
LAB_005728c4:
    uVar5 = 0;
  }
  else {
LAB_0057266c:
    local_8 = local_8 + -1;
    local_10 = (uint)*pbVar8 << 8;
    pbVar8 = pbVar8 + 1;
    if (local_8 == 0) {
      cVar3 = (*(code *)piVar7[3])();
      if (cVar3 == '\0') goto LAB_005728c4;
      pbVar8 = (byte *)*piVar7;
      local_8 = piVar7[1];
      local_c = pbVar8;
    }
    local_8 = local_8 + -1;
    local_10 = local_10 + (uint)*pbVar8 + -2;
    pbVar8 = pbVar8 + 1;
    while (0 < local_10) {
      if (local_8 == 0) {
        cVar3 = (*(code *)piVar7[3])();
        if (cVar3 == '\0') goto LAB_005728c4;
        pbVar8 = (byte *)*piVar7;
        local_8 = piVar7[1];
        local_c = pbVar8;
      }
      local_34 = (uint)*pbVar8;
      local_8 = local_8 + -1;
      *(undefined4 *)(*unaff_EBX + 0x14) = 0x4f;
      *(uint *)(*unaff_EBX + 0x18) = local_34;
      pbVar8 = pbVar8 + 1;
      (**(code **)(*unaff_EBX + 4))();
      local_2c = local_2c & 0xffffff00;
      local_18 = 0;
      local_14 = (int *)0x1;
      do {
        if (local_8 == 0) {
          cVar3 = (*(code *)piVar7[3])();
          if (cVar3 == '\0') goto LAB_005728c4;
          pbVar8 = (byte *)*piVar7;
          local_8 = piVar7[1];
        }
        bVar1 = *pbVar8;
        local_8 = local_8 + -1;
        *(byte *)((int)&local_2c + (int)local_14) = bVar1;
        local_18 = local_18 + (uint)bVar1;
        pbVar8 = pbVar8 + 1;
        local_14 = (int *)((int)local_14 + 1);
        local_c = pbVar8;
      } while ((int)local_14 < 0x11);
      iVar9 = *unaff_EBX;
      *(uint *)(iVar9 + 0x18) = local_2c >> 8 & 0xff;
      *(uint *)(iVar9 + 0x1c) = local_2c >> 0x10 & 0xff;
      *(uint *)(iVar9 + 0x20) = local_2c >> 0x18;
      local_10 = local_10 + -0x11;
      *(uint *)(iVar9 + 0x24) = local_28 & 0xff;
      *(uint *)(iVar9 + 0x28) = local_28 >> 8 & 0xff;
      *(uint *)(iVar9 + 0x2c) = local_28 >> 0x10 & 0xff;
      *(uint *)(iVar9 + 0x30) = local_28 >> 0x18;
      *(uint *)(iVar9 + 0x34) = local_24 & 0xff;
      *(undefined4 *)(*unaff_EBX + 0x14) = 0x55;
      (**(code **)(*unaff_EBX + 4))();
      iVar9 = *unaff_EBX;
      *(uint *)(iVar9 + 0x18) = local_24 >> 8 & 0xff;
      *(uint *)(iVar9 + 0x1c) = local_24 >> 0x10 & 0xff;
      *(uint *)(iVar9 + 0x20) = local_24 >> 0x18;
      *(uint *)(iVar9 + 0x24) = local_20 & 0xff;
      *(uint *)(iVar9 + 0x28) = local_20 >> 8 & 0xff;
      *(uint *)(iVar9 + 0x2c) = local_20 >> 0x10 & 0xff;
      *(uint *)(iVar9 + 0x30) = local_20 >> 0x18;
      *(uint *)(iVar9 + 0x34) = (uint)local_1c;
      *(undefined4 *)(*unaff_EBX + 0x14) = 0x55;
      (**(code **)(*unaff_EBX + 4))();
      if ((0x100 < local_18) || (local_10 < local_18)) {
        *(undefined4 *)(*unaff_EBX + 0x14) = 0x1c;
        (**(code **)*unaff_EBX)();
      }
      iVar9 = 0;
      if (0 < local_18) {
        do {
          if (local_8 == 0) {
            cVar3 = (*(code *)piVar7[3])();
            if (cVar3 == '\0') goto LAB_005728c4;
            local_c = (byte *)*piVar7;
            local_8 = piVar7[1];
          }
          local_8 = local_8 + -1;
          bVar1 = *local_c;
          local_c = local_c + 1;
          local_134[iVar9] = bVar1;
          iVar9 = iVar9 + 1;
        } while (iVar9 < local_18);
      }
      local_10 = local_10 - local_18;
      if ((local_34 & 0x10) == 0) {
        piVar7 = unaff_EBX + local_34 + 0x28;
        local_14 = piVar7;
        uVar4 = local_34;
      }
      else {
        piVar7 = unaff_EBX + local_34 + 0x1c;
        local_14 = piVar7;
        uVar4 = local_34 - 0x10;
      }
      local_14 = piVar7;
      if (((int)uVar4 < 0) || (3 < (int)uVar4)) {
        *(undefined4 *)(*unaff_EBX + 0x14) = 0x1d;
        *(uint *)(*unaff_EBX + 0x18) = uVar4;
        (**(code **)*unaff_EBX)();
      }
      if (*piVar7 == 0) {
        iVar9 = FUN_00578eaa();
        *piVar7 = iVar9;
      }
      puVar2 = (uint *)*piVar7;
      *puVar2 = local_2c;
      puVar2[1] = local_28;
      puVar2[2] = local_24;
      puVar2[3] = local_20;
      *(byte *)(puVar2 + 4) = local_1c;
      pbVar6 = local_134;
      puVar10 = (undefined4 *)(*local_14 + 0x11);
      for (iVar9 = 0x40; piVar7 = local_30, pbVar8 = local_c, iVar9 != 0; iVar9 = iVar9 + -1) {
        *puVar10 = *(undefined4 *)pbVar6;
        pbVar6 = pbVar6 + 4;
        puVar10 = puVar10 + 1;
      }
    }
    piVar7[1] = local_8;
    *piVar7 = (int)pbVar8;
    uVar5 = 1;
  }
  return uVar5;
}

