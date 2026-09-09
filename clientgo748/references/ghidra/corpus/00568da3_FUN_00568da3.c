// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00568da3 | Name: FUN_00568da3


undefined4 FUN_00568da3(void)

{
  int iVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  undefined4 uVar5;
  void *pvVar6;
  undefined4 *puVar7;
  int *extraout_ECX;
  int unaff_EBP;
  uint uVar8;
  undefined4 *puVar9;
  uint uVar10;
  
  FUN_00592900();
  if ((char)extraout_ECX[2] == '\x02') {
    iVar1 = *(int *)(*extraout_ECX + 0x1058);
    pvVar6 = operator_new(iVar1 << 4);
    *(void **)(unaff_EBP + -0x30) = pvVar6;
    *(undefined4 *)(unaff_EBP + -4) = 0;
    if (pvVar6 == (void *)0x0) {
      *(undefined4 *)(unaff_EBP + -0x10) = 0;
    }
    else {
      _vector_constructor_iterator_(pvVar6,0x10,iVar1,FUN_00430f10);
      *(undefined4 *)(unaff_EBP + -0x10) = *(undefined4 *)(unaff_EBP + -0x30);
    }
    *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
    if (*(int *)(unaff_EBP + -0x10) == 0) {
      uVar5 = 0x8007000e;
    }
    else {
      iVar1 = *(int *)(extraout_ECX[1] + 0x1058);
      pvVar6 = operator_new(iVar1 << 4);
      *(void **)(unaff_EBP + -0x30) = pvVar6;
      *(undefined4 *)(unaff_EBP + -4) = 1;
      if (pvVar6 == (void *)0x0) {
        *(undefined4 *)(unaff_EBP + -0x14) = 0;
      }
      else {
        _vector_constructor_iterator_(pvVar6,0x10,iVar1,FUN_00430f10);
        *(undefined4 *)(unaff_EBP + -0x14) = *(undefined4 *)(unaff_EBP + -0x30);
      }
      *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
      if (*(int *)(unaff_EBP + -0x14) == 0) {
        FUN_0058ea3a(*(undefined4 *)(unaff_EBP + -0x10));
        uVar5 = 0x8007000e;
      }
      else {
        iVar1 = *extraout_ECX;
        iVar2 = extraout_ECX[1];
        iVar3 = *(int *)(iVar1 + 0x1058);
        uVar8 = *(uint *)(iVar2 + 0x1058);
        uVar10 = *(uint *)(iVar2 + 0x105c);
        uVar4 = *(uint *)(iVar2 + 0x1060);
        *(undefined4 *)(unaff_EBP + -0x28) = 0;
        *(undefined4 *)(unaff_EBP + -0x1c) = 0;
        *(uint *)(unaff_EBP + -0x34) = (uint)(iVar3 << 0x10) / uVar8;
        *(uint *)(unaff_EBP + -0x38) = (uint)(*(int *)(iVar1 + 0x105c) << 0x10) / uVar10;
        *(uint *)(unaff_EBP + -0x3c) = (uint)(*(int *)(iVar1 + 0x1060) << 0x10) / uVar4;
        if (uVar4 != 0) {
          do {
            *(undefined4 *)(unaff_EBP + -0x2c) = 0xffffffff;
            uVar8 = 0;
            *(undefined4 *)(unaff_EBP + -0x30) = 0;
            *(undefined4 *)(unaff_EBP + -0x18) = 0;
            if (uVar10 != 0) {
              do {
                *(undefined4 *)(unaff_EBP + -0x20) = 0;
                *(undefined4 *)(unaff_EBP + -0x24) = 0;
                if (((*(uint *)(unaff_EBP + -0x2c) ^ uVar8) & 0xffff0000) != 0) {
                  (**(code **)(*(int *)*extraout_ECX + 4))
                            (uVar8 >> 0x10,*(uint *)(unaff_EBP + -0x28) >> 0x10,
                             *(undefined4 *)(unaff_EBP + -0x10));
                  *(uint *)(unaff_EBP + -0x2c) = uVar8;
                }
                if (*(int *)(extraout_ECX[1] + 0x1058) != 0) {
                  puVar7 = *(undefined4 **)(unaff_EBP + -0x14);
                  do {
                    uVar8 = *(uint *)(unaff_EBP + -0x20);
                    *(int *)(unaff_EBP + -0x20) =
                         *(int *)(unaff_EBP + -0x20) + *(int *)(unaff_EBP + -0x34);
                    puVar9 = (undefined4 *)((uVar8 >> 0x10) * 0x10 + *(int *)(unaff_EBP + -0x10));
                    *(int *)(unaff_EBP + -0x24) = *(int *)(unaff_EBP + -0x24) + 1;
                    uVar8 = *(uint *)(unaff_EBP + -0x24);
                    *puVar7 = *puVar9;
                    puVar7[1] = puVar9[1];
                    puVar7[2] = puVar9[2];
                    puVar7[3] = puVar9[3];
                    puVar7 = puVar7 + 4;
                  } while (uVar8 < *(uint *)(extraout_ECX[1] + 0x1058));
                }
                (**(code **)(*(int *)extraout_ECX[1] + 8))
                          (*(undefined4 *)(unaff_EBP + -0x18),*(undefined4 *)(unaff_EBP + -0x1c),
                           *(undefined4 *)(unaff_EBP + -0x14));
                uVar8 = *(int *)(unaff_EBP + -0x30) + *(int *)(unaff_EBP + -0x38);
                iVar1 = extraout_ECX[1];
                *(int *)(unaff_EBP + -0x18) = *(int *)(unaff_EBP + -0x18) + 1;
                uVar10 = *(uint *)(iVar1 + 0x105c);
                *(uint *)(unaff_EBP + -0x30) = uVar8;
              } while (*(uint *)(unaff_EBP + -0x18) < uVar10);
            }
            *(int *)(unaff_EBP + -0x28) = *(int *)(unaff_EBP + -0x28) + *(int *)(unaff_EBP + -0x3c);
            *(int *)(unaff_EBP + -0x1c) = *(int *)(unaff_EBP + -0x1c) + 1;
          } while (*(uint *)(unaff_EBP + -0x1c) < *(uint *)(extraout_ECX[1] + 0x1060));
        }
        FUN_0058ea3a(*(undefined4 *)(unaff_EBP + -0x10));
        FUN_0058ea3a(*(undefined4 *)(unaff_EBP + -0x14));
        uVar5 = 0;
      }
    }
  }
  else {
    uVar5 = 0x80004005;
  }
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return uVar5;
}

