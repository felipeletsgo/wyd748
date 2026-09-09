// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005860f9 | Name: FUN_005860f9


undefined4 FUN_005860f9(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  ushort *puVar3;
  int iVar4;
  char cVar5;
  int iVar6;
  undefined4 uVar7;
  int iVar8;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  int local_24;
  int local_18;
  int local_14;
  int local_10;
  int local_8;
  
  iVar4 = param_1;
  iVar2 = *(int *)(param_1 + 0x194);
  local_8 = 1 << ((byte)*(undefined4 *)(param_1 + 0x174) & 0x1f);
  if (((*(int *)(param_1 + 0xfc) == 0) || (*(int *)(iVar2 + 0x34) != 0)) ||
     (cVar5 = FUN_00585c78(), cVar5 != '\0')) {
    piVar1 = (int *)(param_1 + 0x13c);
    local_14 = param_1;
    local_30 = **(undefined4 **)(param_1 + 0x14);
    local_2c = (*(undefined4 **)(param_1 + 0x14))[1];
    local_28 = *(undefined4 *)(param_1 + 0x178);
    iVar6 = *(int *)(iVar2 + 0x10);
    iVar8 = *(int *)(iVar2 + 0x14);
    local_10 = iVar2 + 0x18;
    param_1 = 0;
    if (0 < *piVar1) {
      do {
        puVar3 = *(ushort **)(param_2 + param_1 * 4);
        if ((iVar8 < 1) &&
           (cVar5 = FUN_00585510(&local_30,iVar6,iVar8,1), iVar6 = local_24, iVar8 = local_18,
           cVar5 == '\0')) goto LAB_005861d7;
        iVar8 = iVar8 + -1;
        if ((iVar6 >> ((byte)iVar8 & 0x1f) & 1U) != 0) {
          *puVar3 = *puVar3 | (ushort)local_8;
        }
        param_1 = param_1 + 1;
      } while (param_1 < *(int *)(iVar4 + 0x13c));
    }
    **(undefined4 **)(iVar4 + 0x14) = local_30;
    *(undefined4 *)(*(int *)(iVar4 + 0x14) + 4) = local_2c;
    *(undefined4 *)(iVar4 + 0x178) = local_28;
    *(int *)(iVar2 + 0x34) = *(int *)(iVar2 + 0x34) + -1;
    *(int *)(iVar2 + 0x10) = iVar6;
    *(int *)(iVar2 + 0x14) = iVar8;
    uVar7 = 1;
  }
  else {
LAB_005861d7:
    uVar7 = 0;
  }
  return uVar7;
}

