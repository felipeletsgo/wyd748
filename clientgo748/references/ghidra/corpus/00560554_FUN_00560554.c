// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00560554 | Name: FUN_00560554


uint FUN_00560554(int *param_1,undefined4 param_2,uint param_3,uint param_4)

{
  int *piVar1;
  uint uVar2;
  int **ppiVar3;
  uint uVar4;
  bool bVar5;
  undefined1 local_68 [20];
  uint local_54;
  uint local_50;
  uint local_4c;
  undefined1 local_48 [24];
  uint local_30;
  uint local_2c;
  uint local_28;
  uint local_24;
  int local_20;
  int *local_1c;
  uint local_18;
  int local_14;
  int *local_10;
  int *local_c;
  int *local_8;
  
  piVar1 = param_1;
  if ((param_1 == (int *)0x0) ||
     (((local_14 = (**(code **)(*param_1 + 0x28))(param_1), local_14 != 3 && (local_14 != 4)) &&
      (local_14 != 5)))) {
    return 0x8876086c;
  }
  local_1c = piVar1;
  uVar2 = 5;
  if (param_4 != 0xffffffff) goto LAB_005605f6;
  if (local_14 == 3) {
    (**(code **)(*piVar1 + 0x38))(piVar1,0,local_48);
    if (((local_30 & local_30 - 1) == 0) && (param_4 = uVar2, (local_2c & local_2c - 1) == 0))
    goto LAB_005605f6;
    param_4 = 0;
LAB_00560645:
    bVar5 = param_4 != 0;
    param_4 = uVar2;
    if (bVar5) goto LAB_005605f6;
  }
  else {
    if (local_14 == 4) {
      (**(code **)(*piVar1 + 0x38))(piVar1,0,local_68);
      if ((local_54 & local_54 - 1) != 0) goto LAB_005605ef;
      local_30 = local_50 & local_50 - 1;
    }
    else {
      if (local_14 != 5) goto LAB_00560645;
      (**(code **)(*piVar1 + 0x38))(piVar1,0,local_48);
      local_30 = local_30 & local_30 - 1;
      local_4c = local_2c;
    }
    if ((local_30 == 0) && (param_4 = uVar2, (local_4c & local_4c - 1) == 0)) goto LAB_005605f6;
  }
LAB_005605ef:
  param_4 = 0x80004;
LAB_005605f6:
  if (local_14 == 5) {
    param_4 = param_4 | 0x70000;
  }
  local_24 = (**(code **)(*piVar1 + 0x34))(piVar1);
  if (param_3 == 0xffffffff) {
    param_3 = 0;
  }
  if (param_3 < local_24) {
    param_1 = (int *)0x0;
    local_c = (int *)0x0;
    local_8 = (int *)0x0;
    local_10 = (int *)0x0;
    local_28 = ((local_14 != 5) - 1 & 5) + 1;
    if (((param_4 & 0xff) == 2) || (local_20 = 0, (param_4 & 0xff) == 5)) {
      local_20 = 1;
    }
    local_18 = 0;
    uVar2 = param_4;
    if (local_28 != 0) {
      do {
        if (local_14 == 3) {
          ppiVar3 = &param_1;
LAB_005606bf:
          uVar2 = (**(code **)(*local_1c + 0x3c))(local_1c,param_3,ppiVar3);
        }
        else {
          if (local_14 == 4) {
            ppiVar3 = &local_c;
            goto LAB_005606bf;
          }
          if (local_14 == 5) {
            uVar2 = (**(code **)(*local_1c + 0x3c))(local_1c,local_18,param_3,&param_1);
          }
        }
        uVar4 = param_3;
        if ((int)uVar2 < 0) goto LAB_005607f2;
LAB_005607b7:
        uVar4 = uVar4 + 1;
        if (uVar4 < local_24) {
          if (local_14 == 3) {
            ppiVar3 = &local_8;
LAB_00560707:
            uVar2 = (**(code **)(*local_1c + 0x3c))(local_1c,uVar4,ppiVar3);
          }
          else {
            if (local_14 == 4) {
              ppiVar3 = &local_10;
              goto LAB_00560707;
            }
            if (local_14 == 5) {
              uVar2 = (**(code **)(*local_1c + 0x3c))(local_1c,local_18,uVar4,&local_8);
            }
          }
          if (-1 < (int)uVar2) {
            if (local_14 == 3) {
LAB_0056072a:
              uVar2 = FUN_00560328(local_8,param_2,0,param_1,param_2,0,param_4,0);
            }
            else if (local_14 == 4) {
              uVar2 = FUN_005604ce(local_10,param_2,0,local_c,param_2,0,param_4,0);
            }
            else if (local_14 == 5) goto LAB_0056072a;
            if (-1 < (int)uVar2) {
              if (local_20 == 0) goto LAB_00560797;
              if (param_1 != (int *)0x0) {
                (**(code **)(*param_1 + 8))(param_1);
                param_1 = (int *)0x0;
              }
              if (local_c != (int *)0x0) {
                (**(code **)(*local_c + 8))(local_c);
              }
              param_1 = local_8;
              local_8 = (int *)0x0;
              local_c = local_10;
              goto LAB_005607b4;
            }
          }
          goto LAB_005607f2;
        }
        if (param_1 != (int *)0x0) {
          (**(code **)(*param_1 + 8))(param_1);
          param_1 = (int *)0x0;
        }
        if (local_c != (int *)0x0) {
          (**(code **)(*local_c + 8))(local_c);
          local_c = (int *)0x0;
        }
        local_18 = local_18 + 1;
      } while (local_18 < local_28);
    }
    uVar2 = 0;
LAB_005607f2:
    if (param_1 != (int *)0x0) {
      (**(code **)(*param_1 + 8))(param_1);
      param_1 = (int *)0x0;
    }
    if (local_c != (int *)0x0) {
      (**(code **)(*local_c + 8))(local_c);
      local_c = (int *)0x0;
    }
    if (local_8 != (int *)0x0) {
      (**(code **)(*local_8 + 8))(local_8);
      local_8 = (int *)0x0;
    }
    if (local_10 != (int *)0x0) {
      (**(code **)(*local_10 + 8))(local_10);
    }
  }
  else {
    uVar2 = 0x8876086c;
  }
  return uVar2;
LAB_00560797:
  if (local_8 != (int *)0x0) {
    (**(code **)(*local_8 + 8))(local_8);
    local_8 = (int *)0x0;
  }
  if (local_10 != (int *)0x0) {
    (**(code **)(*local_10 + 8))(local_10);
LAB_005607b4:
    local_10 = (int *)0x0;
  }
  goto LAB_005607b7;
}

