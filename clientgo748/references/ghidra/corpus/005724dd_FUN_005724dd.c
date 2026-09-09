// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005724dd | Name: FUN_005724dd


uint FUN_005724dd(void)

{
  byte bVar1;
  undefined4 *puVar2;
  uint uVar3;
  uint uVar4;
  byte bVar5;
  int *unaff_ESI;
  byte *pbVar6;
  byte *pbVar7;
  int local_c;
  int local_8;
  
  puVar2 = (undefined4 *)unaff_ESI[5];
  local_8 = puVar2[1];
  pbVar6 = (byte *)*puVar2;
  if (local_8 == 0) {
    uVar3 = (*(code *)puVar2[3])();
    if ((char)uVar3 != '\0') {
      local_8 = puVar2[1];
      pbVar6 = (byte *)*puVar2;
      goto LAB_00572509;
    }
LAB_00572630:
    uVar3 = uVar3 & 0xffffff00;
  }
  else {
LAB_00572509:
    local_8 = local_8 + -1;
    bVar1 = *pbVar6;
    pbVar6 = pbVar6 + 1;
    if (local_8 == 0) {
      uVar3 = (*(code *)puVar2[3])();
      if ((char)uVar3 == '\0') goto LAB_00572630;
      local_8 = puVar2[1];
      pbVar6 = (byte *)*puVar2;
    }
    local_8 = local_8 + -1;
    pbVar7 = pbVar6 + 1;
    local_c = (uint)bVar1 * 0x100 + (uint)*pbVar6 + -2;
    while (0 < local_c) {
      if (local_8 == 0) {
        uVar3 = (*(code *)puVar2[3])();
        if ((char)uVar3 == '\0') goto LAB_00572630;
        local_8 = puVar2[1];
        pbVar7 = (byte *)*puVar2;
      }
      uVar4 = (uint)*pbVar7;
      local_8 = local_8 + -1;
      pbVar7 = pbVar7 + 1;
      if (local_8 == 0) {
        uVar3 = (*(code *)puVar2[3])();
        if ((char)uVar3 == '\0') goto LAB_00572630;
        local_8 = puVar2[1];
        pbVar7 = (byte *)*puVar2;
      }
      bVar1 = *pbVar7;
      uVar3 = (uint)bVar1;
      local_8 = local_8 + -1;
      local_c = local_c + -2;
      *(undefined4 *)(*unaff_ESI + 0x14) = 0x4e;
      *(uint *)(*unaff_ESI + 0x18) = uVar4;
      *(uint *)(*unaff_ESI + 0x1c) = uVar3;
      pbVar7 = pbVar7 + 1;
      (**(code **)(*unaff_ESI + 4))();
      if (0x1f < uVar4) {
        *(undefined4 *)(*unaff_ESI + 0x14) = 0x1a;
        *(uint *)(*unaff_ESI + 0x18) = uVar4;
        (**(code **)*unaff_ESI)();
      }
      if (uVar4 < 0x10) {
        bVar5 = (byte)((int)uVar3 >> 4);
        *(byte *)(uVar4 + 0xca + (int)unaff_ESI) = bVar1 & 0xf;
        *(byte *)(uVar4 + 0xda + (int)unaff_ESI) = bVar5;
        if (bVar5 < (bVar1 & 0xf)) {
          *(undefined4 *)(*unaff_ESI + 0x14) = 0x1b;
          *(uint *)(*unaff_ESI + 0x18) = uVar3;
          (**(code **)*unaff_ESI)();
        }
      }
      else {
        *(byte *)(uVar4 + 0xda + (int)unaff_ESI) = bVar1;
      }
    }
    puVar2[1] = local_8;
    *puVar2 = pbVar7;
    uVar3 = CONCAT31((int3)((uint)local_8 >> 8),1);
  }
  return uVar3;
}

