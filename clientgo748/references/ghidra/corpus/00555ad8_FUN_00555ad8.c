// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00555ad8 | Name: FUN_00555ad8


undefined4
FUN_00555ad8(int param_1,int param_2,int *param_3,int *param_4,void *param_5,int param_6,int param_7
            ,int param_8)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int iVar12;
  int iVar13;
  undefined4 uVar14;
  undefined4 local_14;
  
  iVar4 = param_2;
  iVar3 = param_1;
  iVar1 = *param_3;
  iVar2 = *param_4;
  _memset(param_5,0,0x18);
  local_14 = 0;
  while( true ) {
    if ((param_6 <= local_14) || (0x16 < local_14)) goto LAB_0055635f;
    if ((param_1 - DAT_005b8924 < 1) ||
       (((param_2 - DAT_005b8928 < 1 || (DAT_005b891c + -2 < param_1 - DAT_005b8924)) ||
        (DAT_005b8920 + -2 < param_2 - DAT_005b8928)))) break;
    iVar5 = (int)*(char *)(param_7 +
                          (((param_2 - DAT_005b8928) * DAT_005b891c + param_1) - DAT_005b8924));
    iVar6 = (int)*(char *)(param_7 +
                          ((((param_2 - DAT_005b8928) + -1) * DAT_005b891c + param_1) - DAT_005b8924
                          ));
    iVar7 = (int)*(char *)(param_7 + 1 +
                          ((((param_2 - DAT_005b8928) + -1) * DAT_005b891c + param_1) - DAT_005b8924
                          ));
    iVar8 = (int)*(char *)(param_7 + 1 +
                          (((param_2 - DAT_005b8928) * DAT_005b891c + param_1) - DAT_005b8924));
    iVar9 = (int)*(char *)(param_7 + 1 +
                          ((((param_2 - DAT_005b8928) + 1) * DAT_005b891c + param_1) - DAT_005b8924)
                          );
    iVar10 = (int)*(char *)(param_7 +
                           ((((param_2 - DAT_005b8928) + 1) * DAT_005b891c + param_1) - DAT_005b8924
                           ));
    iVar11 = (int)*(char *)(param_7 + -1 +
                           ((((param_2 - DAT_005b8928) + 1) * DAT_005b891c + param_1) - DAT_005b8924
                           ));
    iVar12 = (int)*(char *)(param_7 + -1 +
                           (((param_2 - DAT_005b8928) * DAT_005b891c + param_1) - DAT_005b8924));
    iVar13 = (int)*(char *)(param_7 + -1 +
                           ((((param_2 - DAT_005b8928) + -1) * DAT_005b891c + param_1) -
                           DAT_005b8924));
    if ((iVar1 == param_1) && (iVar2 == param_2)) {
      *(undefined1 *)((int)param_5 + local_14) = 0;
      goto LAB_0055635f;
    }
    if ((iVar1 == param_1) &&
       (((param_2 < iVar2 && (iVar10 < iVar5 + param_8)) && (iVar5 - param_8 < iVar10)))) {
      *(undefined1 *)((int)param_5 + local_14) = 0x38;
      param_2 = param_2 + 1;
    }
    else if ((((iVar1 == param_1) && (iVar2 < param_2)) && (iVar6 < iVar5 + param_8)) &&
            (iVar5 - param_8 < iVar6)) {
      *(undefined1 *)((int)param_5 + local_14) = 0x32;
      param_2 = param_2 + -1;
    }
    else if (((param_1 < iVar1) && (iVar2 < param_2)) &&
            ((iVar7 < iVar5 + param_8 && (iVar5 - param_8 < iVar7)))) {
      *(undefined1 *)((int)param_5 + local_14) = 0x33;
      param_1 = param_1 + 1;
      param_2 = param_2 + -1;
    }
    else if (((param_1 < iVar1) && (iVar2 == param_2)) &&
            ((iVar8 < iVar5 + param_8 && (iVar5 - param_8 < iVar8)))) {
      *(undefined1 *)((int)param_5 + local_14) = 0x36;
      param_1 = param_1 + 1;
    }
    else if ((((param_1 < iVar1) && (param_2 < iVar2)) && (iVar9 < iVar5 + param_8)) &&
            (iVar5 - param_8 < iVar9)) {
      *(undefined1 *)((int)param_5 + local_14) = 0x39;
      param_1 = param_1 + 1;
      param_2 = param_2 + 1;
    }
    else if (((iVar1 < param_1) && (param_2 < iVar2)) &&
            ((iVar11 < iVar5 + param_8 && (iVar5 - param_8 < iVar11)))) {
      *(undefined1 *)((int)param_5 + local_14) = 0x37;
      param_1 = param_1 + -1;
      param_2 = param_2 + 1;
    }
    else if (((iVar1 < param_1) && (iVar2 == param_2)) &&
            ((iVar12 < iVar5 + param_8 && (iVar5 - param_8 < iVar12)))) {
      *(undefined1 *)((int)param_5 + local_14) = 0x34;
      param_1 = param_1 + -1;
    }
    else if ((((iVar1 < param_1) && (iVar2 < param_2)) && (iVar13 < iVar5 + param_8)) &&
            (iVar5 - param_8 < iVar13)) {
      *(undefined1 *)((int)param_5 + local_14) = 0x31;
      param_1 = param_1 + -1;
      param_2 = param_2 + -1;
    }
    else if (((param_1 < iVar1) && (iVar2 < param_2)) &&
            ((iVar8 < iVar5 + param_8 && (iVar5 - param_8 < iVar8)))) {
      *(undefined1 *)((int)param_5 + local_14) = 0x36;
      param_1 = param_1 + 1;
    }
    else if (((param_1 < iVar1) && (iVar2 < param_2)) &&
            ((iVar6 < iVar5 + param_8 && (iVar5 - param_8 < iVar6)))) {
      *(undefined1 *)((int)param_5 + local_14) = 0x32;
      param_2 = param_2 + -1;
    }
    else if ((((param_1 < iVar1) && (param_2 < iVar2)) && (iVar8 < iVar5 + param_8)) &&
            (iVar5 - param_8 < iVar8)) {
      *(undefined1 *)((int)param_5 + local_14) = 0x36;
      param_1 = param_1 + 1;
    }
    else if (((param_1 < iVar1) && (param_2 < iVar2)) &&
            ((iVar10 < iVar5 + param_8 && (iVar5 - param_8 < iVar10)))) {
      *(undefined1 *)((int)param_5 + local_14) = 0x38;
      param_2 = param_2 + 1;
    }
    else if ((((iVar1 < param_1) && (param_2 < iVar2)) && (iVar12 < iVar5 + param_8)) &&
            (iVar5 - param_8 < iVar12)) {
      *(undefined1 *)((int)param_5 + local_14) = 0x34;
      param_1 = param_1 + -1;
    }
    else if ((((iVar1 < param_1) && (param_2 < iVar2)) && (iVar10 < iVar5 + param_8)) &&
            (iVar5 - param_8 < iVar10)) {
      *(undefined1 *)((int)param_5 + local_14) = 0x38;
      param_2 = param_2 + 1;
    }
    else if (((iVar1 < param_1) && (iVar2 < param_2)) &&
            ((iVar12 < iVar5 + param_8 && (iVar5 - param_8 < iVar12)))) {
      *(undefined1 *)((int)param_5 + local_14) = 0x34;
      param_1 = param_1 + -1;
    }
    else if (((iVar1 < param_1) && (iVar2 < param_2)) &&
            ((iVar6 < iVar5 + param_8 && (iVar5 - param_8 < iVar6)))) {
      *(undefined1 *)((int)param_5 + local_14) = 0x32;
      param_2 = param_2 + -1;
    }
    else {
      if ((((iVar1 == param_1 + 1) || (iVar2 == param_2 + 1)) || (iVar1 == param_1 + -1)) ||
         (iVar2 == param_2 + -1)) {
        *(undefined1 *)((int)param_5 + local_14) = 0;
        goto LAB_0055635f;
      }
      if (((iVar1 == param_1) && (param_2 < iVar2)) &&
         ((iVar9 < iVar5 + param_8 && (iVar5 - param_8 < iVar9)))) {
        *(undefined1 *)((int)param_5 + local_14) = 0x39;
        param_1 = param_1 + 1;
        param_2 = param_2 + 1;
      }
      else if (((iVar1 == param_1) && (param_2 < iVar2)) &&
              ((iVar11 < iVar5 + param_8 && (iVar5 - param_8 < iVar11)))) {
        *(undefined1 *)((int)param_5 + local_14) = 0x37;
        param_1 = param_1 + -1;
        param_2 = param_2 + 1;
      }
      else if ((((iVar1 == param_1) && (iVar2 < param_2)) && (iVar7 < iVar5 + param_8)) &&
              (iVar5 - param_8 < iVar7)) {
        *(undefined1 *)((int)param_5 + local_14) = 0x33;
        param_1 = param_1 + 1;
        param_2 = param_2 + -1;
      }
      else if (((iVar1 == param_1) && (iVar2 < param_2)) &&
              ((iVar13 < iVar5 + param_8 && (iVar5 - param_8 < iVar13)))) {
        *(undefined1 *)((int)param_5 + local_14) = 0x31;
        param_1 = param_1 + -1;
        param_2 = param_2 + -1;
      }
      else if (((iVar1 < param_1) && (iVar2 == param_2)) &&
              ((iVar11 < iVar5 + param_8 && (iVar5 - param_8 < iVar11)))) {
        *(undefined1 *)((int)param_5 + local_14) = 0x37;
        param_1 = param_1 + -1;
        param_2 = param_2 + 1;
      }
      else if ((((iVar1 < param_1) && (iVar2 == param_2)) && (iVar13 < iVar5 + param_8)) &&
              (iVar5 - param_8 < iVar13)) {
        *(undefined1 *)((int)param_5 + local_14) = 0x31;
        param_1 = param_1 + -1;
        param_2 = param_2 + -1;
      }
      else if (((param_1 < iVar1) && (iVar2 == param_2)) &&
              ((iVar9 < iVar5 + param_8 && (iVar5 - param_8 < iVar9)))) {
        *(undefined1 *)((int)param_5 + local_14) = 0x39;
        param_1 = param_1 + 1;
        param_2 = param_2 + 1;
      }
      else {
        if (((iVar1 <= param_1) || (iVar2 != param_2)) ||
           ((iVar5 + param_8 <= iVar7 || (iVar7 <= iVar5 - param_8)))) {
          *(undefined1 *)((int)param_5 + local_14) = 0;
          goto LAB_0055635f;
        }
        *(undefined1 *)((int)param_5 + local_14) = 0x33;
        param_1 = param_1 + 1;
        param_2 = param_2 + -1;
      }
    }
    local_14 = local_14 + 1;
  }
  *(undefined1 *)((int)param_5 + local_14) = 0;
LAB_0055635f:
  if ((iVar3 == param_1) && (iVar4 == param_2)) {
    uVar14 = 0;
  }
  else {
    *param_3 = param_1;
    *param_4 = param_2;
    if ((iVar3 == param_1) && (iVar4 == param_2)) {
      uVar14 = 0;
    }
    else {
      uVar14 = 1;
    }
  }
  return uVar14;
}

