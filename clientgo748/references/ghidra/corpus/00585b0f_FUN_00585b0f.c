// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00585b0f | Name: FUN_00585b0f


void FUN_00585b0f(int *param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  int local_c;
  undefined4 *local_8;
  
  piVar4 = param_1;
  iVar1 = param_1[0x65];
  if ((((param_1[0x5a] != 0) || (param_1[0x5b] != 0x3f)) || (param_1[0x5c] != 0)) ||
     (param_1[0x5d] != 0)) {
    *(undefined4 *)(*param_1 + 0x14) = 0x76;
    (**(code **)(*param_1 + 4))(param_1,0xffffffff);
  }
  local_c = 0;
  if (0 < param_1[0x48]) {
    local_8 = (undefined4 *)(iVar1 + 0x20);
    param_1 = param_1 + 0x49;
    do {
      iVar2 = *(int *)(*param_1 + 0x14);
      iVar3 = *(int *)(*param_1 + 0x18);
      if (((iVar2 < 0) || (3 < iVar2)) || (piVar4[iVar2 + 0x28] == 0)) {
        *(undefined4 *)(*piVar4 + 0x14) = 0x31;
        *(int *)(*piVar4 + 0x18) = iVar2;
        (**(code **)*piVar4)(piVar4);
      }
      if (((iVar3 < 0) || (3 < iVar3)) || (piVar4[iVar3 + 0x2c] == 0)) {
        *(undefined4 *)(*piVar4 + 0x14) = 0x31;
        *(int *)(*piVar4 + 0x18) = iVar3;
        (**(code **)*piVar4)(piVar4);
      }
      FUN_00585366(piVar4,piVar4[iVar2 + 0x28],iVar1 + 0x34 + iVar2 * 4);
      FUN_00585366(piVar4,piVar4[iVar3 + 0x2c],iVar1 + 0x44 + iVar3 * 4);
      *local_8 = 0;
      param_1 = param_1 + 1;
      local_8 = local_8 + 1;
      local_c = local_c + 1;
    } while (local_c < piVar4[0x48]);
  }
  *(undefined1 *)(iVar1 + 0x18) = 0;
  *(undefined4 *)(iVar1 + 0x14) = 0;
  *(undefined4 *)(iVar1 + 8) = 0;
  *(undefined4 *)(iVar1 + 0xc) = 0;
  *(undefined4 *)(iVar1 + 0x10) = 0;
  *(int *)(iVar1 + 0x30) = piVar4[0x3f];
  return;
}

