// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005693ca | Name: FUN_005693ca


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_005693ca(void)

{
  int *piVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  uint uVar6;
  int iVar7;
  undefined4 *puVar8;
  float fVar9;
  undefined4 uVar10;
  void *pvVar11;
  int iVar12;
  int *extraout_ECX;
  int iVar13;
  float *pfVar14;
  float *pfVar15;
  float *pfVar16;
  int unaff_EBP;
  int iVar17;
  int iVar18;
  
  FUN_00592900();
  if ((char)extraout_ECX[2] == '\x05') {
    iVar17 = *extraout_ECX;
    uVar6 = *(uint *)(iVar17 + 0x1058);
    iVar12 = extraout_ECX[1];
    if ((((*(uint *)(iVar12 + 0x1058) == uVar6 >> 1) ||
         ((*(uint *)(iVar12 + 0x1058) == 1 && (uVar6 == 1)))) &&
        ((*(uint *)(iVar12 + 0x105c) == *(uint *)(iVar17 + 0x105c) >> 1 ||
         ((*(uint *)(iVar12 + 0x105c) == 1 && (*(int *)(iVar17 + 0x105c) == 1)))))) &&
       (*(uint *)(iVar12 + 0x1060) == *(uint *)(iVar17 + 0x1060) >> 1)) {
      if ((1 < uVar6) && ((uVar6 & 1) != 0)) {
        *(uint *)(iVar17 + 0x1058) = uVar6 & 0xfffffffe;
        piVar1 = (int *)(*extraout_ECX + 0x1064);
        *piVar1 = *piVar1 - *(int *)(*extraout_ECX + 0x1068);
      }
      uVar6 = *(uint *)(*extraout_ECX + 0x105c);
      if (1 < uVar6) {
        *(uint *)(*extraout_ECX + 0x105c) = uVar6 & 0xfffffffe;
      }
      uVar6 = *(uint *)(*extraout_ECX + 0x1060);
      if (1 < uVar6) {
        *(uint *)(*extraout_ECX + 0x1060) = uVar6 & 0xfffffffe;
      }
      iVar17 = *(int *)(extraout_ECX[1] + 0x1058);
      pvVar11 = operator_new(iVar17 << 4);
      *(void **)(unaff_EBP + -0x14) = pvVar11;
      *(undefined4 *)(unaff_EBP + -4) = 0;
      if (pvVar11 == (void *)0x0) {
        *(undefined4 *)(unaff_EBP + -0x20) = 0;
      }
      else {
        _vector_constructor_iterator_(pvVar11,0x10,iVar17,FUN_00430f10);
        *(undefined4 *)(unaff_EBP + -0x20) = *(undefined4 *)(unaff_EBP + -0x14);
      }
      *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
      if (*(int *)(unaff_EBP + -0x20) == 0) {
        uVar10 = 0x8007000e;
      }
      else {
        iVar17 = *(int *)(*extraout_ECX + 0x1058);
        if (*(int *)(*extraout_ECX + 0x105c) == 1) {
          pvVar11 = operator_new(iVar17 << 5);
          *(void **)(unaff_EBP + -0x14) = pvVar11;
          *(undefined4 *)(unaff_EBP + -4) = 1;
          if (pvVar11 == (void *)0x0) {
            iVar13 = 0;
          }
          else {
            _vector_constructor_iterator_(pvVar11,0x10,iVar17 << 1,FUN_00430f10);
            iVar13 = *(int *)(unaff_EBP + -0x14);
          }
          iVar12 = *(int *)(*extraout_ECX + 0x1058);
          *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
          iVar17 = iVar12 * 0x10 + iVar13;
          *(int *)(unaff_EBP + -0x38) = iVar13;
          *(int *)(unaff_EBP + -0x18) = iVar13;
          *(int *)(unaff_EBP + -0x30) = iVar17;
          *(int *)(unaff_EBP + -0x1c) = iVar13;
          *(int *)(unaff_EBP + -0x28) = iVar17;
          iVar18 = iVar17;
        }
        else {
          pvVar11 = operator_new(iVar17 << 6);
          *(void **)(unaff_EBP + -0x14) = pvVar11;
          *(undefined4 *)(unaff_EBP + -4) = 2;
          if (pvVar11 == (void *)0x0) {
            iVar17 = 0;
          }
          else {
            _vector_constructor_iterator_(pvVar11,0x10,iVar17 << 2,FUN_00430f10);
            iVar17 = *(int *)(unaff_EBP + -0x14);
          }
          iVar12 = *(int *)(*extraout_ECX + 0x1058);
          *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
          *(int *)(unaff_EBP + -0x30) = iVar12 * 0x10 + iVar17;
          iVar18 = *(int *)(unaff_EBP + -0x30);
          *(int *)(unaff_EBP + -0x1c) = iVar12 * 0x20 + iVar17;
          *(int *)(unaff_EBP + -0x28) = iVar12 * 0x30 + iVar17;
          iVar13 = *(int *)(unaff_EBP + -0x1c);
          *(int *)(unaff_EBP + -0x38) = iVar17;
          *(int *)(unaff_EBP + -0x18) = iVar17;
          iVar17 = *(int *)(unaff_EBP + -0x28);
        }
        if (*(int *)(unaff_EBP + -0x38) == 0) {
          uVar10 = 0x8007000e;
        }
        else {
          if (iVar12 == 1) {
            *(int *)(unaff_EBP + -0x3c) = *(int *)(unaff_EBP + -0x18);
            *(int *)(unaff_EBP + -0x40) = iVar18;
          }
          else {
            *(int *)(unaff_EBP + -0x3c) = *(int *)(unaff_EBP + -0x18) + 0x10;
            iVar13 = iVar13 + 0x10;
            *(int *)(unaff_EBP + -0x40) = iVar18 + 0x10;
            iVar17 = iVar17 + 0x10;
          }
          iVar12 = extraout_ECX[1];
          *(undefined4 *)(unaff_EBP + -0x24) = 0;
          iVar7 = *(int *)(iVar12 + 0x1060);
          *(int *)(unaff_EBP + -0x44) = iVar13;
          *(int *)(unaff_EBP + -0x14) = iVar17;
          if (iVar7 != 0) {
            do {
              *(undefined4 *)(unaff_EBP + -0x10) = 0;
              if (*(int *)(iVar12 + 0x105c) != 0) {
                do {
                  iVar17 = *(int *)(unaff_EBP + -0x24) * 2;
                  (**(code **)(*(int *)*extraout_ECX + 4))
                            (*(int *)(unaff_EBP + -0x10) * 2,iVar17,
                             *(undefined4 *)(unaff_EBP + -0x18));
                  if (iVar18 != *(int *)(unaff_EBP + -0x18)) {
                    (**(code **)(*(int *)*extraout_ECX + 4))
                              (*(int *)(unaff_EBP + -0x10) * 2,iVar17 + 1,iVar18);
                  }
                  if (*(int *)(unaff_EBP + -0x1c) != *(int *)(unaff_EBP + -0x18)) {
                    (**(code **)(*(int *)*extraout_ECX + 4))
                              (*(int *)(unaff_EBP + -0x10) * 2 + 1,iVar17,
                               *(int *)(unaff_EBP + -0x1c));
                  }
                  iVar12 = *(int *)(unaff_EBP + -0x28);
                  if ((iVar12 != iVar18) && (iVar12 != *(int *)(unaff_EBP + -0x1c))) {
                    (**(code **)(*(int *)*extraout_ECX + 4))
                              (*(int *)(unaff_EBP + -0x10) * 2 + 1,iVar17 + 1,iVar12);
                  }
                  iVar17 = extraout_ECX[1];
                  *(undefined4 *)(unaff_EBP + -0x2c) = 0;
                  fVar9 = _DAT_005a67f0;
                  if (*(int *)(iVar17 + 0x1058) != 0) {
                    *(undefined4 *)(unaff_EBP + -0x34) = *(undefined4 *)(unaff_EBP + -0x20);
                    do {
                      iVar17 = *(int *)(unaff_EBP + -0x2c) * 0x20;
                      pfVar14 = (float *)(*(int *)(unaff_EBP + -0x3c) + iVar17);
                      fVar2 = *pfVar14;
                      pfVar16 = (float *)(*(int *)(unaff_EBP + -0x18) + iVar17);
                      fVar3 = *pfVar16;
                      fVar4 = pfVar14[1];
                      fVar5 = pfVar16[1];
                      *(float *)(unaff_EBP + -0x8c) = pfVar14[2] + pfVar16[2];
                      pfVar15 = (float *)(iVar17 + iVar18);
                      *(float *)(unaff_EBP + -0x88) = pfVar14[3] + pfVar16[3];
                      *(float *)(unaff_EBP + -0xb4) = fVar2 + fVar3 + *pfVar15;
                      fVar2 = pfVar15[1];
                      fVar3 = pfVar15[2];
                      pfVar14 = (float *)(*(int *)(unaff_EBP + -0x40) + iVar17);
                      *(float *)(unaff_EBP + -0xa8) = *(float *)(unaff_EBP + -0x88) + pfVar15[3];
                      *(float *)(unaff_EBP + -0x84) = *(float *)(unaff_EBP + -0xb4) + *pfVar14;
                      *(float *)(unaff_EBP + -0x80) = fVar4 + fVar5 + fVar2 + pfVar14[1];
                      fVar2 = pfVar14[2];
                      fVar4 = pfVar14[3];
                      pfVar15 = (float *)(*(int *)(unaff_EBP + -0x1c) + iVar17);
                      *(float *)(unaff_EBP + -0x54) = *(float *)(unaff_EBP + -0x84) + *pfVar15;
                      *(float *)(unaff_EBP + -0x50) = *(float *)(unaff_EBP + -0x80) + pfVar15[1];
                      *(float *)(unaff_EBP + -0x4c) =
                           *(float *)(unaff_EBP + -0x8c) + fVar3 + fVar2 + pfVar15[2];
                      fVar2 = pfVar15[3];
                      pfVar15 = (float *)(*(int *)(unaff_EBP + -0x44) + iVar17);
                      fVar3 = *pfVar15;
                      *(float *)(unaff_EBP + -0x60) = *(float *)(unaff_EBP + -0x50) + pfVar15[1];
                      *(float *)(unaff_EBP + -0x5c) = *(float *)(unaff_EBP + -0x4c) + pfVar15[2];
                      *(float *)(unaff_EBP + -0x58) =
                           *(float *)(unaff_EBP + -0xa8) + fVar4 + fVar2 + pfVar15[3];
                      pfVar15 = (float *)(*(int *)(unaff_EBP + -0x28) + iVar17);
                      fVar2 = *pfVar15;
                      puVar8 = *(undefined4 **)(unaff_EBP + -0x34);
                      *(int *)(unaff_EBP + -0x34) = *(int *)(unaff_EBP + -0x34) + 0x10;
                      fVar4 = pfVar15[1];
                      *(float *)(unaff_EBP + -0x9c) = *(float *)(unaff_EBP + -0x5c) + pfVar15[2];
                      fVar5 = pfVar15[3];
                      pfVar15 = (float *)(iVar17 + *(int *)(unaff_EBP + -0x14));
                      *(int *)(unaff_EBP + -0x2c) = *(int *)(unaff_EBP + -0x2c) + 1;
                      *(float *)(unaff_EBP + -0x98) = *(float *)(unaff_EBP + -0x58) + fVar5;
                      uVar6 = *(uint *)(unaff_EBP + -0x2c);
                      *(float *)(unaff_EBP + -0xc4) =
                           *(float *)(unaff_EBP + -0x54) + fVar3 + fVar2 + *pfVar15;
                      fVar2 = pfVar15[1];
                      fVar3 = pfVar15[2];
                      *(float *)(unaff_EBP + -0xb8) = *(float *)(unaff_EBP + -0x98) + pfVar15[3];
                      *(float *)(unaff_EBP + -0x74) = *(float *)(unaff_EBP + -0xc4) * fVar9;
                      *(float *)(unaff_EBP + -0x70) =
                           (*(float *)(unaff_EBP + -0x60) + fVar4 + fVar2) * fVar9;
                      *(float *)(unaff_EBP + -0x6c) =
                           (*(float *)(unaff_EBP + -0x9c) + fVar3) * fVar9;
                      *(float *)(unaff_EBP + -0x68) = *(float *)(unaff_EBP + -0xb8) * fVar9;
                      *puVar8 = *(undefined4 *)(unaff_EBP + -0x74);
                      puVar8[1] = *(undefined4 *)(unaff_EBP + -0x70);
                      puVar8[2] = *(undefined4 *)(unaff_EBP + -0x6c);
                      puVar8[3] = *(undefined4 *)(unaff_EBP + -0x68);
                      iVar18 = *(int *)(unaff_EBP + -0x30);
                    } while (uVar6 < *(uint *)(extraout_ECX[1] + 0x1058));
                  }
                  (**(code **)(*(int *)extraout_ECX[1] + 8))
                            (*(undefined4 *)(unaff_EBP + -0x10),*(undefined4 *)(unaff_EBP + -0x24),
                             *(undefined4 *)(unaff_EBP + -0x20));
                  *(int *)(unaff_EBP + -0x10) = *(int *)(unaff_EBP + -0x10) + 1;
                } while (*(uint *)(unaff_EBP + -0x10) < *(uint *)(extraout_ECX[1] + 0x105c));
              }
              *(int *)(unaff_EBP + -0x24) = *(int *)(unaff_EBP + -0x24) + 1;
              iVar12 = extraout_ECX[1];
            } while (*(uint *)(unaff_EBP + -0x24) < *(uint *)(iVar12 + 0x1060));
          }
          FUN_0058ea3a(*(undefined4 *)(unaff_EBP + -0x38));
          uVar10 = 0;
        }
        FUN_0058ea3a(*(undefined4 *)(unaff_EBP + -0x20));
      }
    }
    else {
      uVar10 = 0x80004005;
    }
  }
  else {
    uVar10 = 0x80004005;
  }
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return uVar10;
}

