// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00571b7a | Name: FUN_00571b7a


uint __thiscall FUN_00571b7a(undefined1 param_1,undefined1 param_2)

{
  byte bVar1;
  byte bVar2;
  undefined4 *puVar3;
  uint uVar4;
  int iVar5;
  uint *puVar6;
  int *unaff_ESI;
  byte *pbVar7;
  byte *pbVar8;
  uint local_c;
  int local_8;
  
  puVar3 = (undefined4 *)unaff_ESI[5];
  local_8 = puVar3[1];
  pbVar7 = (byte *)*puVar3;
  *(undefined1 *)(unaff_ESI + 0x32) = param_1;
  *(undefined1 *)((int)unaff_ESI + 0xc9) = param_2;
  if (local_8 == 0) {
    uVar4 = (*(code *)puVar3[3])();
    if ((char)uVar4 != '\0') {
      local_8 = puVar3[1];
      pbVar7 = (byte *)*puVar3;
      goto LAB_00571bb4;
    }
LAB_00571e6e:
    uVar4 = uVar4 & 0xffffff00;
  }
  else {
LAB_00571bb4:
    local_8 = local_8 + -1;
    bVar1 = *pbVar7;
    pbVar7 = pbVar7 + 1;
    if (local_8 == 0) {
      uVar4 = (*(code *)puVar3[3])();
      if ((char)uVar4 == '\0') goto LAB_00571e6e;
      local_8 = puVar3[1];
      pbVar7 = (byte *)*puVar3;
    }
    bVar2 = *pbVar7;
    local_8 = local_8 + -1;
    pbVar7 = pbVar7 + 1;
    if (local_8 == 0) {
      uVar4 = (*(code *)puVar3[3])();
      if ((char)uVar4 == '\0') goto LAB_00571e6e;
      local_8 = puVar3[1];
      pbVar7 = (byte *)*puVar3;
    }
    local_8 = local_8 + -1;
    pbVar8 = pbVar7 + 1;
    unaff_ESI[0x30] = (uint)*pbVar7;
    if (local_8 == 0) {
      uVar4 = (*(code *)puVar3[3])();
      if ((char)uVar4 == '\0') goto LAB_00571e6e;
      local_8 = puVar3[1];
      pbVar8 = (byte *)*puVar3;
    }
    local_8 = local_8 + -1;
    pbVar7 = pbVar8 + 1;
    unaff_ESI[7] = (uint)*pbVar8 << 8;
    if (local_8 == 0) {
      uVar4 = (*(code *)puVar3[3])();
      if ((char)uVar4 == '\0') goto LAB_00571e6e;
      local_8 = puVar3[1];
      pbVar7 = (byte *)*puVar3;
    }
    local_8 = local_8 + -1;
    unaff_ESI[7] = unaff_ESI[7] + (uint)*pbVar7;
    pbVar7 = pbVar7 + 1;
    if (local_8 == 0) {
      uVar4 = (*(code *)puVar3[3])();
      if ((char)uVar4 == '\0') goto LAB_00571e6e;
      local_8 = puVar3[1];
      pbVar7 = (byte *)*puVar3;
    }
    local_8 = local_8 + -1;
    pbVar8 = pbVar7 + 1;
    unaff_ESI[6] = (uint)*pbVar7 << 8;
    if (local_8 == 0) {
      uVar4 = (*(code *)puVar3[3])();
      if ((char)uVar4 == '\0') goto LAB_00571e6e;
      local_8 = puVar3[1];
      pbVar8 = (byte *)*puVar3;
    }
    local_8 = local_8 + -1;
    unaff_ESI[6] = unaff_ESI[6] + (uint)*pbVar8;
    pbVar8 = pbVar8 + 1;
    if (local_8 == 0) {
      uVar4 = (*(code *)puVar3[3])();
      if ((char)uVar4 == '\0') goto LAB_00571e6e;
      local_8 = puVar3[1];
      pbVar8 = (byte *)*puVar3;
    }
    local_8 = local_8 + -1;
    unaff_ESI[8] = (uint)*pbVar8;
    iVar5 = *unaff_ESI;
    *(int *)(iVar5 + 0x18) = unaff_ESI[0x5e];
    *(int *)(iVar5 + 0x1c) = unaff_ESI[6];
    *(int *)(iVar5 + 0x20) = unaff_ESI[7];
    *(int *)(iVar5 + 0x24) = unaff_ESI[8];
    *(undefined4 *)(*unaff_ESI + 0x14) = 99;
    pbVar8 = pbVar8 + 1;
    (**(code **)(*unaff_ESI + 4))();
    if (*(char *)(unaff_ESI[100] + 0x51) != '\0') {
      *(undefined4 *)(*unaff_ESI + 0x14) = 0x39;
      (**(code **)*unaff_ESI)();
    }
    if (((unaff_ESI[7] == 0) || (unaff_ESI[6] == 0)) || (unaff_ESI[8] < 1)) {
      *(undefined4 *)(*unaff_ESI + 0x14) = 0x1f;
      (**(code **)*unaff_ESI)();
    }
    if ((uint)bVar1 * 0x100 + (uint)bVar2 + -8 != unaff_ESI[8] * 3) {
      *(undefined4 *)(*unaff_ESI + 0x14) = 9;
      (**(code **)*unaff_ESI)();
    }
    if (unaff_ESI[0x31] == 0) {
      iVar5 = (**(code **)unaff_ESI[1])();
      unaff_ESI[0x31] = iVar5;
    }
    local_c = 0;
    puVar6 = (uint *)unaff_ESI[0x31];
    if (0 < unaff_ESI[8]) {
      do {
        puVar6[1] = local_c;
        if (local_8 == 0) {
          uVar4 = (*(code *)puVar3[3])();
          if ((char)uVar4 == '\0') goto LAB_00571e6e;
          local_8 = puVar3[1];
          pbVar8 = (byte *)*puVar3;
        }
        local_8 = local_8 + -1;
        pbVar7 = pbVar8 + 1;
        *puVar6 = (uint)*pbVar8;
        if (local_8 == 0) {
          uVar4 = (*(code *)puVar3[3])();
          if ((char)uVar4 == '\0') goto LAB_00571e6e;
          local_8 = puVar3[1];
          pbVar7 = (byte *)*puVar3;
        }
        bVar1 = *pbVar7;
        local_8 = local_8 + -1;
        pbVar7 = pbVar7 + 1;
        puVar6[2] = (int)(uint)bVar1 >> 4;
        puVar6[3] = bVar1 & 0xf;
        if (local_8 == 0) {
          uVar4 = (*(code *)puVar3[3])();
          if ((char)uVar4 == '\0') goto LAB_00571e6e;
          local_8 = puVar3[1];
          pbVar7 = (byte *)*puVar3;
        }
        local_8 = local_8 + -1;
        puVar6[4] = (uint)*pbVar7;
        iVar5 = *unaff_ESI;
        *(uint *)(iVar5 + 0x18) = *puVar6;
        *(uint *)(iVar5 + 0x1c) = puVar6[2];
        *(uint *)(iVar5 + 0x20) = puVar6[3];
        *(uint *)(iVar5 + 0x24) = puVar6[4];
        *(undefined4 *)(*unaff_ESI + 0x14) = 100;
        pbVar8 = pbVar7 + 1;
        (**(code **)(*unaff_ESI + 4))();
        local_c = local_c + 1;
        puVar6 = puVar6 + 0x15;
      } while ((int)local_c < unaff_ESI[8]);
    }
    *(undefined1 *)(unaff_ESI[100] + 0x51) = 1;
    puVar3[1] = local_8;
    *puVar3 = pbVar8;
    uVar4 = CONCAT31((int3)((uint)local_8 >> 8),1);
  }
  return uVar4;
}

