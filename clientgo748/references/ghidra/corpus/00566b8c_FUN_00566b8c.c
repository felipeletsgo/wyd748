// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00566b8c | Name: FUN_00566b8c


int FUN_00566b8c(void)

{
  undefined4 uVar1;
  int iVar2;
  uint uVar3;
  void *pvVar4;
  int *extraout_ECX;
  int iVar5;
  undefined *puVar6;
  undefined4 *puVar7;
  int unaff_EBP;
  undefined4 *puVar8;
  undefined4 *puVar9;
  
  FUN_00592900();
  *(int **)(unaff_EBP + -0x20) = extraout_ECX;
  FUN_005704b2();
  iVar5 = *(int *)(unaff_EBP + 0xc);
  *(undefined4 *)(unaff_EBP + -4) = 0;
  if (iVar5 == 0) {
LAB_00566be1:
    puVar6 = &DAT_005c5258;
LAB_00566be6:
    uVar1 = FUN_0055fab2(*extraout_ECX);
    iVar2 = FUN_0055fb4b(puVar6,extraout_ECX[2],uVar1);
    *(int *)(unaff_EBP + -0x1c) = iVar2;
    if (iVar2 != 0) {
      if (iVar2 != *extraout_ECX) {
        FUN_0056705a();
        *(undefined1 *)(unaff_EBP + -4) = 1;
        iVar2 = FUN_0055fab2(iVar2);
        uVar3 = *(uint *)(iVar2 + 8) >> 3;
        *(uint *)(unaff_EBP + -0x10) = uVar3;
        *(int **)(unaff_EBP + -0x14) = extraout_ECX;
        do {
          puVar7 = *(undefined4 **)(unaff_EBP + -0x14);
          if (puVar7 != (undefined4 *)0x0) {
            do {
              pvVar4 = operator_new(puVar7[5] * puVar7[3] * uVar3 * puVar7[4]);
              *(void **)(unaff_EBP + -0x18) = pvVar4;
              if (pvVar4 == (void *)0x0) {
                *(undefined1 *)(unaff_EBP + -4) = 0;
                FUN_00567064();
                iVar2 = -0x7ff8fff2;
                goto LAB_00566e36;
              }
              *(undefined4 *)(unaff_EBP + -0xd4) = puVar7[1];
              *(undefined4 *)(unaff_EBP + -0xd0) = *puVar7;
              *(undefined4 *)(unaff_EBP + -0xcc) = puVar7[0xc];
              *(undefined4 *)(unaff_EBP + -200) = puVar7[0xd];
              puVar8 = puVar7 + 6;
              puVar9 = (undefined4 *)(unaff_EBP + -0xc4);
              for (iVar5 = 6; iVar5 != 0; iVar5 = iVar5 + -1) {
                *puVar9 = *puVar8;
                puVar8 = puVar8 + 1;
                puVar9 = puVar9 + 1;
              }
              puVar8 = puVar7 + 6;
              puVar9 = (undefined4 *)(unaff_EBP + -0xac);
              for (iVar5 = 6; iVar5 != 0; iVar5 = iVar5 + -1) {
                *puVar9 = *puVar8;
                puVar8 = puVar8 + 1;
                puVar9 = puVar9 + 1;
              }
              *(undefined4 *)(unaff_EBP + -0x90) = 0;
              *(undefined4 *)(unaff_EBP + -0x94) = 1;
              *(undefined4 *)(unaff_EBP + -0x8c) = puVar7[2];
              *(undefined4 *)(unaff_EBP + -0x88) = *(undefined4 *)(unaff_EBP + -0x18);
              *(undefined4 *)(unaff_EBP + -0x84) = *(undefined4 *)(unaff_EBP + -0x1c);
              *(int *)(unaff_EBP + -0x80) = puVar7[3] * *(int *)(unaff_EBP + -0x10);
              *(int *)(unaff_EBP + -0x7c) = puVar7[3] * *(int *)(unaff_EBP + -0x10) * puVar7[4];
              *(undefined4 *)(unaff_EBP + -0x78) = 0;
              *(undefined4 *)(unaff_EBP + -0x74) = 0;
              *(undefined4 *)(unaff_EBP + -0x70) = puVar7[3];
              *(undefined4 *)(unaff_EBP + -0x6c) = puVar7[4];
              *(undefined4 *)(unaff_EBP + -0x68) = 0;
              *(undefined4 *)(unaff_EBP + -100) = puVar7[5];
              puVar8 = (undefined4 *)(unaff_EBP + -0x78);
              puVar9 = (undefined4 *)(unaff_EBP + -0x60);
              for (iVar5 = 6; iVar5 != 0; iVar5 = iVar5 + -1) {
                *puVar9 = *puVar8;
                puVar8 = puVar8 + 1;
                puVar9 = puVar9 + 1;
              }
              *(undefined4 *)(unaff_EBP + -0x44) = 0;
              *(undefined4 *)(unaff_EBP + -0x48) = 1;
              *(undefined4 *)(unaff_EBP + -0x40) = puVar7[2];
              iVar2 = FUN_0056a815(unaff_EBP + -0x88,unaff_EBP + -0xd4,0x80001);
              if (iVar2 < 0) {
                FUN_0058ea3a(*(undefined4 *)(unaff_EBP + -0x18));
                *(undefined1 *)(unaff_EBP + -4) = 0;
                FUN_00567064();
                goto LAB_00566e36;
              }
              if ((puVar7[1] != 0) && (puVar7[0xe] != 0)) {
                FUN_0058ea3a(puVar7[1]);
                puVar7[1] = 0;
              }
              *puVar7 = *(undefined4 *)(unaff_EBP + -0x1c);
              puVar7[1] = *(undefined4 *)(unaff_EBP + -0x18);
              puVar8 = (undefined4 *)(unaff_EBP + -0x60);
              puVar9 = puVar7 + 6;
              for (iVar5 = 6; iVar5 != 0; iVar5 = iVar5 + -1) {
                *puVar9 = *puVar8;
                puVar8 = puVar8 + 1;
                puVar9 = puVar9 + 1;
              }
              puVar7[0xc] = *(undefined4 *)(unaff_EBP + -0x80);
              puVar7[0xd] = *(undefined4 *)(unaff_EBP + -0x7c);
              uVar3 = *(uint *)(unaff_EBP + -0x10);
              puVar7[0xe] = 1;
              puVar7 = (undefined4 *)puVar7[0x13];
            } while (puVar7 != (undefined4 *)0x0);
            iVar5 = *(int *)(unaff_EBP + 0xc);
          }
          iVar2 = *(int *)(*(int *)(unaff_EBP + -0x14) + 0x50);
          *(int *)(unaff_EBP + -0x14) = iVar2;
        } while (iVar2 != 0);
        *(undefined1 *)(unaff_EBP + -4) = 0;
        FUN_00567064();
      }
      iVar2 = FUN_005704c4(*(undefined4 *)(unaff_EBP + 8),*(undefined4 *)(unaff_EBP + 0x10));
      if (iVar2 < 0) goto LAB_00566e36;
      if (iVar5 == 0) {
        uVar1 = 1;
LAB_00566e26:
        iVar2 = FUN_005652fb(*(undefined4 *)(unaff_EBP + -0x3c),uVar1);
      }
      else {
        if (iVar5 != 4) {
          if (iVar5 != 6) {
            iVar2 = -0x7fffbfff;
            goto LAB_00566e36;
          }
          uVar1 = 0;
          goto LAB_00566e26;
        }
        iVar2 = FUN_005668e4(*(undefined4 *)(unaff_EBP + -0x3c));
      }
      if (-1 < iVar2) {
        iVar2 = 0;
      }
      goto LAB_00566e36;
    }
  }
  else {
    if (iVar5 < 1) {
LAB_00566dcc:
      iVar2 = -0x7789f794;
      goto LAB_00566e36;
    }
    if (3 < iVar5) {
      if (iVar5 == 4) {
        puVar6 = &DAT_005c5270;
        goto LAB_00566be6;
      }
      if (iVar5 != 5) {
        if (iVar5 == 6) goto LAB_00566be1;
        goto LAB_00566dcc;
      }
    }
  }
  iVar2 = -0x7789f4a7;
LAB_00566e36:
  *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
  FUN_005705cf();
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return iVar2;
}

