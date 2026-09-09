// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005780ca | Name: FUN_005780ca


uint FUN_005780ca(void)

{
  uint uVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  int *unaff_ESI;
  int iVar5;
  int *local_c;
  uint local_8;
  
  iVar2 = unaff_ESI[0x48];
  if (iVar2 == 1) {
    iVar2 = unaff_ESI[0x49];
    unaff_ESI[0x4d] = *(int *)(iVar2 + 0x1c);
    unaff_ESI[0x4e] = *(int *)(iVar2 + 0x20);
    uVar4 = *(uint *)(iVar2 + 0xc);
    *(undefined4 *)(iVar2 + 0x40) = *(undefined4 *)(iVar2 + 0x24);
    uVar1 = *(uint *)(iVar2 + 0x20) / uVar4;
    uVar3 = *(uint *)(iVar2 + 0x20) % uVar4;
    *(undefined4 *)(iVar2 + 0x34) = 1;
    *(undefined4 *)(iVar2 + 0x38) = 1;
    *(undefined4 *)(iVar2 + 0x3c) = 1;
    *(undefined4 *)(iVar2 + 0x44) = 1;
    if (uVar3 == 0) {
      uVar3 = uVar4;
    }
    *(uint *)(iVar2 + 0x48) = uVar3;
    unaff_ESI[0x50] = 0;
    unaff_ESI[0x4f] = 1;
  }
  else {
    if ((iVar2 < 1) || (4 < iVar2)) {
      *(undefined4 *)(*unaff_ESI + 0x14) = 0x18;
      *(int *)(*unaff_ESI + 0x18) = unaff_ESI[0x48];
      *(undefined4 *)(*unaff_ESI + 0x1c) = 4;
      (**(code **)*unaff_ESI)();
    }
    iVar2 = FUN_00577e31(unaff_ESI[6],unaff_ESI[0x43] << 3);
    unaff_ESI[0x4d] = iVar2;
    uVar1 = FUN_00577e31(unaff_ESI[7],unaff_ESI[0x44] << 3);
    unaff_ESI[0x4e] = uVar1;
    unaff_ESI[0x4f] = 0;
    local_8 = 0;
    if (0 < unaff_ESI[0x48]) {
      local_c = unaff_ESI + 0x49;
      do {
        iVar2 = *local_c;
        uVar1 = *(uint *)(iVar2 + 8);
        *(uint *)(iVar2 + 0x40) = *(int *)(iVar2 + 0x24) * uVar1;
        uVar4 = *(uint *)(iVar2 + 0x1c) % uVar1;
        *(int *)(iVar2 + 0x38) = *(int *)(iVar2 + 0xc);
        iVar5 = *(int *)(iVar2 + 0xc) * uVar1;
        *(uint *)(iVar2 + 0x34) = uVar1;
        *(int *)(iVar2 + 0x3c) = iVar5;
        if (uVar4 == 0) {
          uVar4 = uVar1;
        }
        *(uint *)(iVar2 + 0x44) = uVar4;
        uVar1 = *(uint *)(iVar2 + 0x20) % *(uint *)(iVar2 + 0xc);
        if (uVar1 == 0) {
          uVar1 = *(uint *)(iVar2 + 0xc);
        }
        *(uint *)(iVar2 + 0x48) = uVar1;
        if (10 < unaff_ESI[0x4f] + iVar5) {
          *(undefined4 *)(*unaff_ESI + 0x14) = 0xb;
          (**(code **)*unaff_ESI)();
        }
        if (0 < iVar5) {
          do {
            unaff_ESI[unaff_ESI[0x4f] + 0x50] = local_8;
            unaff_ESI[0x4f] = unaff_ESI[0x4f] + 1;
            iVar5 = iVar5 + -1;
          } while (iVar5 != 0);
        }
        uVar1 = local_8 + 1;
        local_c = local_c + 1;
        local_8 = uVar1;
      } while ((int)uVar1 < unaff_ESI[0x48]);
    }
  }
  return uVar1;
}

