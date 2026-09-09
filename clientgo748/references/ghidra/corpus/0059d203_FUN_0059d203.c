// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059d203 | Name: FUN_0059d203


undefined4
FUN_0059d203(int param_1,uint param_2,uint param_3,int param_4,byte param_5,short *param_6)

{
  short *psVar1;
  short *psVar2;
  char cVar3;
  uint uVar4;
  short *psVar5;
  short *psVar6;
  short sVar7;
  int iVar8;
  int iVar9;
  char *pcVar10;
  undefined1 local_20;
  undefined1 local_1f;
  undefined1 local_1e;
  undefined1 local_1d;
  undefined1 local_1c;
  undefined1 local_1b;
  undefined1 local_1a;
  undefined1 local_19;
  undefined1 local_18;
  undefined1 local_17;
  undefined1 local_16;
  undefined1 local_15;
  undefined2 local_14;
  undefined2 local_12;
  undefined2 uStack_10;
  undefined2 local_e;
  undefined2 uStack_c;
  undefined1 local_a;
  char cStack_9;
  undefined4 local_8;
  
  psVar2 = param_6;
  uVar4 = param_3 & 0x7fff;
  local_20 = 0xcc;
  local_1f = 0xcc;
  local_1e = 0xcc;
  local_1d = 0xcc;
  local_1c = 0xcc;
  local_1b = 0xcc;
  local_1a = 0xcc;
  local_19 = 0xcc;
  local_18 = 0xcc;
  local_17 = 0xcc;
  local_16 = 0xfb;
  local_15 = 0x3f;
  local_8 = 1;
  if ((param_3 & 0x8000) == 0) {
    *(undefined1 *)(param_6 + 1) = 0x20;
  }
  else {
    *(undefined1 *)(param_6 + 1) = 0x2d;
  }
  if ((((short)uVar4 != 0) || (param_2 != 0)) || (param_1 != 0)) {
    if ((short)uVar4 == 0x7fff) {
      *param_6 = 1;
      if (((param_2 == 0x80000000) && (param_1 == 0)) || ((param_2 & 0x40000000) != 0)) {
        if (((param_3 & 0x8000) == 0) || (param_2 != 0xc0000000)) {
          if ((param_2 != 0x80000000) || (param_1 != 0)) goto LAB_0059d2f8;
          pcVar10 = "1#INF";
        }
        else {
          if (param_1 != 0) {
LAB_0059d2f8:
            pcVar10 = "1#QNAN";
            goto LAB_0059d2fd;
          }
          pcVar10 = "1#IND";
        }
        FUN_0058ee20(param_6 + 2,pcVar10);
        *(undefined1 *)((int)psVar2 + 3) = 5;
      }
      else {
        pcVar10 = "1#SNAN";
LAB_0059d2fd:
        FUN_0058ee20(param_6 + 2,pcVar10);
        *(undefined1 *)((int)psVar2 + 3) = 6;
      }
      return 0;
    }
    local_14 = 0;
    local_a = (undefined1)uVar4;
    cStack_9 = (char)(uVar4 >> 8);
    local_e = (undefined2)param_2;
    uStack_c = (undefined2)(param_2 >> 0x10);
    local_12 = (undefined2)param_1;
    uStack_10 = (undefined2)((uint)param_1 >> 0x10);
    sVar7 = (short)(((uVar4 >> 8) + (param_2 >> 0x18) * 2) * 0x4d + -0x134312f4 + uVar4 * 0x4d10 >>
                   0x10);
    FUN_0059d763(&local_14,-(int)sVar7,1);
    if (0x3ffe < CONCAT11(cStack_9,local_a)) {
      sVar7 = sVar7 + 1;
      FUN_0059d543(&local_14,&local_20);
    }
    *psVar2 = sVar7;
    iVar9 = param_4;
    if (((param_5 & 1) == 0) || (iVar9 = param_4 + sVar7, 0 < param_4 + sVar7)) {
      if (0x15 < iVar9) {
        iVar9 = 0x15;
      }
      iVar8 = CONCAT11(cStack_9,local_a) - 0x3ffe;
      local_a = 0;
      cStack_9 = '\0';
      param_6 = (short *)0x8;
      do {
        FUN_0059cc10(&local_14);
        param_6 = (short *)((int)param_6 + -1);
      } while (param_6 != (short *)0x0);
      if (iVar8 < 0) {
        param_6 = (short *)0x0;
        for (uVar4 = -iVar8 & 0xff; uVar4 != 0; uVar4 = uVar4 - 1) {
          FUN_0059cc3e(&local_14);
        }
      }
      param_4 = iVar9 + 1;
      psVar5 = psVar2 + 2;
      param_6 = psVar5;
      if (0 < param_4) {
        do {
          param_1 = CONCAT22(local_12,local_14);
          param_2 = CONCAT22(local_e,uStack_10);
          param_3 = CONCAT13(cStack_9,CONCAT12(local_a,uStack_c));
          FUN_0059cc10(&local_14);
          FUN_0059cc10(&local_14);
          ___add_12(&local_14,&param_1);
          FUN_0059cc10(&local_14);
          cVar3 = cStack_9;
          cStack_9 = '\0';
          psVar5 = (short *)((int)param_6 + 1);
          param_4 = param_4 + -1;
          *(char *)param_6 = cVar3 + '0';
          param_6 = psVar5;
        } while (param_4 != 0);
      }
      psVar6 = psVar5 + -1;
      psVar1 = psVar2 + 2;
      if ('4' < *(char *)((int)psVar5 + -1)) {
        for (; psVar1 <= psVar6; psVar6 = (short *)((int)psVar6 + -1)) {
          if ((char)*psVar6 != '9') {
            if (psVar1 <= psVar6) goto LAB_0059d455;
            break;
          }
          *(char *)psVar6 = '0';
        }
        psVar6 = (short *)((int)psVar6 + 1);
        *psVar2 = *psVar2 + 1;
LAB_0059d455:
        *(char *)psVar6 = (char)*psVar6 + '\x01';
LAB_0059d457:
        cVar3 = ((char)psVar6 - (char)psVar2) + -3;
        *(char *)((int)psVar2 + 3) = cVar3;
        *(undefined1 *)(cVar3 + 4 + (int)psVar2) = 0;
        return local_8;
      }
      for (; psVar1 <= psVar6; psVar6 = (short *)((int)psVar6 + -1)) {
        if ((char)*psVar6 != '0') {
          if (psVar1 <= psVar6) goto LAB_0059d457;
          break;
        }
      }
      *psVar2 = 0;
      *(undefined1 *)(psVar2 + 1) = 0x20;
      *(undefined1 *)((int)psVar2 + 3) = 1;
      *(char *)psVar1 = '0';
      goto LAB_0059d48d;
    }
  }
  *psVar2 = 0;
  *(undefined1 *)(psVar2 + 1) = 0x20;
  *(undefined1 *)((int)psVar2 + 3) = 1;
  *(undefined1 *)(psVar2 + 2) = 0x30;
LAB_0059d48d:
  *(undefined1 *)((int)psVar2 + 5) = 0;
  return 1;
}

