// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00488b52 | Name: FUN_00488b52


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_00488b52(int param_1,int param_2)

{
  int iVar1;
  int *piVar2;
  undefined4 uVar3;
  int iVar4;
  int local_20;
  int local_14;
  int local_c;
  
  if (*(int *)(param_1 + 0x2872c) == 0) {
    uVar3 = 0;
  }
  else {
    iVar1 = *(int *)(param_1 + 0x2872c);
    if (*(int *)(param_2 + 0xc) == 0) {
      *(undefined1 *)(*(int *)(param_1 + 0x4c) + 0x248) = 0;
      for (local_14 = 0; local_14 < *(int *)(iVar1 + 0x210); local_14 = local_14 + 1) {
        iVar4 = (**(code **)(*DAT_013b71e8 + 0x34))
                          (*(undefined4 *)(*(int *)(iVar1 + 0x214 + local_14 * 4) + 0xe50));
        if ((iVar4 != 0) && (*(undefined1 *)(iVar4 + 0x248) = 0, *(int *)(iVar4 + 0x430) != 0)) {
          if (*(undefined4 **)(iVar4 + 0x430) != (undefined4 *)0x0) {
            (**(code **)**(undefined4 **)(iVar4 + 0x430))(1);
          }
          *(undefined4 *)(iVar4 + 0x430) = 0;
        }
      }
      FUN_004091ab();
    }
    else {
      for (local_20 = 0; local_20 < *(int *)(iVar1 + 0x210); local_20 = local_20 + 1) {
        iVar4 = *(int *)(iVar1 + 0x214 + local_20 * 4);
        if (*(int *)(iVar4 + 0xe50) == *(int *)(param_2 + 0xc)) {
          iVar4 = (**(code **)(*DAT_013b71e8 + 0x34))(*(undefined4 *)(iVar4 + 0xe50));
          if ((iVar4 != 0) && (*(undefined1 *)(iVar4 + 0x248) = 0, *(int *)(iVar4 + 0x430) != 0)) {
            if (*(undefined4 **)(iVar4 + 0x430) != (undefined4 *)0x0) {
              (**(code **)**(undefined4 **)(iVar4 + 0x430))(1);
            }
            *(undefined4 *)(iVar4 + 0x430) = 0;
          }
          FUN_00408f1f(local_20);
          break;
        }
      }
    }
    local_c = *(int *)(iVar1 + 0x210);
    if (local_c == 1) {
      FUN_004091ab();
      *(undefined1 *)(*(int *)(param_1 + 0x4c) + 0x248) = 0;
      local_c = 0;
    }
    if (DAT_005b892c != 2) {
      piVar2 = *(int **)(param_1 + 0x27d8c);
      (**(code **)(*piVar2 + 0x74))(0x42e40000,(float)local_c * _DAT_005a4374 + _DAT_005a4370);
      (**(code **)(*piVar2 + 0x70))(0,_DAT_005a436c - (float)local_c * _DAT_005a4374);
      FUN_0040981b(0x42e40000,(float)local_c * _DAT_005a4374);
    }
    uVar3 = 1;
  }
  return uVar3;
}

