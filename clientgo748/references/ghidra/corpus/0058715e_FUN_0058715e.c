// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058715e | Name: FUN_0058715e


void FUN_0058715e(int *param_1)

{
  int iVar1;
  int iVar2;
  bool bVar3;
  undefined4 *puVar4;
  int iVar5;
  int iVar6;
  undefined4 uVar7;
  undefined4 *puVar8;
  int *piVar9;
  undefined1 local_10;
  int local_c;
  
  puVar4 = (undefined4 *)(**(code **)param_1[1])(param_1,1,0xa0);
  param_1[0x67] = (int)puVar4;
  *(undefined1 *)(puVar4 + 2) = 0;
  *puVar4 = &LAB_00586d22;
  puVar4[1] = FUN_00586d3c;
  if ((char)param_1[0x42] != '\0') {
    *(undefined4 *)(*param_1 + 0x14) = 0x17;
    (**(code **)*param_1)(param_1);
  }
  if (((char)param_1[0x12] == '\0') || (param_1[0x45] < 2)) {
    bVar3 = false;
  }
  else {
    bVar3 = true;
  }
  local_c = 0;
  if (0 < param_1[8]) {
    puVar8 = puVar4 + 0xd;
    piVar9 = (int *)(param_1[0x31] + 0x24);
    do {
      iVar5 = (piVar9[-7] * *piVar9) / param_1[0x45];
      iVar6 = (piVar9[-6] * *piVar9) / param_1[0x45];
      iVar1 = param_1[0x44];
      iVar2 = param_1[0x43];
      puVar8[0xc] = iVar6;
      if ((char)piVar9[3] == '\0') {
        *puVar8 = &LAB_00586e0d;
      }
      else if ((iVar5 == iVar2) && (iVar6 == iVar1)) {
        *puVar8 = &LAB_00586e02;
      }
      else {
        if (iVar5 * 2 == iVar2) {
          if (iVar6 == iVar1) {
            if ((bVar3) && (2 < (uint)piVar9[1])) {
              *puVar8 = FUN_00586fb3;
            }
            else {
              *puVar8 = FUN_00586ef2;
            }
          }
          else {
            if ((iVar5 * 2 != iVar2) || (iVar6 * 2 != iVar1)) goto LAB_00587293;
            if ((bVar3) && (2 < (uint)piVar9[1])) {
              *puVar8 = FUN_00587057;
              *(undefined1 *)(puVar4 + 2) = 1;
            }
            else {
              *puVar8 = FUN_00586f49;
            }
          }
        }
        else {
LAB_00587293:
          if ((iVar2 % iVar5 == 0) && (iVar1 % iVar6 == 0)) {
            local_10 = (undefined1)(iVar2 / iVar5);
            *puVar8 = FUN_00586e15;
            *(undefined1 *)(local_c + 0x8c + (int)puVar4) = local_10;
            *(char *)(local_c + 0x96 + (int)puVar4) = (char)(iVar1 / iVar6);
          }
          else {
            *(undefined4 *)(*param_1 + 0x14) = 0x25;
            (**(code **)*param_1)(param_1);
          }
        }
        iVar1 = param_1[1];
        uVar7 = FUN_00577e41(param_1[0x17],param_1[0x43],param_1[0x44]);
        uVar7 = (**(code **)(iVar1 + 8))(param_1,1,uVar7);
        puVar8[-10] = uVar7;
      }
      local_c = local_c + 1;
      puVar8 = puVar8 + 1;
      piVar9 = piVar9 + 0x15;
    } while (local_c < param_1[8]);
  }
  return;
}

