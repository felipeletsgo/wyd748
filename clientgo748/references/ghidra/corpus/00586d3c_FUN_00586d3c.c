// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00586d3c | Name: FUN_00586d3c


void FUN_00586d3c(int param_1,int param_2,int *param_3,undefined4 param_4,int param_5,int *param_6,
                 int param_7)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  undefined4 local_8;
  
  iVar3 = param_1;
  iVar2 = *(int *)(param_1 + 0x19c);
  if (*(int *)(param_1 + 0x110) <= *(int *)(iVar2 + 0x5c)) {
    param_1 = 0;
    local_8 = *(int *)(iVar3 + 0xc4);
    if (0 < *(int *)(iVar3 + 0x20)) {
      iVar4 = iVar2 + 0xc;
      do {
        (**(code **)(iVar4 + 0x28))
                  (iVar3,local_8,
                   *(int *)(param_2 + param_1 * 4) + *(int *)(iVar4 + 0x58) * *param_3 * 4,iVar4);
        local_8 = local_8 + 0x54;
        param_1 = param_1 + 1;
        iVar4 = iVar4 + 4;
      } while (param_1 < *(int *)(iVar3 + 0x20));
    }
    *(undefined4 *)(iVar2 + 0x5c) = 0;
  }
  uVar5 = *(int *)(iVar3 + 0x110) - *(int *)(iVar2 + 0x5c);
  if (*(uint *)(iVar2 + 0x60) < uVar5) {
    uVar5 = *(uint *)(iVar2 + 0x60);
  }
  uVar1 = param_7 - *param_6;
  if (uVar1 < uVar5) {
    uVar5 = uVar1;
  }
  (**(code **)(*(int *)(iVar3 + 0x1a0) + 4))
            (iVar3,iVar2 + 0xc,*(undefined4 *)(iVar2 + 0x5c),param_5 + *param_6 * 4,uVar5);
  *param_6 = *param_6 + uVar5;
  *(int *)(iVar2 + 0x60) = *(int *)(iVar2 + 0x60) - uVar5;
  *(int *)(iVar2 + 0x5c) = *(int *)(iVar2 + 0x5c) + uVar5;
  if (*(int *)(iVar3 + 0x110) <= *(int *)(iVar2 + 0x5c)) {
    *param_3 = *param_3 + 1;
  }
  return;
}

