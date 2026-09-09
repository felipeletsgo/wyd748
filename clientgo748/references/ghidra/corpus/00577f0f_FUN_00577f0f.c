// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00577f0f | Name: FUN_00577f0f


void FUN_00577f0f(void)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int *unaff_ESI;
  int *piVar4;
  undefined4 *puVar5;
  
  if ((0xffdc < unaff_ESI[7]) || (0xffdc < unaff_ESI[6])) {
    *(undefined4 *)(*unaff_ESI + 0x14) = 0x28;
    *(undefined4 *)(*unaff_ESI + 0x18) = 0xffdc;
    (**(code **)*unaff_ESI)();
  }
  if (unaff_ESI[0x30] != 8) {
    *(undefined4 *)(*unaff_ESI + 0x14) = 0xd;
    *(int *)(*unaff_ESI + 0x18) = unaff_ESI[0x30];
    (**(code **)*unaff_ESI)();
  }
  if (10 < unaff_ESI[8]) {
    *(undefined4 *)(*unaff_ESI + 0x14) = 0x18;
    *(int *)(*unaff_ESI + 0x18) = unaff_ESI[8];
    *(undefined4 *)(*unaff_ESI + 0x1c) = 10;
    (**(code **)*unaff_ESI)();
  }
  iVar3 = 0;
  unaff_ESI[0x43] = 1;
  unaff_ESI[0x44] = 1;
  if (0 < unaff_ESI[8]) {
    piVar4 = (int *)(unaff_ESI[0x31] + 0xc);
    do {
      if ((((piVar4[-1] < 1) || (4 < piVar4[-1])) || (*piVar4 < 1)) || (4 < *piVar4)) {
        *(undefined4 *)(*unaff_ESI + 0x14) = 0x10;
        (**(code **)*unaff_ESI)();
      }
      iVar1 = unaff_ESI[0x43];
      if (unaff_ESI[0x43] <= piVar4[-1]) {
        iVar1 = piVar4[-1];
      }
      unaff_ESI[0x43] = iVar1;
      iVar1 = unaff_ESI[0x44];
      if (unaff_ESI[0x44] <= *piVar4) {
        iVar1 = *piVar4;
      }
      iVar3 = iVar3 + 1;
      unaff_ESI[0x44] = iVar1;
      piVar4 = piVar4 + 0x15;
    } while (iVar3 < unaff_ESI[8]);
  }
  iVar3 = 0;
  unaff_ESI[0x45] = 8;
  if (0 < unaff_ESI[8]) {
    puVar5 = (undefined4 *)(unaff_ESI[0x31] + 0x1c);
    do {
      puVar5[2] = 8;
      uVar2 = FUN_00577e31(puVar5[-5] * unaff_ESI[6],unaff_ESI[0x43] << 3);
      *puVar5 = uVar2;
      uVar2 = FUN_00577e31(puVar5[-4] * unaff_ESI[7],unaff_ESI[0x44] << 3);
      puVar5[1] = uVar2;
      uVar2 = FUN_00577e31(puVar5[-5] * unaff_ESI[6],unaff_ESI[0x43]);
      puVar5[3] = uVar2;
      uVar2 = FUN_00577e31(puVar5[-4] * unaff_ESI[7],unaff_ESI[0x44]);
      puVar5[0xc] = 0;
      puVar5[4] = uVar2;
      *(undefined1 *)(puVar5 + 5) = 1;
      iVar3 = iVar3 + 1;
      puVar5 = puVar5 + 0x15;
    } while (iVar3 < unaff_ESI[8]);
  }
  iVar3 = FUN_00577e31(unaff_ESI[7],unaff_ESI[0x44] << 3);
  unaff_ESI[0x46] = iVar3;
  if ((unaff_ESI[8] <= unaff_ESI[0x48]) && ((char)unaff_ESI[0x32] == '\0')) {
    *(undefined1 *)(unaff_ESI[99] + 0x10) = 0;
    return;
  }
  *(undefined1 *)(unaff_ESI[99] + 0x10) = 1;
  return;
}

