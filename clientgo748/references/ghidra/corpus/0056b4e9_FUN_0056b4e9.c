// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056b4e9 | Name: FUN_0056b4e9


void FUN_0056b4e9(void)

{
  int iVar1;
  undefined4 *puVar2;
  void *pvVar3;
  undefined4 uVar4;
  uint uVar5;
  int extraout_ECX;
  int iVar6;
  uint uVar7;
  int unaff_EBP;
  uint uVar8;
  undefined4 *puVar9;
  uint *puVar10;
  
  FUN_00592900();
  uVar8 = *(int *)(unaff_EBP + 8) + *(int *)(extraout_ECX + 0x1034);
  *(int *)(unaff_EBP + 0xc) = *(int *)(unaff_EBP + 0xc) + *(int *)(extraout_ECX + 0x1040);
  iVar6 = *(int *)(extraout_ECX + 0x10c0);
  *(uint *)(unaff_EBP + 8) = uVar8;
  if (iVar6 == 0) {
    puVar2 = operator_new(*(int *)(extraout_ECX + 0x10a8) * *(int *)(extraout_ECX + 0x10ac) * 8);
    *(undefined4 **)(extraout_ECX + 0x10c0) = puVar2;
    if (puVar2 == (undefined4 *)0x0) goto LAB_0056b8d2;
    for (iVar6 = (*(int *)(extraout_ECX + 0x10a8) * *(int *)(extraout_ECX + 0x10ac) & 0x1fffffffU)
                 << 1; iVar6 != 0; iVar6 = iVar6 + -1) {
      *puVar2 = 0;
      puVar2 = puVar2 + 1;
    }
    for (iVar6 = 0; iVar6 != 0; iVar6 = iVar6 + -1) {
      *(undefined1 *)puVar2 = 0;
      puVar2 = (undefined4 *)((int)puVar2 + 1);
    }
  }
  puVar10 = (uint *)(*(int *)(extraout_ECX + 0x10c0) +
                    ((*(int *)(unaff_EBP + 0xc) - *(int *)(extraout_ECX + 0x109c)) *
                     *(int *)(extraout_ECX + 0x10a8) +
                    (uVar8 - *(int *)(extraout_ECX + 0x1090) >> 2)) * 8);
  uVar5 = puVar10[1];
  *(uint **)(unaff_EBP + -0x10) = puVar10;
  if (uVar5 == 0) {
    iVar6 = *(int *)(extraout_ECX + 0x10a4);
    *(int *)(unaff_EBP + -0x18) = iVar6 << 4;
    pvVar3 = operator_new(iVar6 << 8);
    *(void **)(unaff_EBP + -0x14) = pvVar3;
    *(undefined4 *)(unaff_EBP + -4) = 0;
    if (pvVar3 == (void *)0x0) {
      uVar5 = 0;
    }
    else {
      _vector_constructor_iterator_(pvVar3,0x10,*(int *)(unaff_EBP + -0x18),FUN_00430f10);
      uVar5 = *(uint *)(unaff_EBP + -0x14);
    }
    *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
    puVar10[1] = uVar5;
    if (uVar5 == 0) goto LAB_0056b8d2;
    *puVar10 = 0;
    *(int *)(extraout_ECX + 0x10bc) = *(int *)(extraout_ECX + 0x10bc) + 1;
  }
  if (*puVar10 == 0) {
    uVar5 = 4;
    if (3 < *(uint *)(extraout_ECX + 0x103c)) {
      uVar5 = *(uint *)(extraout_ECX + 0x103c);
    }
    if (((uVar8 & 3) < *(uint *)(extraout_ECX + 0x1034)) || (uVar5 < (uVar8 & 3) + 4)) {
      uVar5 = *(uint *)(extraout_ECX + 0x108c);
      iVar6 = (uVar8 >> 2) * *(int *)(extraout_ECX + 0x1050) +
              (uVar5 >> 2) * *(int *)(extraout_ECX + 0x1080) +
              *(int *)(extraout_ECX + 0x1054) * *(int *)(unaff_EBP + 0xc) +
              *(int *)(extraout_ECX + 0x18);
      *(undefined4 *)(unaff_EBP + -0x14) = *(undefined4 *)(*(int *)(unaff_EBP + -0x10) + 4);
      *(uint *)(unaff_EBP + -0x18) = uVar5;
      while (uVar5 < *(uint *)(extraout_ECX + 0x1094)) {
        (**(code **)(extraout_ECX + 0x1084))(*(undefined4 *)(unaff_EBP + -0x14),iVar6);
        iVar6 = iVar6 + *(int *)(extraout_ECX + 0x1080);
        *(int *)(unaff_EBP + -0x14) = *(int *)(unaff_EBP + -0x14) + 0x100;
        *(int *)(unaff_EBP + -0x18) = *(int *)(unaff_EBP + -0x18) + 4;
        uVar5 = *(uint *)(unaff_EBP + -0x18);
      }
      puVar10 = *(uint **)(unaff_EBP + -0x10);
    }
    else {
      uVar5 = *(uint *)(extraout_ECX + 0x1038);
      *(undefined4 *)(unaff_EBP + -0x18) = 4;
      if (3 < uVar5) {
        *(uint *)(unaff_EBP + -0x18) = uVar5;
      }
      if (*(uint *)(extraout_ECX + 0x108c) < *(uint *)(extraout_ECX + 0x1030)) {
        (**(code **)(extraout_ECX + 0x1084))
                  (puVar10[1],
                   (*(uint *)(extraout_ECX + 0x108c) >> 2) * *(int *)(extraout_ECX + 0x1080) +
                   (uVar8 >> 2) * *(int *)(extraout_ECX + 0x1050) +
                   *(int *)(extraout_ECX + 0x1054) * *(int *)(unaff_EBP + 0xc) +
                   *(int *)(extraout_ECX + 0x18));
      }
      if (*(uint *)(unaff_EBP + -0x18) < *(uint *)(extraout_ECX + 0x1094)) {
        (**(code **)(extraout_ECX + 0x1084))
                  (puVar10[1],
                   (*(uint *)(extraout_ECX + 0x1094) - 4 >> 2) * *(int *)(extraout_ECX + 0x1080) +
                   (uVar8 >> 2) * *(int *)(extraout_ECX + 0x1050) +
                   *(int *)(extraout_ECX + 0x1054) * *(int *)(unaff_EBP + 0xc) +
                   *(int *)(extraout_ECX + 0x18));
      }
    }
  }
  if (*(int *)(extraout_ECX + 0x1048) != 0) {
    uVar4 = FUN_0056b152(*(undefined4 *)(unaff_EBP + 0x10));
    *(undefined4 *)(unaff_EBP + 0x10) = uVar4;
  }
  uVar5 = *(int *)(extraout_ECX + 0x1030) - *(int *)(extraout_ECX + 0x108c);
  iVar6 = *(int *)(extraout_ECX + 0x1090);
  uVar7 = *(int *)(extraout_ECX + 0x1058) + uVar5;
  if (uVar5 < uVar7) {
    do {
      puVar9 = (undefined4 *)
               (((uVar5 & 0xfffffffc | uVar8 - iVar6 & 3) << 2 | uVar5 & 3) * 0x10 +
               *(int *)(*(int *)(unaff_EBP + -0x10) + 4));
      puVar2 = *(undefined4 **)(unaff_EBP + 0x10);
      *(int *)(unaff_EBP + 0x10) = *(int *)(unaff_EBP + 0x10) + 0x10;
      *puVar9 = *puVar2;
      puVar9[1] = puVar2[1];
      puVar9[2] = puVar2[2];
      uVar5 = uVar5 + 1;
      puVar9[3] = puVar2[3];
    } while (uVar5 < uVar7);
    uVar8 = *(uint *)(unaff_EBP + 8);
    puVar10 = *(uint **)(unaff_EBP + -0x10);
  }
  uVar5 = uVar8 - *(int *)(extraout_ECX + 0x1090) & 3;
  if (*(int *)(extraout_ECX + 0x1074) == 0) {
    *puVar10 = *puVar10 | 1 << (sbyte)uVar5;
  }
  else {
    *(undefined4 *)(unaff_EBP + 0x10) = 0;
    do {
      if ((*(uint *)(extraout_ECX + 0x107c) & *(uint *)(unaff_EBP + 0x10)) == uVar5) {
        *puVar10 = *puVar10 | 1 << ((byte)*(undefined4 *)(unaff_EBP + 0x10) & 0x1f);
      }
      *(int *)(unaff_EBP + 0x10) = *(int *)(unaff_EBP + 0x10) + 1;
    } while (*(uint *)(unaff_EBP + 0x10) < 4);
  }
  if (*puVar10 == 0xf) {
    uVar5 = *(uint *)(extraout_ECX + 0x108c);
    iVar6 = (uVar8 >> 2) * *(int *)(extraout_ECX + 0x1050) +
            (uVar5 >> 2) * *(int *)(extraout_ECX + 0x1080) +
            *(int *)(extraout_ECX + 0x1054) * *(int *)(unaff_EBP + 0xc) +
            *(int *)(extraout_ECX + 0x18);
    *(uint *)(unaff_EBP + 0xc) = puVar10[1];
    *(uint *)(unaff_EBP + -0x18) = uVar5;
    while (uVar8 = *(uint *)(extraout_ECX + 0x1094), *(int *)(unaff_EBP + -0x14) = iVar6,
          uVar5 < uVar8) {
      uVar8 = 0;
      if (*(int *)(extraout_ECX + 0x1074) != 0) {
        uVar4 = *(undefined4 *)(unaff_EBP + 0xc);
        do {
          *(undefined4 *)(unaff_EBP + 0x10) = 0;
          iVar6 = uVar8 << 2;
          *(undefined4 *)(unaff_EBP + 8) = uVar4;
          do {
            iVar1 = (*(uint *)(unaff_EBP + 0x10) & *(uint *)(extraout_ECX + 0x1078)) +
                    (*(uint *)(extraout_ECX + 0x107c) & uVar8) * 4;
            if (iVar6 != iVar1) {
              puVar2 = *(undefined4 **)(unaff_EBP + 8);
              puVar9 = (undefined4 *)(iVar1 * 0x10 + *(int *)(unaff_EBP + 0xc));
              *puVar2 = *puVar9;
              puVar2[1] = puVar9[1];
              puVar2[2] = puVar9[2];
              puVar2[3] = puVar9[3];
              puVar10 = *(uint **)(unaff_EBP + -0x10);
            }
            *(int *)(unaff_EBP + 0x10) = *(int *)(unaff_EBP + 0x10) + 1;
            *(int *)(unaff_EBP + 8) = *(int *)(unaff_EBP + 8) + 0x10;
            iVar6 = iVar6 + 1;
          } while (*(uint *)(unaff_EBP + 0x10) < 4);
          uVar4 = *(undefined4 *)(unaff_EBP + 8);
          uVar8 = uVar8 + 1;
        } while (uVar8 < 4);
      }
      iVar6 = *(int *)(unaff_EBP + -0x14);
      (**(code **)(extraout_ECX + 0x1088))(iVar6,*(undefined4 *)(unaff_EBP + 0xc));
      iVar6 = iVar6 + *(int *)(extraout_ECX + 0x1080);
      *(int *)(unaff_EBP + 0xc) = *(int *)(unaff_EBP + 0xc) + 0x100;
      *(int *)(unaff_EBP + -0x18) = *(int *)(unaff_EBP + -0x18) + 4;
      uVar5 = *(uint *)(unaff_EBP + -0x18);
    }
    if (((puVar10 + 2 <
          (uint *)(*(int *)(extraout_ECX + 0x10c0) +
                  *(int *)(extraout_ECX + 0x10ac) * *(int *)(extraout_ECX + 0x10a8) * 8)) &&
        (puVar10[2] == 0)) && (puVar10[3] == 0)) {
      puVar10[3] = puVar10[1];
      puVar10[1] = 0;
    }
    else {
      FUN_0058ea3a(puVar10[1]);
      *(int *)(extraout_ECX + 0x10bc) = *(int *)(extraout_ECX + 0x10bc) + -1;
    }
    puVar10[1] = 0;
  }
LAB_0056b8d2:
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return;
}

