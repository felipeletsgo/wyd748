// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00586a78 | Name: FUN_00586a78


void FUN_00586a78(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,int param_5,
                 int *param_6,int param_7)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  uint uVar4;
  
  piVar3 = param_6;
  iVar2 = param_1;
  iVar1 = *(int *)(param_1 + 0x188);
  uVar4 = param_7 - *param_6;
  if (*(uint *)(iVar1 + 0x10) < (uint)(param_7 - *param_6)) {
    uVar4 = *(uint *)(iVar1 + 0x10);
  }
  param_1 = 0;
  (**(code **)(*(int *)(iVar2 + 0x19c) + 4))
            (iVar2,param_2,param_3,param_4,*(undefined4 *)(iVar1 + 0xc),&param_1,uVar4);
  (**(code **)(*(int *)(iVar2 + 0x1a4) + 4))
            (iVar2,*(undefined4 *)(iVar1 + 0xc),param_5 + *piVar3 * 4,param_1);
  *piVar3 = *piVar3 + param_1;
  return;
}

