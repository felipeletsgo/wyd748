// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00589036 | Name: FUN_00589036


void FUN_00589036(void)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int *unaff_ESI;
  int iVar8;
  undefined1 local_34;
  int local_28;
  int local_24;
  int *local_20;
  int *local_1c;
  int local_18;
  int local_14;
  int local_8;
  
  iVar2 = unaff_ESI[0x69];
  local_1c = (int *)(iVar2 + 0x20);
  iVar3 = FUN_00588f6b();
  iVar5 = *unaff_ESI;
  if (unaff_ESI[0x19] == 3) {
    *(int *)(iVar5 + 0x18) = iVar3;
    *(int *)(iVar5 + 0x1c) = *local_1c;
    *(undefined4 *)(iVar5 + 0x20) = *(undefined4 *)(iVar2 + 0x24);
    *(undefined4 *)(iVar5 + 0x24) = *(undefined4 *)(iVar2 + 0x28);
    *(undefined4 *)(*unaff_ESI + 0x14) = 0x5d;
  }
  else {
    *(undefined4 *)(iVar5 + 0x14) = 0x5e;
    *(int *)(*unaff_ESI + 0x18) = iVar3;
  }
  (**(code **)(*unaff_ESI + 4))();
  piVar4 = (int *)(**(code **)(unaff_ESI[1] + 8))();
  local_24 = 0;
  local_20 = piVar4;
  local_18 = iVar3;
  if (0 < unaff_ESI[0x19]) {
    do {
      local_28 = *local_1c;
      iVar5 = local_18 / local_28;
      iVar8 = 0;
      if (0 < local_28) {
        iVar1 = local_28 + -1;
        local_14 = 0;
        do {
          for (iVar7 = local_14; iVar7 < iVar3; iVar7 = iVar7 + local_18) {
            local_8 = 0;
            if (0 < iVar5) {
              do {
                iVar6 = *local_20 + local_8;
                local_34 = (undefined1)((iVar1 / 2 + iVar8) / iVar1);
                local_8 = local_8 + 1;
                *(undefined1 *)(iVar6 + iVar7) = local_34;
              } while (local_8 < iVar5);
            }
          }
          local_14 = local_14 + iVar5;
          iVar8 = iVar8 + 0xff;
          local_28 = local_28 + -1;
        } while (local_28 != 0);
      }
      local_24 = local_24 + 1;
      local_1c = local_1c + 1;
      local_20 = local_20 + 1;
      local_18 = iVar5;
    } while (local_24 < unaff_ESI[0x19]);
  }
  *(int *)(iVar2 + 0x14) = iVar3;
  *(int **)(iVar2 + 0x10) = piVar4;
  return;
}

