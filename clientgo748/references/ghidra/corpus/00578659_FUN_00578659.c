// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00578659 | Name: FUN_00578659


int FUN_00578659(int *param_1,undefined4 param_2,int param_3,uint param_4)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  
  uVar1 = (uint)(0x3b9ac9f0 / (ulonglong)(uint)(param_3 * 0x80));
  iVar2 = param_1[1];
  if (uVar1 == 0) {
    *(undefined4 *)(*param_1 + 0x14) = 0x45;
    (**(code **)*param_1)(param_1);
  }
  if ((int)param_4 <= (int)uVar1) {
    uVar1 = param_4;
  }
  *(uint *)(iVar2 + 0x4c) = uVar1;
  iVar2 = FUN_0057843d(param_1,param_2,param_4 << 2);
  uVar5 = 0;
  if (param_4 != 0) {
    do {
      if (param_4 - uVar5 <= uVar1) {
        uVar1 = param_4 - uVar5;
      }
      iVar3 = FUN_00578539(param_1,param_2,uVar1 * param_3 * 0x80);
      for (uVar4 = uVar1; uVar4 != 0; uVar4 = uVar4 - 1) {
        *(int *)(iVar2 + uVar5 * 4) = iVar3;
        iVar3 = iVar3 + param_3 * 0x80;
        uVar5 = uVar5 + 1;
      }
    } while (uVar5 < param_4);
  }
  return iVar2;
}

