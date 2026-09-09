// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00406bd7 | Name: FUN_00406bd7


undefined4 __thiscall FUN_00406bd7(int *param_1,char param_2)

{
  char *pcVar1;
  undefined4 uVar2;
  int iVar3;
  LPSTR pCVar4;
  int iVar5;
  
  if (param_1[0xb] == 0) {
    uVar2 = 0;
  }
  else if (param_1[0xc] == 1) {
    if (param_2 == '\x1b') {
      if (DAT_013b722c == 0) {
        (**(code **)(*param_1 + 0x90))(&DAT_005ccef4);
        param_1[0xc] = 0;
        iVar3 = DAT_0067cf38;
        (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x40))(0);
        if (param_1[0x11] == 0x1403) {
          (**(code **)(*param_1 + 0x60))(0);
          if ((DAT_005b892c == 2) && (*(int *)(iVar3 + 0x27b28) != 0)) {
            (**(code **)(**(int **)(iVar3 + 0x27b28) + 0x60))(0);
          }
        }
      }
      uVar2 = 1;
    }
    else if (param_2 == '\r') {
      if (param_1[0x17] == 0) {
        uVar2 = 0;
      }
      else {
        uVar2 = (*(code *)**(undefined4 **)param_1[0x17])(param_1[0x11],0);
      }
    }
    else if (param_2 == '\t') {
      if (param_1[0x17] == 0) {
        uVar2 = 0;
      }
      else {
        uVar2 = (*(code *)**(undefined4 **)param_1[0x17])(param_1[0x11],1);
      }
    }
    else if (param_2 == '\b') {
      iVar3 = lstrlenA((LPCSTR)(param_1 + 0x3d3));
      if (iVar3 == 0) {
        iVar3 = lstrlenA((LPCSTR)(param_1 + 0x393));
        if (iVar3 == 0) {
          uVar2 = (*(code *)**(undefined4 **)param_1[0x17])(param_1[0x11],7);
        }
        else {
          pCVar4 = CharPrevA((LPCSTR)(param_1 + 0x393),(LPCSTR)((int)param_1 + iVar3 + 0xe4c));
          *pCVar4 = '\0';
          (**(code **)(*param_1 + 0x54))();
          uVar2 = 1;
        }
      }
      else {
        uVar2 = 1;
      }
    }
    else {
      iVar3 = lstrlenA((LPCSTR)(param_1 + 0x393));
      if (iVar3 < param_1[0x414]) {
        if ((char)param_1[0x413] != '\0') {
          *(undefined1 *)(param_1 + 0x413) = 0;
        }
        iVar3 = lstrlenA((LPCSTR)(param_1 + 0x393));
        pcVar1 = (char *)((int)param_1 + iVar3 + 0xe4c);
        *pcVar1 = param_2;
        pcVar1[1] = '\0';
        (**(code **)(*param_1 + 0x54))();
        uVar2 = 1;
      }
      else {
        iVar3 = lstrlenA((LPCSTR)(param_1 + 0x393));
        if (iVar3 == 0) {
          uVar2 = 1;
        }
        else {
          iVar5 = FUN_00421a90(param_1 + 0x393,iVar3 + -1);
          if (iVar5 == 0) {
            *(undefined1 *)((int)param_1 + iVar3 + 0xe4b) = 0;
            *(undefined1 *)(param_1 + 0x3d3) = 0;
          }
          (**(code **)(*param_1 + 0x54))();
          *(char *)(param_1 + 0x413) = param_2;
          uVar2 = 1;
        }
      }
    }
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}

