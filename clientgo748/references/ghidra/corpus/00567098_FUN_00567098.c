// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00567098 | Name: FUN_00567098


/* WARNING: Removing unreachable block (ram,0x0056710d) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int * FUN_00567098(uint param_1,int param_2,int param_3)

{
  int *piVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  int iVar10;
  int *piVar11;
  int iVar12;
  int iVar13;
  int iVar14;
  uint uVar15;
  int iVar16;
  int iVar17;
  uint uVar18;
  float local_24;
  uint local_1c;
  float local_18;
  uint local_10;
  float local_c;
  float local_8;
  
  uVar18 = 0;
  if ((param_1 != 0) && (param_2 != 0)) {
    fVar2 = (float)param_2;
    if (param_2 < 0) {
      fVar2 = fVar2 + _DAT_005a67dc;
    }
    fVar3 = (float)(int)param_1;
    if ((int)param_1 < 0) {
      fVar3 = fVar3 + _DAT_005a67dc;
    }
    fVar4 = fVar2 / fVar3;
    uVar15 = 0x10;
    fVar5 = _DAT_005a67d4 / fVar4;
    if (param_1 != 0) {
      do {
        iVar10 = __ftol();
        uVar18 = uVar18 + 1;
        uVar15 = uVar15 + 0xc + iVar10 * 0x10;
      } while (uVar18 < param_1);
    }
    piVar11 = operator_new(uVar15);
    if (piVar11 != (int *)0x0) {
      iVar10 = 0;
      local_1c = 0;
      iVar17 = 4;
      iVar16 = iVar17;
      if (param_1 != 0) {
        do {
          local_8 = 0.0;
          local_10 = 0;
          fVar6 = (float)(int)local_1c;
          iVar17 = iVar16 + 4;
          if ((int)local_1c < 0) {
            fVar6 = fVar6 + _DAT_005a67dc;
          }
          do {
            fVar7 = (float)(int)local_10;
            if ((int)local_10 < 0) {
              fVar7 = fVar7 + _DAT_005a67dc;
            }
            fVar7 = (fVar7 + fVar6) - _DAT_005a67d4;
            local_24 = fVar7 * fVar4;
            local_c = fVar4 + local_24;
            if (param_3 == 0) {
              if (local_24 < _DAT_005a67ec) {
                local_24 = 0.0;
              }
              if (fVar2 < local_c) {
                local_c = fVar2;
              }
            }
            FUN_00592f0d((double)local_24);
            iVar12 = __ftol();
            fVar8 = (float)iVar12;
            if (fVar8 < local_c) {
              iVar13 = iVar12 - param_2;
              do {
                local_18 = _DAT_005a67d8 + fVar8;
                if (iVar12 < 0) {
                  iVar14 = iVar12 + param_2;
                }
                else {
                  iVar14 = iVar13;
                  if (iVar12 < param_2) {
                    iVar14 = iVar12;
                  }
                }
                if (iVar14 != iVar10) {
                  if (_DAT_005a6948 < local_8) {
                    piVar1 = (int *)(iVar17 + (int)piVar11);
                    *piVar1 = iVar10;
                    iVar17 = iVar17 + 8;
                    piVar1[1] = (int)local_8;
                  }
                  local_8 = 0.0;
                  iVar10 = iVar14;
                }
                if (fVar8 < local_24) {
                  fVar8 = local_24;
                }
                if (local_c < local_18) {
                  local_18 = local_c;
                }
                if (param_3 == 0) {
                  if (_DAT_005a67ec <= fVar7) {
                    if (fVar7 + _DAT_005a67d8 < fVar3) goto LAB_005672bf;
                    fVar9 = 0.0;
                  }
                  else {
                    fVar9 = 1.0;
                  }
                }
                else {
LAB_005672bf:
                  fVar9 = (local_18 + fVar8) * fVar5 - fVar7;
                }
                if (local_10 != 0) {
                  fVar9 = 1.0 - fVar9;
                }
                iVar12 = iVar12 + 1;
                iVar13 = iVar13 + 1;
                local_8 = (local_18 - fVar8) * fVar9 + local_8;
                fVar8 = (float)iVar12;
              } while (fVar8 < local_c);
            }
            local_10 = local_10 + 1;
          } while (local_10 < 2);
          if (_DAT_005a6948 < local_8) {
            piVar1 = (int *)(iVar17 + (int)piVar11);
            iVar17 = iVar17 + 8;
            *piVar1 = iVar10;
            piVar1[1] = (int)local_8;
          }
          local_1c = local_1c + 1;
          *(int *)(iVar16 + (int)piVar11) = iVar17 - iVar16;
          iVar16 = iVar17;
        } while (local_1c < param_1);
      }
      *piVar11 = iVar17;
      return piVar11;
    }
  }
  return (int *)0x0;
}

