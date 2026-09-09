// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00594029 | Name: FUN_00594029


int FUN_00594029(undefined4 param_1,byte *param_2,undefined4 *param_3)

{
  byte *pbVar1;
  short sVar2;
  uint uVar3;
  short *psVar4;
  undefined4 uVar5;
  int *piVar6;
  int iVar7;
  byte bVar8;
  int iVar9;
  short *psVar10;
  undefined1 *puVar11;
  ulonglong uVar12;
  longlong lVar13;
  short local_250 [255];
  undefined1 local_51;
  undefined4 local_50;
  undefined4 local_4c;
  undefined1 local_48 [4];
  undefined4 local_44;
  int local_40;
  int local_3c;
  int local_38;
  short *local_34;
  int local_30;
  int local_2c;
  int local_28;
  undefined8 local_24;
  undefined1 local_1a;
  char local_19;
  int local_18;
  undefined1 *local_14;
  short *local_10;
  int local_c;
  uint local_8;
  
  local_40 = 0;
  local_14 = (undefined1 *)0x0;
  bVar8 = *param_2;
  local_18 = 0;
  local_34 = (short *)0x0;
  pbVar1 = param_2;
  do {
    if ((bVar8 == 0) || (param_2 = pbVar1 + 1, local_18 < 0)) {
      return local_18;
    }
    if (((char)bVar8 < ' ') || ('x' < (char)bVar8)) {
      uVar3 = 0;
    }
    else {
      uVar3 = *(byte *)((int)&PTR_LAB_005aa0f0 + (int)(char)bVar8) & 0xf;
    }
    local_40 = (int)(char)(&DAT_005aa110)[uVar3 * 8 + local_40] >> 4;
    switch(local_40) {
    case 0:
switchD_005940a5_caseD_0:
      local_30 = 0;
      if ((PTR_DAT_005cc110[(uint)bVar8 * 2 + 1] & 0x80) != 0) {
        FUN_005947c7((int)(char)bVar8,param_1,&local_18);
        bVar8 = *param_2;
        param_2 = pbVar1 + 2;
      }
      FUN_005947c7((int)(char)bVar8,param_1,&local_18);
      break;
    case 1:
      local_c = -1;
      local_44 = 0;
      local_3c = 0;
      local_2c = 0;
      local_28 = 0;
      local_8 = 0;
      local_30 = 0;
      break;
    case 2:
      if (bVar8 == 0x20) {
        local_8 = local_8 | 2;
      }
      else if (bVar8 == 0x23) {
        local_8 = local_8 | 0x80;
      }
      else if (bVar8 == 0x2b) {
        local_8 = local_8 | 1;
      }
      else if (bVar8 == 0x2d) {
        local_8 = local_8 | 4;
      }
      else if (bVar8 == 0x30) {
        local_8 = local_8 | 8;
      }
      break;
    case 3:
      if (bVar8 == 0x2a) {
        local_2c = FUN_00594865(&param_3);
        if (local_2c < 0) {
          local_8 = local_8 | 4;
          local_2c = -local_2c;
        }
      }
      else {
        local_2c = (char)bVar8 + -0x30 + local_2c * 10;
      }
      break;
    case 4:
      local_c = 0;
      break;
    case 5:
      if (bVar8 == 0x2a) {
        local_c = FUN_00594865(&param_3);
        if (local_c < 0) {
          local_c = -1;
        }
      }
      else {
        local_c = (char)bVar8 + -0x30 + local_c * 10;
      }
      break;
    case 6:
      if (bVar8 == 0x49) {
        if ((*param_2 != 0x36) || (pbVar1[2] != 0x34)) {
          local_40 = 0;
          goto switchD_005940a5_caseD_0;
        }
        param_2 = pbVar1 + 3;
        local_8 = local_8 | 0x8000;
      }
      else if (bVar8 == 0x68) {
        local_8 = local_8 | 0x20;
      }
      else if (bVar8 == 0x6c) {
        local_8 = local_8 | 0x10;
      }
      else if (bVar8 == 0x77) {
        local_8 = local_8 | 0x800;
      }
      break;
    case 7:
      psVar10 = local_10;
      if ((char)bVar8 < 'h') {
        if ((char)bVar8 < 'e') {
          if ((char)bVar8 < 'Y') {
            if (bVar8 != 0x58) {
              if (bVar8 == 0x43) {
                if ((local_8 & 0x830) == 0) {
                  local_8 = local_8 | 0x800;
                }
                goto LAB_005942fa;
              }
              if ((bVar8 != 0x45) && (bVar8 != 0x47)) {
                if (bVar8 == 0x53) {
                  if ((local_8 & 0x830) == 0) {
                    local_8 = local_8 | 0x800;
                  }
                  goto LAB_0059426a;
                }
                goto LAB_0059467d;
              }
              local_44 = 1;
              bVar8 = bVar8 + 0x20;
              goto LAB_005942cc;
            }
LAB_005944d9:
            local_38 = 7;
LAB_005944e0:
            local_14 = (undefined1 *)0x10;
            if ((local_8 & 0x80) != 0) {
              local_1a = 0x30;
              local_19 = (char)local_38 + 'Q';
              local_28 = 2;
            }
            goto LAB_00594549;
          }
          if (bVar8 == 0x5a) {
            psVar4 = (short *)FUN_00594865(&param_3);
            if ((psVar4 == (short *)0x0) ||
               (psVar10 = *(short **)(psVar4 + 2), psVar10 == (short *)0x0)) {
              local_10 = (short *)PTR_DAT_005c9d68;
              psVar10 = (short *)PTR_DAT_005c9d68;
              goto LAB_0059445a;
            }
            if ((local_8 & 0x800) == 0) {
              local_30 = 0;
              local_14 = (undefined1 *)(int)*psVar4;
            }
            else {
              local_30 = 1;
              local_14 = (undefined1 *)((uint)(int)*psVar4 >> 1);
            }
          }
          else if (bVar8 == 99) {
LAB_005942fa:
            if ((local_8 & 0x810) == 0) {
              local_250[0]._0_1_ = FUN_00594865(&param_3);
              local_14 = (undefined1 *)0x1;
            }
            else {
              uVar5 = FUN_00594882();
              local_14 = (undefined1 *)FUN_0059adea(local_250,uVar5);
              if ((int)local_14 < 0) {
                local_3c = 1;
              }
            }
            psVar10 = local_250;
          }
          else if (bVar8 == 100) goto LAB_0059453e;
        }
        else {
LAB_005942cc:
          local_8 = local_8 | 0x40;
          psVar10 = local_250;
          psVar4 = local_250;
          if (local_c < 0) {
            local_c = 6;
          }
          else if (local_c == 0) {
            if (bVar8 == 0x67) {
              local_c = 1;
            }
          }
          else {
            if (0x200 < local_c) {
              local_c = 0x200;
            }
            if (0xa3 < local_c) {
              local_10 = local_250;
              local_34 = _malloc(local_c + 0x15d);
              psVar10 = local_34;
              psVar4 = local_34;
              if (local_34 == (short *)0x0) {
                local_c = 0xa3;
                psVar10 = local_250;
                psVar4 = local_10;
              }
            }
          }
          local_10 = psVar4;
          local_50 = *param_3;
          local_4c = param_3[1];
          param_3 = param_3 + 2;
          (*(code *)PTR_FUN_005c9d3c)(&local_50,psVar10,(int)(char)bVar8,local_c,local_44);
          uVar3 = local_8 & 0x80;
          if ((uVar3 != 0) && (local_c == 0)) {
            (*(code *)PTR_FUN_005c9d48)(psVar10);
          }
          if ((bVar8 == 0x67) && (uVar3 == 0)) {
            (*(code *)PTR_FUN_005c9d40)(psVar10);
          }
          if ((char)*psVar10 == '-') {
            local_8 = local_8 | 0x100;
            psVar10 = (short *)((int)psVar10 + 1);
            local_10 = psVar10;
          }
LAB_0059445a:
          local_14 = (undefined1 *)_strlen((char *)psVar10);
          psVar10 = local_10;
        }
LAB_0059467d:
        local_10 = psVar10;
        uVar3 = local_8;
        if (local_3c == 0) {
          if ((local_8 & 0x40) != 0) {
            if ((local_8 & 0x100) == 0) {
              if ((local_8 & 1) == 0) {
                if ((local_8 & 2) == 0) goto LAB_005946b5;
                local_1a = 0x20;
              }
              else {
                local_1a = 0x2b;
              }
            }
            else {
              local_1a = 0x2d;
            }
            local_28 = 1;
          }
LAB_005946b5:
          iVar9 = (local_2c - local_28) - (int)local_14;
          if ((local_8 & 0xc) == 0) {
            FUN_005947fc(0x20,iVar9,param_1,&local_18);
          }
          FUN_0059482d(&local_1a,local_28,param_1,&local_18);
          if (((uVar3 & 8) != 0) && ((uVar3 & 4) == 0)) {
            FUN_005947fc(0x30,iVar9,param_1,&local_18);
          }
          if ((local_30 == 0) || (puVar11 = local_14, psVar10 = local_10, (int)local_14 < 1)) {
            FUN_0059482d(local_10,local_14,param_1,&local_18);
          }
          else {
            do {
              puVar11 = puVar11 + -1;
              iVar7 = FUN_0059adea(local_48,*psVar10);
              if (iVar7 < 1) break;
              FUN_0059482d(local_48,iVar7,param_1,&local_18);
              psVar10 = psVar10 + 1;
            } while (puVar11 != (undefined1 *)0x0);
          }
          if ((local_8 & 4) != 0) {
            FUN_005947fc(0x20,iVar9,param_1,&local_18);
          }
        }
      }
      else {
        if (bVar8 == 0x69) {
LAB_0059453e:
          local_8 = local_8 | 0x40;
LAB_00594542:
          local_14 = (undefined1 *)0xa;
LAB_00594549:
          if ((local_8 & 0x8000) == 0) {
            if ((local_8 & 0x20) == 0) {
              if ((local_8 & 0x40) == 0) {
                uVar3 = FUN_00594865(&param_3);
                uVar12 = (ulonglong)uVar3;
                goto LAB_0059459c;
              }
              uVar3 = FUN_00594865(&param_3);
            }
            else if ((local_8 & 0x40) == 0) {
              uVar3 = FUN_00594865(&param_3);
              uVar3 = uVar3 & 0xffff;
            }
            else {
              sVar2 = FUN_00594865(&param_3);
              uVar3 = (uint)sVar2;
            }
            uVar12 = (ulonglong)(int)uVar3;
          }
          else {
            uVar12 = FUN_00594872(&param_3);
          }
LAB_0059459c:
          if ((((local_8 & 0x40) != 0) && ((longlong)uVar12 < 0x100000000)) &&
             ((longlong)uVar12 < 0)) {
            uVar12 = CONCAT44(-((int)(uVar12 >> 0x20) + (uint)((int)uVar12 != 0)),-(int)uVar12);
            local_8 = local_8 | 0x100;
          }
          local_24._4_4_ = (int)(uVar12 >> 0x20);
          local_24._0_4_ = (int)uVar12;
          if ((local_8 & 0x8000) == 0) {
            local_24._4_4_ = 0;
          }
          if (local_c < 0) {
            local_c = 1;
          }
          else {
            local_8 = local_8 & 0xfffffff7;
            if (0x200 < local_c) {
              local_c = 0x200;
            }
          }
          if ((int)local_24 == 0 && local_24._4_4_ == 0) {
            local_28 = 0;
          }
          local_10 = (short *)&local_51;
          while ((puVar11 = local_14, iVar9 = local_c + -1, 0 < local_c || (local_24 != 0))) {
            iVar7 = (int)local_14 >> 0x1f;
            local_c = iVar9;
            iVar9 = __aullrem(local_24,local_14,iVar7);
            iVar9 = iVar9 + 0x30;
            lVar13 = __aulldiv(local_24,puVar11,iVar7);
            if (0x39 < iVar9) {
              iVar9 = iVar9 + local_38;
            }
            psVar10 = (short *)((int)local_10 + -1);
            local_24 = lVar13;
            *(char *)local_10 = (char)iVar9;
            local_10 = psVar10;
          }
          iVar7 = -(int)local_10;
          local_14 = &local_51 + iVar7;
          psVar10 = (short *)((int)local_10 + 1);
          local_c = iVar9;
          if (((local_8 & 0x200) != 0) &&
             ((*(char *)psVar10 != '0' || (local_14 == (undefined1 *)0x0)))) {
            *(char *)local_10 = '0';
            local_14 = (undefined1 *)((int)&local_50 + iVar7);
            psVar10 = local_10;
          }
          goto LAB_0059467d;
        }
        if (bVar8 != 0x6e) {
          if (bVar8 == 0x6f) {
            local_14 = (undefined1 *)0x8;
            if ((local_8 & 0x80) != 0) {
              local_8 = local_8 | 0x200;
            }
            goto LAB_00594549;
          }
          if (bVar8 == 0x70) {
            local_c = 8;
            goto LAB_005944d9;
          }
          if (bVar8 == 0x73) {
LAB_0059426a:
            iVar9 = 0x7fffffff;
            if (local_c != -1) {
              iVar9 = local_c;
            }
            psVar4 = (short *)FUN_00594865(&param_3);
            if ((local_8 & 0x810) == 0) {
              psVar10 = psVar4;
              if (psVar4 == (short *)0x0) {
                psVar4 = (short *)PTR_DAT_005c9d68;
                psVar10 = (short *)PTR_DAT_005c9d68;
              }
              for (; (iVar9 != 0 && ((char)*psVar4 != '\0')); psVar4 = (short *)((int)psVar4 + 1)) {
                iVar9 = iVar9 + -1;
              }
              local_14 = (undefined1 *)((int)psVar4 - (int)psVar10);
            }
            else {
              if (psVar4 == (short *)0x0) {
                psVar4 = (short *)PTR_DAT_005c9d6c;
              }
              local_30 = 1;
              for (psVar10 = psVar4; (iVar9 != 0 && (*psVar10 != 0)); psVar10 = psVar10 + 1) {
                iVar9 = iVar9 + -1;
              }
              local_14 = (undefined1 *)((int)psVar10 - (int)psVar4 >> 1);
              psVar10 = psVar4;
            }
            goto LAB_0059467d;
          }
          if (bVar8 != 0x75) {
            if (bVar8 != 0x78) goto LAB_0059467d;
            local_38 = 0x27;
            goto LAB_005944e0;
          }
          goto LAB_00594542;
        }
        piVar6 = (int *)FUN_00594865(&param_3);
        if ((local_8 & 0x20) == 0) {
          *piVar6 = local_18;
        }
        else {
          *(undefined2 *)piVar6 = (undefined2)local_18;
        }
        local_3c = 1;
      }
      if (local_34 != (short *)0x0) {
        FUN_0058ffd8(local_34);
        local_34 = (short *)0x0;
      }
    }
    bVar8 = *param_2;
    pbVar1 = param_2;
  } while( true );
}

