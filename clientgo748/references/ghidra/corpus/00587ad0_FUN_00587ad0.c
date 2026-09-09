// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00587ad0 | Name: FUN_00587ad0


void FUN_00587ad0(int param_1,undefined4 param_2,int *param_3,undefined4 param_4,int param_5,
                 int *param_6,int param_7)

{
  undefined4 *puVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar3 = *(int *)(param_1 + 0x19c);
  if (*(char *)(iVar3 + 0x24) == '\0') {
    uVar4 = 2;
    if (*(uint *)(iVar3 + 0x2c) < 2) {
      uVar4 = *(uint *)(iVar3 + 0x2c);
    }
    uVar2 = param_7 - *param_6;
    if (uVar2 < uVar4) {
      uVar4 = uVar2;
    }
    puVar1 = (undefined4 *)(param_5 + *param_6 * 4);
    local_c = *puVar1;
    if (uVar4 < 2) {
      local_8 = *(undefined4 *)(iVar3 + 0x20);
      *(undefined1 *)(iVar3 + 0x24) = 1;
    }
    else {
      local_8 = puVar1[1];
    }
    (**(code **)(iVar3 + 0xc))(param_1,param_2,*param_3,&local_c);
  }
  else {
    uVar4 = 1;
    FUN_00577e59(iVar3 + 0x20,0,param_5 + *param_6 * 4,0,1,*(undefined4 *)(iVar3 + 0x28));
    *(undefined1 *)(iVar3 + 0x24) = 0;
  }
  *param_6 = *param_6 + uVar4;
  *(int *)(iVar3 + 0x2c) = *(int *)(iVar3 + 0x2c) - uVar4;
  if (*(char *)(iVar3 + 0x24) == '\0') {
    *param_3 = *param_3 + 1;
  }
  return;
}

