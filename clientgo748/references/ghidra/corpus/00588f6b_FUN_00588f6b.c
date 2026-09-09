// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00588f6b | Name: FUN_00588f6b


int FUN_00588f6b(int *param_1,int *param_2)

{
  int iVar1;
  int iVar2;
  bool bVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int *piVar8;
  int local_10;
  int local_c;
  
  iVar1 = param_1[0x15];
  iVar2 = param_1[0x19];
  iVar7 = 1;
  do {
    iVar5 = iVar7;
    iVar7 = iVar5 + 1;
    iVar4 = iVar7;
    if (1 < iVar2) {
      iVar6 = iVar2 + -1;
      do {
        iVar4 = iVar4 * iVar7;
        iVar6 = iVar6 + -1;
      } while (iVar6 != 0);
    }
  } while (iVar4 <= iVar1);
  if (iVar5 < 2) {
    *(undefined4 *)(*param_1 + 0x14) = 0x37;
    *(int *)(*param_1 + 0x18) = iVar4;
    (**(code **)*param_1)(param_1);
  }
  local_10 = 1;
  iVar7 = iVar2;
  piVar8 = param_2;
  if (0 < iVar2) {
    for (; iVar4 = iVar2, iVar7 != 0; iVar7 = iVar7 + -1) {
      *piVar8 = iVar5;
      piVar8 = piVar8 + 1;
    }
    do {
      local_10 = local_10 * iVar5;
      iVar4 = iVar4 + -1;
    } while (iVar4 != 0);
  }
  do {
    bVar3 = false;
    local_c = 0;
    if (iVar2 < 1) {
      return local_10;
    }
    do {
      iVar7 = local_c;
      if (param_1[10] == 2) {
        iVar7 = *(int *)(&DAT_005a96d0 + local_c * 4);
      }
      iVar5 = param_2[iVar7];
      iVar4 = iVar5 + 1;
      iVar5 = (local_10 / iVar5) * iVar4;
      if (iVar5 - iVar1 != 0 && iVar1 <= iVar5) break;
      local_c = local_c + 1;
      param_2[iVar7] = iVar4;
      bVar3 = true;
      local_10 = iVar5;
    } while (local_c < iVar2);
    if (!bVar3) {
      return local_10;
    }
  } while( true );
}

