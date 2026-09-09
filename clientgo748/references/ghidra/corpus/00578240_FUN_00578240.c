// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00578240 | Name: FUN_00578240


void FUN_00578240(void)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  int iVar4;
  int *unaff_EBX;
  undefined4 *puVar5;
  undefined4 *puVar6;
  int local_c;
  int *local_8;
  
  local_c = 0;
  if (0 < unaff_EBX[0x48]) {
    local_8 = unaff_EBX + 0x49;
    do {
      iVar1 = *local_8;
      if (*(int *)(iVar1 + 0x4c) == 0) {
        iVar2 = *(int *)(iVar1 + 0x10);
        if (((iVar2 < 0) || (3 < iVar2)) || (unaff_EBX[iVar2 + 0x24] == 0)) {
          *(undefined4 *)(*unaff_EBX + 0x14) = 0x33;
          *(int *)(*unaff_EBX + 0x18) = iVar2;
          (**(code **)*unaff_EBX)();
        }
        puVar3 = (undefined4 *)(**(code **)unaff_EBX[1])();
        puVar5 = (undefined4 *)unaff_EBX[iVar2 + 0x24];
        puVar6 = puVar3;
        for (iVar4 = 0x20; iVar4 != 0; iVar4 = iVar4 + -1) {
          *puVar6 = *puVar5;
          puVar5 = puVar5 + 1;
          puVar6 = puVar6 + 1;
        }
        *(undefined2 *)puVar6 = *(undefined2 *)puVar5;
        *(undefined4 **)(iVar1 + 0x4c) = puVar3;
      }
      local_c = local_c + 1;
      local_8 = local_8 + 1;
    } while (local_c < unaff_EBX[0x48]);
  }
  return;
}

