// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00564be5 | Name: FUN_00564be5


undefined4 __thiscall FUN_00564be5(int *param_1,uint *param_2,uint param_3)

{
  uint *puVar1;
  byte bVar2;
  ushort uVar3;
  uint *puVar4;
  byte bVar5;
  uint uVar6;
  uint uVar7;
  void *pvVar8;
  undefined1 *puVar9;
  uint uVar10;
  byte *pbVar11;
  byte *pbVar12;
  int iVar13;
  uint uVar14;
  uint uVar15;
  byte *pbVar16;
  byte *pbVar17;
  uint uVar18;
  byte *pbVar19;
  byte *pbVar20;
  byte *pbVar21;
  byte *pbVar22;
  int local_1c;
  byte *local_14;
  
  puVar4 = param_2;
  if (param_3 < 0x28) {
    return 0x80004005;
  }
  uVar18 = *param_2;
  if (uVar18 < 0x28) {
    return 0x80004005;
  }
  uVar15 = param_2[1];
  uVar6 = param_2[2];
  if ((int)uVar6 < 1) {
    uVar6 = -uVar6;
  }
  uVar3 = *(ushort *)((int)param_2 + 0xe);
  uVar7 = param_2[8];
  if ((uVar3 < 9) && (uVar7 == 0)) {
    uVar7 = 1 << ((byte)uVar3 & 0x1f);
  }
  uVar7 = uVar18 + uVar7 * 4;
  if (param_3 < uVar7) {
    return 0x80004005;
  }
  if ((short)param_2[3] != 1) {
    return 0x80004005;
  }
  if ((((int)param_2[2] < 0) && (param_2[4] != 0)) && (param_2[4] != 3)) {
    return 0x80004005;
  }
  if (param_2[4] < 3) {
    if (((uVar3 == 1) || (uVar3 == 4)) || (uVar3 == 8)) {
      param_3 = 0x29;
      goto LAB_00564efb;
    }
    if (uVar3 == 0x10) goto LAB_00564eeb;
    if (uVar3 == 0x18) goto LAB_00564d9b;
    if (uVar3 != 0x20) {
      return 0x80004005;
    }
  }
  else {
    if (param_2[4] != 3) {
      return 0x80004001;
    }
    if (uVar18 < 0x6c) {
      param_3 = 0;
      puVar1 = (uint *)(uVar18 + (int)param_2);
      uVar18 = puVar1[2];
      uVar10 = puVar1[1];
      uVar14 = *puVar1;
    }
    else {
      param_3 = param_2[0xd];
      uVar18 = param_2[0xc];
      uVar10 = param_2[0xb];
      uVar14 = param_2[10];
    }
    uVar3 = *(ushort *)((int)param_2 + 0xe);
    if (uVar3 == 0x10) {
      if (uVar18 == 0xff) {
        if (uVar10 != 0xff) {
          return 0x80004005;
        }
        if (uVar14 != 0xff) {
          return 0x80004005;
        }
        if (param_3 != 0xff00) {
          return 0x80004005;
        }
        param_3 = 0x33;
        goto LAB_00564efb;
      }
      if (uVar18 != 0x1f) {
        if (uVar18 == 0xf) {
          if (uVar10 != 0xf0) {
            return 0x80004005;
          }
          if (uVar14 != 0xf00) {
            return 0x80004005;
          }
          if (param_3 == 0xf000) {
            param_3 = 0x1a;
          }
          else {
            if (param_3 != 0) {
              return 0x80004005;
            }
            param_3 = 0x1e;
          }
        }
        else {
          if (uVar18 != 3) {
            return 0x80004005;
          }
          if (uVar10 != 0x1c) {
            return 0x80004005;
          }
          if (uVar14 != 0xe0) {
            return 0x80004005;
          }
          if (param_3 != 0xff00) {
            return 0x80004005;
          }
          param_3 = 0x1d;
        }
        goto LAB_00564efb;
      }
      if (uVar10 == 0x7e0) {
        if (uVar14 != 0xf800) {
          return 0x80004005;
        }
        if (param_3 != 0) {
          return 0x80004005;
        }
        param_3 = 0x17;
        goto LAB_00564efb;
      }
      if (uVar10 != 0x3e0) {
        return 0x80004005;
      }
      if (uVar14 != 0x7c00) {
        return 0x80004005;
      }
      if (param_3 != 0) {
        if (param_3 != 0x8000) {
          return 0x80004005;
        }
        param_3 = 0x19;
        goto LAB_00564efb;
      }
LAB_00564eeb:
      param_3 = 0x18;
      goto LAB_00564efb;
    }
    if (uVar3 == 0x18) {
      if (uVar18 != 0xff) {
        return 0x80004005;
      }
      if (uVar10 != 0xff00) {
        return 0x80004005;
      }
      if (uVar14 != 0xff0000) {
        return 0x80004005;
      }
      if (param_3 != 0) {
        return 0x80004005;
      }
LAB_00564d9b:
      param_3 = 0x14;
      goto LAB_00564efb;
    }
    if (uVar3 != 0x20) {
      return 0x80004005;
    }
    if (uVar18 != 0xff) {
      if (uVar18 == 0x3ff00000) {
        if (uVar10 != 0xffc00) {
          return 0x80004005;
        }
        if (uVar14 != 0x3ff) {
          return 0x80004005;
        }
        if (param_3 != 0xc0000000) {
          return 0x80004005;
        }
        param_3 = 0x1f;
      }
      else {
        if (uVar18 != 0) {
          return 0x80004005;
        }
        if (uVar10 != 0xffff0000) {
          return 0x80004005;
        }
        if (uVar14 != 0xffff) {
          return 0x80004005;
        }
        if (param_3 != 0) {
          return 0x80004005;
        }
        param_3 = 0x22;
      }
      goto LAB_00564efb;
    }
    if (uVar10 != 0xff00) {
      return 0x80004005;
    }
    if (uVar14 != 0xff0000) {
      return 0x80004005;
    }
    if (param_3 != 0) {
      if (param_3 != 0xff000000) {
        return 0x80004005;
      }
      param_3 = 0x15;
      goto LAB_00564efb;
    }
  }
  param_3 = 0x16;
LAB_00564efb:
  if ((param_1[0x10] != 0) && (param_3 == 0x29)) {
    uVar18 = param_2[8];
    if (uVar18 == 0) {
      uVar18 = 1 << ((byte)uVar3 & 0x1f);
    }
    param_1[0xf] = 1;
    pvVar8 = operator_new(0x400);
    param_1[2] = (int)pvVar8;
    if (pvVar8 == (void *)0x0) {
      return 0x8007000e;
    }
    uVar10 = *param_2;
    param_2 = (uint *)0x0;
    puVar9 = (undefined1 *)(uVar10 + (int)puVar4);
    if (uVar18 != 0) {
      do {
        iVar13 = (int)param_2 * 4;
        *(undefined1 *)(iVar13 + param_1[2]) = puVar9[2];
        *(undefined1 *)(iVar13 + 1 + param_1[2]) = puVar9[1];
        *(undefined1 *)(iVar13 + 2 + param_1[2]) = *puVar9;
        *(undefined1 *)(iVar13 + 3 + param_1[2]) = 0xff;
        param_2 = (uint *)((int)param_2 + 1);
        puVar9 = puVar9 + 4;
      } while (param_2 < uVar18);
    }
    for (; uVar18 < 0x100; uVar18 = uVar18 + 1) {
      iVar13 = uVar18 * 4;
      *(undefined1 *)(iVar13 + param_1[2]) = 0xff;
      *(undefined1 *)(iVar13 + 1 + param_1[2]) = 0xff;
      *(undefined1 *)(iVar13 + 2 + param_1[2]) = 0xff;
      *(undefined1 *)(iVar13 + 3 + param_1[2]) = 0xff;
    }
  }
  uVar3 = *(ushort *)((int)puVar4 + 0xe);
  uVar18 = uVar15;
  if (uVar3 == 1) {
    uVar10 = uVar15 + 7 >> 3;
  }
  else if (uVar3 == 4) {
    uVar10 = uVar15 + 1 >> 1;
  }
  else {
    uVar10 = (uVar3 >> 3) * uVar15;
    uVar18 = uVar10;
  }
  uVar14 = uVar10 + 3 & 0xfffffffc;
  *param_1 = param_3;
  uVar10 = uVar18 + 3 & 0xfffffffc;
  param_1[3] = uVar15;
  param_1[0xc] = uVar10;
  param_1[0xd] = 0;
  param_1[4] = uVar6;
  param_1[5] = 1;
  if (param_1[0x10] != 0) {
    if (((int)puVar4[2] < 0) && (7 < *(ushort *)((int)puVar4 + 0xe))) {
      param_1[0xe] = 0;
      param_1[1] = uVar7 + (int)puVar4;
    }
    else {
      param_1[0xe] = 1;
      pbVar11 = operator_new(uVar10 * uVar6);
      param_1[1] = (int)pbVar11;
      if (pbVar11 == (byte *)0x0) {
        return 0x8007000e;
      }
      pbVar16 = (byte *)(uVar7 + (int)puVar4);
      iVar13 = param_1[0xc];
      pbVar19 = pbVar11;
      local_1c = iVar13;
      if (-1 < (int)puVar4[2]) {
        pbVar19 = pbVar11 + (uVar6 - 1) * iVar13;
        local_1c = -iVar13;
      }
      pbVar12 = pbVar11 + iVar13 * uVar6;
      local_14 = pbVar19;
      if (puVar4[4] == 2) {
        while (pbVar11 <= pbVar19) {
          if (*pbVar16 == 0) {
            bVar5 = pbVar16[1];
            if (bVar5 == 0) {
              pbVar19 = local_14 + -param_1[0xc];
              local_14 = pbVar19;
            }
            else if (bVar5 == 1) {
              pbVar19 = pbVar11 + -param_1[0xc];
            }
            else if (bVar5 == 2) {
              pbVar12 = pbVar16 + 3;
              pbVar16 = pbVar16 + 2;
              pbVar19 = pbVar19 + ((uint)*pbVar16 - (uint)*pbVar12 * param_1[0xc]);
            }
            else {
              uVar18 = 0;
              if (bVar5 != 0) {
                do {
                  if ((uVar18 & 1) == 0) {
                    bVar5 = pbVar16[((int)uVar18 >> 1) + 2] >> 4;
                  }
                  else {
                    bVar5 = pbVar16[((int)uVar18 >> 1) + 2] & 0xf;
                  }
                  pbVar19[uVar18] = bVar5;
                  uVar18 = uVar18 + 1;
                } while ((int)uVar18 < (int)(uint)pbVar16[1]);
              }
              pbVar19 = pbVar19 + pbVar16[1];
              pbVar16 = pbVar16 + ((pbVar16[1] >> 1) + 1 & 0xfffffffe);
            }
          }
          else {
            uVar18 = 0;
            if (*pbVar16 != 0) {
              do {
                if ((uVar18 & 1) == 0) {
                  bVar5 = pbVar16[1] >> 4;
                }
                else {
                  bVar5 = pbVar16[1] & 0xf;
                }
                pbVar19[uVar18] = bVar5;
                uVar18 = uVar18 + 1;
              } while ((int)uVar18 < (int)(uint)*pbVar16);
            }
            pbVar19 = pbVar19 + *pbVar16;
          }
          pbVar16 = pbVar16 + 2;
        }
      }
      else if (puVar4[4] == 1) {
        while (pbVar11 <= pbVar19) {
          bVar5 = *pbVar16;
          if (bVar5 == 0) {
            uVar18 = (uint)pbVar16[1];
            if (uVar18 == 0) {
              pbVar19 = local_14 + -param_1[0xc];
              local_14 = pbVar19;
            }
            else if (uVar18 == 1) {
              pbVar19 = pbVar11 + -param_1[0xc];
            }
            else {
              if (uVar18 != 2) {
                pbVar12 = pbVar16 + 2;
                pbVar17 = pbVar19;
                for (uVar15 = (uint)(pbVar16[1] >> 2); uVar15 != 0; uVar15 = uVar15 - 1) {
                  *(undefined4 *)pbVar17 = *(undefined4 *)pbVar12;
                  pbVar12 = pbVar12 + 4;
                  pbVar17 = pbVar17 + 4;
                }
                for (uVar18 = uVar18 & 3; uVar18 != 0; uVar18 = uVar18 - 1) {
                  *pbVar17 = *pbVar12;
                  pbVar12 = pbVar12 + 1;
                  pbVar17 = pbVar17 + 1;
                }
                uVar18 = (uint)pbVar16[1];
                pbVar16 = pbVar16 + (uVar18 + 1 & 0xfffffffe);
                goto LAB_005651fb;
              }
              pbVar12 = pbVar16 + 3;
              pbVar16 = pbVar16 + 2;
              pbVar19 = pbVar19 + ((uint)*pbVar16 - (uint)*pbVar12 * param_1[0xc]);
            }
          }
          else {
            bVar2 = pbVar16[1];
            pbVar12 = pbVar19;
            for (uVar18 = (uint)(bVar5 >> 2); uVar18 != 0; uVar18 = uVar18 - 1) {
              *(uint *)pbVar12 = CONCAT22(CONCAT11(bVar2,bVar2),CONCAT11(bVar2,bVar2));
              pbVar12 = pbVar12 + 4;
            }
            for (uVar18 = bVar5 & 3; uVar18 != 0; uVar18 = uVar18 - 1) {
              *pbVar12 = bVar2;
              pbVar12 = pbVar12 + 1;
            }
            uVar18 = (uint)*pbVar16;
LAB_005651fb:
            pbVar19 = pbVar19 + uVar18;
          }
          pbVar16 = pbVar16 + 2;
        }
      }
      else if (*(short *)((int)puVar4 + 0xe) == 1) {
        for (; (pbVar11 <= pbVar19 && (pbVar19 < pbVar12)); pbVar19 = pbVar19 + local_1c) {
          param_2 = (uint *)0x0;
          if (uVar15 != 0) {
            do {
              pbVar19[(int)param_2] =
                   pbVar16[(uint)param_2 >> 3] >> (7 - ((byte)param_2 & 7) & 0x1f) & 1;
              param_2 = (uint *)((int)param_2 + 1);
            } while (param_2 < uVar15);
          }
          pbVar16 = pbVar16 + uVar14;
        }
      }
      else if (*(short *)((int)puVar4 + 0xe) == 4) {
        for (; (pbVar11 <= pbVar19 && (pbVar19 < pbVar12)); pbVar19 = pbVar19 + local_1c) {
          uVar18 = 0;
          if (uVar15 != 0) {
            do {
              if ((uVar18 & 1) == 0) {
                bVar5 = pbVar16[uVar18 >> 1] >> 4;
              }
              else {
                bVar5 = pbVar16[uVar18 >> 1] & 0xf;
              }
              pbVar19[uVar18] = bVar5;
              uVar18 = uVar18 + 1;
            } while (uVar18 < uVar15);
          }
          pbVar16 = pbVar16 + uVar14;
        }
      }
      else {
        while ((pbVar11 <= pbVar19 && (pbVar19 < pbVar12))) {
          pbVar17 = pbVar16 + uVar14;
          pbVar20 = pbVar19 + local_1c;
          for (uVar15 = uVar18 >> 2; uVar15 != 0; uVar15 = uVar15 - 1) {
            *(undefined4 *)pbVar19 = *(undefined4 *)pbVar16;
            pbVar16 = pbVar16 + 4;
            pbVar19 = pbVar19 + 4;
          }
          pbVar21 = pbVar16;
          pbVar22 = pbVar19;
          for (uVar15 = uVar18 & 3; pbVar16 = pbVar17, pbVar19 = pbVar20, uVar15 != 0;
              uVar15 = uVar15 - 1) {
            *pbVar22 = *pbVar21;
            pbVar21 = pbVar21 + 1;
            pbVar22 = pbVar22 + 1;
          }
        }
      }
    }
  }
  return 0;
}

