// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058684d | Name: FUN_0058684d


void FUN_0058684d(code *param_1)

{
  int *piVar1;
  double dVar2;
  int *piVar3;
  int iVar4;
  short *psVar5;
  undefined2 *puVar6;
  double *pdVar7;
  int iVar8;
  int iVar9;
  int *piVar10;
  int *piVar11;
  int local_10;
  double *local_c;
  int local_8;
  
  piVar3 = (int *)param_1;
  piVar10 = (int *)((int)param_1 + 0x20);
  piVar11 = (int *)((int)param_1 + 0x198);
  piVar1 = (int *)((int)param_1 + 0xc4);
  local_8 = 0;
  param_1 = (code *)0x0;
  local_10 = 0;
  if (0 < *piVar10) {
    piVar10 = (int *)(*piVar11 + 0x2c);
    piVar11 = (int *)(*piVar1 + 0x24);
    do {
      iVar8 = *piVar11;
      if (iVar8 == 1) {
        param_1 = FUN_0058b8eb;
LAB_00586908:
        local_8 = 0;
      }
      else {
        if (iVar8 == 2) {
          param_1 = FUN_0058b72d;
          goto LAB_00586908;
        }
        if (iVar8 == 4) {
          param_1 = FUN_0058b418;
          goto LAB_00586908;
        }
        if (iVar8 == 8) {
          iVar8 = piVar3[0x11];
          if (iVar8 == 0) {
            param_1 = FUN_0058b928;
            goto LAB_00586908;
          }
          if (iVar8 == 1) {
            param_1 = FUN_0058b974;
            local_8 = 1;
          }
          else {
            if (iVar8 != 2) {
              *(undefined4 *)(*piVar3 + 0x14) = 0x2f;
              goto LAB_005868c2;
            }
            param_1 = FUN_0058b9c0;
            local_8 = 2;
          }
        }
        else {
          *(undefined4 *)(*piVar3 + 0x14) = 6;
          *(int *)(*piVar3 + 0x18) = *piVar11;
LAB_005868c2:
          (**(code **)*piVar3)(piVar3);
        }
      }
      piVar10[-10] = (int)param_1;
      if ((((char)piVar11[3] != '\0') && (*piVar10 != local_8)) && (iVar8 = piVar11[10], iVar8 != 0)
         ) {
        *piVar10 = local_8;
        if (local_8 == 0) {
          puVar6 = (undefined2 *)piVar11[0xb];
          iVar8 = iVar8 - (int)puVar6;
          iVar9 = 0x40;
          do {
            *puVar6 = *(undefined2 *)(iVar8 + (int)puVar6);
            puVar6 = puVar6 + 1;
            iVar9 = iVar9 + -1;
          } while (iVar9 != 0);
        }
        else if (local_8 == 1) {
          iVar9 = piVar11[0xb];
          psVar5 = &DAT_005a9510;
          do {
            *(short *)(iVar9 + -0x5a9510 + (int)psVar5) =
                 (short)((int)*(short *)(iVar8 + -0x5a9510 + (int)psVar5) * (int)*psVar5 + 0x800 >>
                        0xc);
            psVar5 = psVar5 + 1;
          } while ((int)psVar5 < 0x5a9590);
        }
        else if (local_8 == 2) {
          iVar9 = piVar11[0xb];
          iVar4 = 0;
          local_c = (double *)&DAT_005a9590;
          do {
            pdVar7 = (double *)&DAT_005a9590;
            do {
              dVar2 = *pdVar7;
              pdVar7 = pdVar7 + 1;
              *(float *)(iVar9 + iVar4 * 4) =
                   (float)*(ushort *)(iVar8 + iVar4 * 2) * (float)dVar2 * (float)*local_c;
              iVar4 = iVar4 + 1;
            } while ((int)pdVar7 < 0x5a95d0);
            local_c = local_c + 1;
          } while ((int)local_c < 0x5a95d0);
        }
        else {
          *(undefined4 *)(*piVar3 + 0x14) = 0x2f;
          (**(code **)*piVar3)(piVar3);
        }
      }
      local_10 = local_10 + 1;
      piVar10 = piVar10 + 1;
      piVar11 = piVar11 + 0x15;
    } while (local_10 < piVar3[8]);
  }
  return;
}

