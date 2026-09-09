// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005951a4 | Name: FUN_005951a4


int FUN_005951a4(undefined4 param_1,int param_2)

{
  int iVar1;
  int *piVar2;
  undefined4 *puVar3;
  HANDLE hFile;
  BOOL BVar4;
  DWORD DVar5;
  DWORD *pDVar6;
  int iVar7;
  undefined1 local_1008 [4064];
  undefined4 uStackY_28;
  undefined4 uStackY_24;
  undefined4 uStackY_20;
  
  FUN_0058f730();
  iVar7 = 0;
  iVar1 = FUN_00595131();
  if ((iVar1 == -1) || (iVar1 = FUN_00595131(), iVar1 == -1)) {
    iVar7 = -1;
  }
  else {
    param_2 = param_2 - iVar1;
    if (param_2 < 1) {
      if (param_2 < 0) {
        uStackY_20 = 0x595275;
        FUN_00595131();
        uStackY_20 = param_1;
        uStackY_24 = 0x59527d;
        hFile = (HANDLE)FUN_00594e4d();
        BVar4 = SetEndOfFile(hFile);
        iVar7 = (BVar4 != 0) - 1;
        if (iVar7 == -1) {
          puVar3 = (undefined4 *)FUN_00594c1d();
          *puVar3 = 0xd;
          DVar5 = GetLastError();
          pDVar6 = (DWORD *)FUN_00594c26();
          *pDVar6 = DVar5;
        }
      }
    }
    else {
      uStackY_20 = 0x595204;
      _memset(local_1008,0,0x1000);
      uStackY_20 = 0x8000;
      uStackY_24 = param_1;
      uStackY_28 = 0x595211;
      FUN_0059af95();
      do {
        uStackY_20 = 0x59522f;
        iVar1 = FUN_00590bdd();
        if (iVar1 == -1) {
          piVar2 = (int *)FUN_00594c26();
          if (*piVar2 == 5) {
            puVar3 = (undefined4 *)FUN_00594c1d();
            *puVar3 = 0xd;
          }
          iVar7 = -1;
          break;
        }
        param_2 = param_2 - iVar1;
      } while (0 < param_2);
      FUN_0059af95();
    }
    uStackY_20 = 0x5952bb;
    FUN_00595131();
  }
  return iVar7;
}

