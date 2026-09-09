// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00593539 | Name: FUN_00593539


void FUN_00593539(int *param_1,int param_2,undefined4 param_3,undefined4 param_4,int param_5,
                 char param_6,undefined4 param_7,undefined4 param_8)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  int *piVar4;
  uint local_1c;
  uint local_18;
  int local_14;
  int local_10;
  int local_c;
  uint local_8;
  
  local_18 = local_18 & 0xffffff00;
  local_14 = *(int *)(param_2 + 8);
  if ((local_14 < -1) || (*(int *)(param_5 + 4) <= local_14)) {
    FUN_00593ebb();
  }
  if (*param_1 == -0x1f928c9d) {
    if (((param_1[4] == 3) && (param_1[5] == 0x19930520)) && (param_1[7] == 0)) {
      iVar2 = FUN_00593d53();
      if (*(int *)(iVar2 + 0x6c) == 0) {
        return;
      }
      iVar2 = FUN_00593d53();
      param_1 = *(int **)(iVar2 + 0x6c);
      iVar2 = FUN_00593d53();
      param_3 = *(undefined4 *)(iVar2 + 0x70);
      local_18 = CONCAT31(local_18._1_3_,1);
      iVar2 = FUN_0059ad16(param_1,1);
      if (iVar2 == 0) {
        FUN_00593ebb();
      }
      if (*param_1 != -0x1f928c9d) goto LAB_005936c1;
      if (((param_1[4] == 3) && (param_1[5] == 0x19930520)) && (param_1[7] == 0)) {
        FUN_00593ebb();
      }
    }
    iVar2 = local_14;
    if (((*param_1 == -0x1f928c9d) && (param_1[4] == 3)) && (param_1[5] == 0x19930520)) {
      piVar3 = (int *)FUN_0058ecb0(param_5,param_7,local_14,&local_8,&local_1c);
      do {
        if (local_1c <= local_8) {
          if (param_6 == '\0') {
            return;
          }
          FUN_00593c15(param_1,1);
          return;
        }
        if ((*piVar3 <= iVar2) && (iVar2 <= piVar3[1])) {
          iVar1 = piVar3[4];
          for (local_10 = piVar3[3]; iVar2 = local_14, 0 < local_10; local_10 = local_10 + -1) {
            piVar4 = *(int **)(param_1[7] + 0xc);
            for (local_c = *piVar4; 0 < local_c; local_c = local_c + -1) {
              piVar4 = piVar4 + 1;
              iVar2 = FUN_00593796(iVar1,*piVar4,param_1[7]);
              if (iVar2 != 0) {
                FUN_00593891(param_1,param_2,param_3,param_4,param_5,iVar1,*piVar4,piVar3,param_7,
                             param_8,local_18);
                iVar2 = local_14;
                goto LAB_005936a1;
              }
            }
            iVar1 = iVar1 + 0x10;
          }
        }
LAB_005936a1:
        local_8 = local_8 + 1;
        piVar3 = piVar3 + 5;
      } while( true );
    }
  }
LAB_005936c1:
  if (param_6 == '\0') {
    FUN_005936ec(param_1,param_2,param_3,param_4,param_5,local_14,param_7,param_8);
    return;
  }
  FUN_00593e5a();
  return;
}

