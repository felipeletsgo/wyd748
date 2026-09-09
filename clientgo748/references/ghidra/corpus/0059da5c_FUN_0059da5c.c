// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059da5c | Name: FUN_0059da5c


undefined4 FUN_0059da5c(char *param_1,int param_2)

{
  char *pcVar1;
  int iVar2;
  int *piVar3;
  size_t sVar4;
  LPCSTR lpName;
  int *piVar5;
  bool bVar6;
  
  if (param_1 == (char *)0x0) {
    return 0xffffffff;
  }
  pcVar1 = (char *)FUN_005923dc(param_1,0x3d);
  if (pcVar1 == (char *)0x0) {
    return 0xffffffff;
  }
  if (param_1 == pcVar1) {
    return 0xffffffff;
  }
  bVar6 = pcVar1[1] == '\0';
  if (DAT_013b8004 == DAT_013b8008) {
    DAT_013b8004 = (int *)FUN_0059dc3b(DAT_013b8004);
  }
  if (DAT_013b8004 == (int *)0x0) {
    if ((param_2 == 0) || (DAT_013b800c == (undefined4 *)0x0)) {
      if (bVar6) {
        return 0;
      }
      DAT_013b8004 = _malloc(4);
      if (DAT_013b8004 == (int *)0x0) {
        return 0xffffffff;
      }
      *DAT_013b8004 = 0;
      if (DAT_013b800c == (undefined4 *)0x0) {
        DAT_013b800c = _malloc(4);
        if (DAT_013b800c == (undefined4 *)0x0) {
          return 0xffffffff;
        }
        *DAT_013b800c = 0;
      }
    }
    else {
      iVar2 = FUN_0059d4d5();
      if (iVar2 != 0) {
        return 0xffffffff;
      }
    }
  }
  piVar3 = DAT_013b8004;
  iVar2 = FUN_0059dbe3(param_1,(int)pcVar1 - (int)param_1);
  if ((iVar2 < 0) || (*piVar3 == 0)) {
    if (bVar6) {
      return 0;
    }
    if (iVar2 < 0) {
      iVar2 = -iVar2;
    }
    piVar3 = (int *)FUN_00599173(piVar3,iVar2 * 4 + 8);
    if (piVar3 == (int *)0x0) {
      return 0xffffffff;
    }
    piVar3[iVar2] = (int)param_1;
    piVar3[iVar2 + 1] = 0;
  }
  else {
    if (!bVar6) {
      piVar3[iVar2] = (int)param_1;
      goto LAB_0059db90;
    }
    piVar5 = piVar3 + iVar2;
    FUN_0058ffd8(piVar3[iVar2]);
    for (; *piVar5 != 0; piVar5 = piVar5 + 1) {
      iVar2 = iVar2 + 1;
      *piVar5 = piVar5[1];
    }
    piVar3 = (int *)FUN_00599173(piVar3,iVar2 << 2);
    if (piVar3 == (int *)0x0) goto LAB_0059db90;
  }
  DAT_013b8004 = piVar3;
LAB_0059db90:
  if (param_2 != 0) {
    sVar4 = _strlen(param_1);
    lpName = _malloc(sVar4 + 2);
    if (lpName != (LPCSTR)0x0) {
      FUN_0058ee20(lpName,param_1);
      pcVar1[(int)lpName - (int)param_1] = '\0';
      SetEnvironmentVariableA
                (lpName,(LPCSTR)(~-(uint)bVar6 & (uint)(pcVar1 + ((int)lpName - (int)param_1) + 1)))
      ;
      FUN_0058ffd8(lpName);
    }
  }
  return 0;
}

