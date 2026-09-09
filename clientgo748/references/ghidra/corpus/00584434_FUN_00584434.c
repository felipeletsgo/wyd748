// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00584434 | Name: FUN_00584434


void FUN_00584434(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  uint *puVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  
  iVar2 = *(int *)(param_1 + 0x180);
  if (*(char *)(iVar2 + 0x30) == '\0') {
    iVar4 = (**(code **)(*(int *)(param_1 + 0x184) + 0xc))(param_1,iVar2 + 8);
    if (iVar4 == 0) {
      return;
    }
    *(undefined1 *)(iVar2 + 0x30) = 1;
  }
  uVar3 = *(uint *)(param_1 + 0x114);
  puVar1 = (uint *)(iVar2 + 0x34);
  (**(code **)(*(int *)(param_1 + 0x188) + 4))
            (param_1,iVar2 + 8,puVar1,uVar3,param_2,param_3,param_4);
  if (uVar3 <= *puVar1) {
    *(undefined1 *)(iVar2 + 0x30) = 0;
    *puVar1 = 0;
  }
  return;
}

