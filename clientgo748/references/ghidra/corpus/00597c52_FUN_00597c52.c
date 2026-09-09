// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00597c52 | Name: FUN_00597c52


LONG FUN_00597c52(undefined4 param_1,_EXCEPTION_POINTERS *param_2)

{
  code *pcVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  int iVar4;
  int *piVar5;
  LONG LVar6;
  int iVar7;
  int iVar8;
  
  iVar4 = FUN_00593d53();
  piVar5 = (int *)FUN_00597d90(param_1,*(undefined4 *)(iVar4 + 0x50));
  if ((piVar5 == (int *)0x0) || (pcVar1 = (code *)piVar5[2], pcVar1 == (code *)0x0)) {
    LVar6 = UnhandledExceptionFilter(param_2);
  }
  else if (pcVar1 == (code *)0x5) {
    piVar5[2] = 0;
    LVar6 = 1;
  }
  else {
    if (pcVar1 != (code *)0x1) {
      uVar2 = *(undefined4 *)(iVar4 + 0x54);
      *(_EXCEPTION_POINTERS **)(iVar4 + 0x54) = param_2;
      if (piVar5[1] == 8) {
        if (DAT_005cc0a8 < DAT_005cc0ac + DAT_005cc0a8) {
          iVar7 = DAT_005cc0a8 * 0xc;
          iVar8 = DAT_005cc0a8;
          do {
            *(undefined4 *)(iVar7 + 8 + *(int *)(iVar4 + 0x50)) = 0;
            iVar8 = iVar8 + 1;
            iVar7 = iVar7 + 0xc;
          } while (iVar8 < DAT_005cc0ac + DAT_005cc0a8);
        }
        iVar7 = *piVar5;
        uVar3 = *(undefined4 *)(iVar4 + 0x58);
        if (iVar7 == -0x3fffff72) {
          *(undefined4 *)(iVar4 + 0x58) = 0x83;
        }
        else if (iVar7 == -0x3fffff70) {
          *(undefined4 *)(iVar4 + 0x58) = 0x81;
        }
        else if (iVar7 == -0x3fffff6f) {
          *(undefined4 *)(iVar4 + 0x58) = 0x84;
        }
        else if (iVar7 == -0x3fffff6d) {
          *(undefined4 *)(iVar4 + 0x58) = 0x85;
        }
        else if (iVar7 == -0x3fffff73) {
          *(undefined4 *)(iVar4 + 0x58) = 0x82;
        }
        else if (iVar7 == -0x3fffff71) {
          *(undefined4 *)(iVar4 + 0x58) = 0x86;
        }
        else if (iVar7 == -0x3fffff6e) {
          *(undefined4 *)(iVar4 + 0x58) = 0x8a;
        }
        (*pcVar1)(8,*(undefined4 *)(iVar4 + 0x58));
        *(undefined4 *)(iVar4 + 0x58) = uVar3;
      }
      else {
        piVar5[2] = 0;
        (*pcVar1)(piVar5[1]);
      }
      *(undefined4 *)(iVar4 + 0x54) = uVar2;
    }
    LVar6 = -1;
  }
  return LVar6;
}

