// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00477513 | Name: FUN_00477513


int __thiscall FUN_00477513(int param_1,undefined4 param_2,int param_3,int param_4,int param_5)

{
  int iVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  int local_20;
  int local_1c;
  
  if (param_4 < 1000000) {
    if (param_3 < 0x15181) {
      if (param_3 < 0xe11) {
        if (param_3 < 0x259) {
          iVar2 = FUN_0058f078(param_2,&PTR_DAT_005b86b0,param_3);
        }
        else {
          iVar2 = FUN_0058f078(param_2,&DAT_00a3d3f8,param_3 / 0x3c);
        }
      }
      else {
        iVar2 = FUN_0058f078(param_2,&DAT_00a3d378,param_3 / 0xe10);
      }
    }
    else {
      iVar2 = FUN_0058f078(param_2,&DAT_00a3d2f8,param_3 / 0x15180);
    }
  }
  else {
    iVar3 = *(int *)(*(int *)(param_1 + 0x4c) + 0x6a8 + param_5 * 8) / 1000000;
    uVar4 = (*(int *)(*(int *)(param_1 + 0x4c) + 0x6a8 + param_5 * 8) % 1000000) / 10000;
    iVar1 = *(int *)(*(int *)(param_1 + 0x4c) + 0x6a8 + param_5 * 8);
    iVar2 = iVar1 / 10000;
    local_20 = 0x16d;
    uVar5 = uVar4 & 0x80000003;
    if ((int)uVar5 < 0) {
      uVar5 = (uVar5 - 1 | 0xfffffffc) + 1;
    }
    if (uVar5 == 0) {
      local_20 = 0x16e;
    }
    local_1c = 0;
    if (iVar3 == 4) {
      local_1c = 7;
    }
    else if (iVar3 == 5) {
      local_1c = 0xf;
    }
    else if (iVar3 == 6) {
      local_1c = 0x1e;
    }
    if (iVar3 != 0) {
      iVar2 = FUN_0058f078(param_2,&DAT_00a3d2f8,
                           (local_1c - (*(int *)(param_1 + 0x276a4) - uVar4) * local_20) -
                           (*(int *)(param_1 + 0x276a8) - iVar1 % 10000));
    }
  }
  return iVar2;
}

