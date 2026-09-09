// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057a1e9 | Name: FUN_0057a1e9


void FUN_0057a1e9(int param_1)

{
  char cVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  undefined4 uVar5;
  int iVar6;
  
  *(undefined4 *)(param_1 + 0x68) = 0;
  FUN_005752f3(param_1);
  uVar3 = *(uint *)(param_1 + 0xbc);
  if (*(char *)(param_1 + 0x113) == '\0') {
    iVar6 = *(int *)(param_1 + 0xb8);
    *(uint *)(param_1 + 0xc0) = uVar3;
    *(int *)(param_1 + 0xd0) = iVar6;
    *(int *)(param_1 + 0xcc) = *(int *)(param_1 + 200) + 1;
  }
  else {
    if ((*(byte *)(param_1 + 0x60) & 2) == 0) {
      uVar3 = uVar3 + 7 >> 3;
    }
    iVar6 = *(int *)(param_1 + 0xb8);
    *(uint *)(param_1 + 0xc0) = uVar3;
    iVar4 = (uint)*(byte *)(param_1 + 0x114) * 4;
    uVar3 = ((iVar6 - *(int *)(&DAT_005a8aa0 + iVar4)) + -1 + *(uint *)(&DAT_005a8abc + iVar4)) /
            *(uint *)(&DAT_005a8abc + iVar4);
    *(uint *)(param_1 + 0xd0) = uVar3;
    *(uint *)(param_1 + 0xcc) = (*(byte *)(param_1 + 0x119) * uVar3 + 7 >> 3) + 1;
  }
  uVar3 = (uint)*(byte *)(param_1 + 0x119);
  uVar2 = *(uint *)(param_1 + 0x60);
  if (((uVar2 & 4) != 0) && (*(byte *)(param_1 + 0x117) < 8)) {
    uVar3 = 8;
  }
  if ((uVar2 & 0x1000) != 0) {
    cVar1 = *(char *)(param_1 + 0x116);
    if (cVar1 == '\x03') {
      uVar3 = (uint)(*(short *)(param_1 + 0x10a) != 0) * 8 + 0x18;
    }
    else if (cVar1 == '\0') {
      if (uVar3 < 8) {
        uVar3 = 8;
      }
      if (*(short *)(param_1 + 0x10a) != 0) {
        uVar3 = uVar3 * 2;
      }
    }
    else if ((cVar1 == '\x02') && (*(short *)(param_1 + 0x10a) != 0)) {
      uVar3 = (uVar3 << 2) / 3;
    }
  }
  if ((char)(uVar2 >> 8) < '\0') {
    cVar1 = *(char *)(param_1 + 0x116);
    if (cVar1 == '\x03') {
      uVar3 = 0x20;
    }
    else if (cVar1 == '\0') {
      uVar3 = ((8 < uVar3) - 1 & 0xfffffff0) + 0x20;
    }
    else if (cVar1 == '\x02') {
      uVar3 = ((0x20 < uVar3) - 1 & 0xffffffe0) + 0x40;
    }
  }
  uVar5 = FUN_005799ef(param_1,((iVar6 + 7U & 0xfffffff8) * uVar3 + 7 >> 3) + 1 +
                               ((int)(uVar3 + 7) >> 3));
  *(undefined4 *)(param_1 + 0xdc) = uVar5;
  uVar5 = FUN_005799ef(param_1,*(int *)(param_1 + 200) + 1);
  *(undefined4 *)(param_1 + 0xd8) = uVar5;
  FUN_00579a61(param_1,uVar5,0,*(int *)(param_1 + 200) + 1);
  *(uint *)(param_1 + 0x5c) = *(uint *)(param_1 + 0x5c) | 0x40;
  return;
}

