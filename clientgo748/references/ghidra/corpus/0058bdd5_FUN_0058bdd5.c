// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058bdd5 | Name: FUN_0058bdd5


void FUN_0058bdd5(uint param_1,int *param_2,int param_3)

{
  byte bVar1;
  int *piVar2;
  undefined1 *puVar3;
  int iVar4;
  int *piVar5;
  byte *pbVar6;
  int iVar7;
  uint uVar8;
  undefined1 *puVar9;
  undefined1 *puVar10;
  undefined1 *local_18;
  undefined1 *local_10;
  uint local_c;
  byte *local_8;
  
  piVar5 = param_2;
  iVar4 = param_1;
  local_8 = (byte *)*param_2;
  local_c = param_2[1];
  puVar10 = *(undefined1 **)(param_1 + 0x34);
  piVar2 = *(int **)(param_1 + 4);
  param_2 = *(int **)(param_1 + 0x20);
  if (puVar10 < *(undefined1 **)(param_1 + 0x30)) {
    local_10 = *(undefined1 **)(param_1 + 0x30) + (-1 - (int)puVar10);
    param_1 = *(uint *)(param_1 + 0x1c);
  }
  else {
    local_10 = (undefined1 *)(*(int *)(param_1 + 0x2c) - (int)puVar10);
    param_1 = *(uint *)(param_1 + 0x1c);
  }
  do {
    puVar9 = puVar10;
    switch(*piVar2) {
    case 0:
      if (((undefined1 *)0x101 < local_10) && (9 < local_c)) {
        *(int **)(iVar4 + 0x20) = param_2;
        *(uint *)(iVar4 + 0x1c) = param_1;
        piVar5[1] = local_c;
        piVar5[2] = (int)(local_8 + (piVar5[2] - *piVar5));
        *piVar5 = (int)local_8;
        *(undefined1 **)(iVar4 + 0x34) = puVar10;
        param_3 = FUN_0058e5dd((char)piVar2[4],*(undefined1 *)((int)piVar2 + 0x11),piVar2[5],
                               piVar2[6],iVar4,piVar5);
        puVar10 = *(undefined1 **)(iVar4 + 0x34);
        local_8 = (byte *)*piVar5;
        local_c = piVar5[1];
        param_2 = *(int **)(iVar4 + 0x20);
        param_1 = *(uint *)(iVar4 + 0x1c);
        if (puVar10 < *(undefined1 **)(iVar4 + 0x30)) {
          local_10 = *(undefined1 **)(iVar4 + 0x30) + (-1 - (int)puVar10);
        }
        else {
          local_10 = (undefined1 *)(*(int *)(iVar4 + 0x2c) - (int)puVar10);
        }
        if (param_3 != 0) {
          *piVar2 = (uint)(param_3 != 1) * 2 + 7;
          break;
        }
      }
      piVar2[3] = (uint)*(byte *)(piVar2 + 4);
      piVar2[2] = piVar2[5];
      *piVar2 = 1;
    case 1:
      for (; param_1 < (uint)piVar2[3]; param_1 = param_1 + 8) {
        if (local_c == 0) goto LAB_0058c29e;
        param_3 = 0;
        local_c = local_c - 1;
        param_2 = (int *)((uint)param_2 | (uint)*local_8 << ((byte)param_1 & 0x1f));
        local_8 = local_8 + 1;
      }
      pbVar6 = (byte *)(piVar2[2] + (*(uint *)(&DAT_005c98e0 + piVar2[3] * 4) & (uint)param_2) * 8);
      param_2 = (int *)((uint)param_2 >> (pbVar6[1] & 0x1f));
      param_1 = param_1 - pbVar6[1];
      bVar1 = *pbVar6;
      uVar8 = (uint)bVar1;
      if (uVar8 == 0) {
        pbVar6 = *(byte **)(pbVar6 + 4);
        *piVar2 = 6;
      }
      else {
        if ((bVar1 & 0x10) != 0) {
          piVar2[2] = uVar8 & 0xf;
          piVar2[1] = *(int *)(pbVar6 + 4);
          *piVar2 = 2;
          break;
        }
        if ((bVar1 & 0x40) != 0) {
          if ((bVar1 & 0x20) == 0) {
            *piVar2 = 9;
            piVar5[6] = (int)"invalid literal/length code";
switchD_0058be1a_caseD_9:
            param_3 = -3;
            puVar9 = puVar10;
            goto LAB_0058c24e;
          }
          *piVar2 = 7;
          break;
        }
        piVar2[3] = uVar8;
        pbVar6 = pbVar6 + *(int *)(pbVar6 + 4) * 8;
      }
LAB_0058c05a:
      piVar2[2] = (int)pbVar6;
      break;
    case 2:
      for (; param_1 < (uint)piVar2[2]; param_1 = param_1 + 8) {
        if (local_c == 0) goto LAB_0058c29e;
        param_3 = 0;
        local_c = local_c - 1;
        param_2 = (int *)((uint)param_2 | (uint)*local_8 << ((byte)param_1 & 0x1f));
        local_8 = local_8 + 1;
      }
      piVar2[1] = piVar2[1] + (*(uint *)(&DAT_005c98e0 + piVar2[2] * 4) & (uint)param_2);
      param_2 = (int *)((uint)param_2 >> ((byte)piVar2[2] & 0x1f));
      param_1 = param_1 - piVar2[2];
      piVar2[3] = (uint)*(byte *)((int)piVar2 + 0x11);
      piVar2[2] = piVar2[6];
      *piVar2 = 3;
    case 3:
      for (; param_1 < (uint)piVar2[3]; param_1 = param_1 + 8) {
        if (local_c == 0) goto LAB_0058c29e;
        param_3 = 0;
        local_c = local_c - 1;
        param_2 = (int *)((uint)param_2 | (uint)*local_8 << ((byte)param_1 & 0x1f));
        local_8 = local_8 + 1;
      }
      pbVar6 = (byte *)(piVar2[2] + (*(uint *)(&DAT_005c98e0 + piVar2[3] * 4) & (uint)param_2) * 8);
      param_2 = (int *)((uint)param_2 >> (pbVar6[1] & 0x1f));
      param_1 = param_1 - pbVar6[1];
      bVar1 = *pbVar6;
      if ((bVar1 & 0x10) == 0) {
        if ((bVar1 & 0x40) != 0) {
          *piVar2 = 9;
          piVar5[6] = (int)"invalid distance code";
          goto switchD_0058be1a_caseD_9;
        }
        piVar2[3] = (uint)bVar1;
        pbVar6 = pbVar6 + *(int *)(pbVar6 + 4) * 8;
        goto LAB_0058c05a;
      }
      piVar2[2] = bVar1 & 0xf;
      piVar2[3] = *(int *)(pbVar6 + 4);
      *piVar2 = 4;
      break;
    case 4:
      for (; param_1 < (uint)piVar2[2]; param_1 = param_1 + 8) {
        if (local_c == 0) goto LAB_0058c29e;
        param_3 = 0;
        local_c = local_c - 1;
        param_2 = (int *)((uint)param_2 | (uint)*local_8 << ((byte)param_1 & 0x1f));
        local_8 = local_8 + 1;
      }
      piVar2[3] = piVar2[3] + (*(uint *)(&DAT_005c98e0 + piVar2[2] * 4) & (uint)param_2);
      param_2 = (int *)((uint)param_2 >> ((byte)piVar2[2] & 0x1f));
      param_1 = param_1 - piVar2[2];
      *piVar2 = 5;
    case 5:
      if ((uint)((int)puVar10 - *(int *)(iVar4 + 0x28)) < (uint)piVar2[3]) {
        iVar7 = (*(int *)(iVar4 + 0x2c) - *(int *)(iVar4 + 0x28)) - piVar2[3];
      }
      else {
        iVar7 = -piVar2[3];
      }
      local_18 = puVar10 + iVar7;
      while (piVar2[1] != 0) {
        puVar9 = puVar10;
        if (local_10 == (undefined1 *)0x0) {
          if (puVar10 == *(undefined1 **)(iVar4 + 0x2c)) {
            local_10 = *(undefined1 **)(iVar4 + 0x30);
            puVar9 = *(undefined1 **)(iVar4 + 0x28);
            if (local_10 != puVar9) {
              if (puVar9 < local_10) {
                local_10 = local_10 + (-1 - (int)puVar9);
              }
              else {
                local_10 = *(undefined1 **)(iVar4 + 0x2c) + -(int)puVar9;
              }
              puVar10 = puVar9;
              if (local_10 != (undefined1 *)0x0) goto LAB_0058c171;
            }
          }
          *(undefined1 **)(iVar4 + 0x34) = puVar10;
          param_3 = FUN_0058c880(iVar4,piVar5,param_3);
          puVar9 = *(undefined1 **)(iVar4 + 0x34);
          puVar10 = *(undefined1 **)(iVar4 + 0x30);
          if (puVar9 < puVar10) {
            local_10 = puVar10 + (-1 - (int)puVar9);
          }
          else {
            local_10 = (undefined1 *)(*(int *)(iVar4 + 0x2c) - (int)puVar9);
          }
          if ((puVar9 == *(undefined1 **)(iVar4 + 0x2c)) &&
             (puVar3 = *(undefined1 **)(iVar4 + 0x28), puVar10 != puVar3)) {
            puVar9 = puVar3;
            if (puVar3 < puVar10) {
              local_10 = puVar10 + (-1 - (int)puVar3);
            }
            else {
              local_10 = *(undefined1 **)(iVar4 + 0x2c) + -(int)puVar3;
            }
          }
          if (local_10 == (undefined1 *)0x0) goto LAB_0058c24e;
        }
LAB_0058c171:
        param_3 = 0;
        *puVar9 = *local_18;
        puVar10 = puVar9 + 1;
        local_18 = local_18 + 1;
        local_10 = local_10 + -1;
        if (local_18 == *(undefined1 **)(iVar4 + 0x2c)) {
          local_18 = *(undefined1 **)(iVar4 + 0x28);
        }
        piVar2[1] = piVar2[1] + -1;
      }
      *piVar2 = 0;
      break;
    case 6:
      if (local_10 != (undefined1 *)0x0) goto LAB_0058c230;
      if (puVar10 == *(undefined1 **)(iVar4 + 0x2c)) {
        local_10 = *(undefined1 **)(iVar4 + 0x30);
        puVar9 = *(undefined1 **)(iVar4 + 0x28);
        if (local_10 != puVar9) {
          if (puVar9 < local_10) {
            local_10 = local_10 + (-1 - (int)puVar9);
          }
          else {
            local_10 = *(undefined1 **)(iVar4 + 0x2c) + -(int)puVar9;
          }
          puVar10 = puVar9;
          if (local_10 != (undefined1 *)0x0) goto LAB_0058c230;
        }
      }
      *(undefined1 **)(iVar4 + 0x34) = puVar10;
      param_3 = FUN_0058c880(iVar4,piVar5,param_3);
      puVar9 = *(undefined1 **)(iVar4 + 0x34);
      puVar10 = *(undefined1 **)(iVar4 + 0x30);
      if (puVar9 < puVar10) {
        local_10 = puVar10 + (-1 - (int)puVar9);
      }
      else {
        local_10 = (undefined1 *)(*(int *)(iVar4 + 0x2c) - (int)puVar9);
      }
      if ((puVar9 == *(undefined1 **)(iVar4 + 0x2c)) &&
         (puVar3 = *(undefined1 **)(iVar4 + 0x28), puVar10 != puVar3)) {
        puVar9 = puVar3;
        if (puVar3 < puVar10) {
          local_10 = puVar10 + (-1 - (int)puVar3);
        }
        else {
          local_10 = *(undefined1 **)(iVar4 + 0x2c) + -(int)puVar3;
        }
      }
      if (local_10 == (undefined1 *)0x0) goto LAB_0058c24e;
LAB_0058c230:
      param_3 = 0;
      *puVar9 = (char)piVar2[2];
      puVar10 = puVar9 + 1;
      local_10 = local_10 + -1;
      *piVar2 = 0;
      break;
    case 7:
      if (7 < param_1) {
        param_1 = param_1 - 8;
        local_c = local_c + 1;
        local_8 = local_8 + -1;
      }
      *(undefined1 **)(iVar4 + 0x34) = puVar10;
      param_3 = FUN_0058c880(iVar4,piVar5,param_3);
      puVar10 = *(undefined1 **)(iVar4 + 0x34);
      if (*(undefined1 **)(iVar4 + 0x30) == puVar10) {
        *piVar2 = 8;
switchD_0058be1a_caseD_8:
        param_3 = 1;
        puVar9 = puVar10;
LAB_0058c24e:
        *(int **)(iVar4 + 0x20) = param_2;
        *(uint *)(iVar4 + 0x1c) = param_1;
        piVar5[1] = local_c;
        puVar10 = puVar9;
LAB_0058c260:
        iVar7 = *piVar5;
        *piVar5 = (int)local_8;
        piVar5[2] = (int)(local_8 + (piVar5[2] - iVar7));
      }
      else {
        *(int **)(iVar4 + 0x20) = param_2;
        *(uint *)(iVar4 + 0x1c) = param_1;
        piVar5[1] = local_c;
        iVar7 = *piVar5;
        *piVar5 = (int)local_8;
        piVar5[2] = (int)(local_8 + (piVar5[2] - iVar7));
      }
      *(undefined1 **)(iVar4 + 0x34) = puVar10;
      FUN_0058c880(iVar4,piVar5,param_3);
      return;
    case 8:
      goto switchD_0058be1a_caseD_8;
    case 9:
      goto switchD_0058be1a_caseD_9;
    default:
      param_3 = -2;
      goto LAB_0058c24e;
    }
  } while( true );
LAB_0058c29e:
  *(int **)(iVar4 + 0x20) = param_2;
  *(uint *)(iVar4 + 0x1c) = param_1;
  piVar5[1] = 0;
  goto LAB_0058c260;
}

