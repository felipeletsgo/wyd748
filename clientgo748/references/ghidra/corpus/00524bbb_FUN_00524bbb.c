// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00524bbb | Name: FUN_00524bbb


void __thiscall FUN_00524bbb(int *param_1,uint param_2,uint param_3,int param_4)

{
  int iVar1;
  undefined1 local_3c [4];
  undefined2 local_38;
  undefined2 local_36;
  short local_30;
  short local_2e;
  undefined4 local_2c;
  undefined4 local_28;
  short local_24;
  short sStack_22;
  undefined4 local_8;
  
  if ((param_1[0x85] == 0) &&
     ((local_8 = (**(code **)(*DAT_0092e654 + 8))(), param_1[0x11f] != param_2 ||
      (param_1[0x120] != param_3)))) {
    _memset(local_3c,0,0x34);
    local_36 = (undefined2)param_1[8];
    local_30 = (short)param_2;
    local_2e = (short)param_3;
    local_28 = 0;
    local_38 = 0x366;
    if (param_1 == *(int **)(DAT_0067cf38 + 0x4c)) {
      local_2c = DAT_005bb418;
    }
    else {
      local_2c = __ftol();
    }
    iVar1 = DAT_0067cf38;
    local_24 = (short)param_2;
    sStack_22 = (short)param_3;
    if ((param_4 == 0) || (param_4 == 2)) {
      *(int *)(DAT_0067cf38 + 0x2878c) = (int)local_30;
      *(int *)(iVar1 + 0x28790) = (int)local_2e;
      *(uint *)(iVar1 + 0x28794) = param_2 & 0xffff;
      *(uint *)(iVar1 + 0x28798) = param_3 & 0xffff;
      FUN_0055f2dd(local_3c,0x34);
    }
    if ((param_4 == 0) || (param_4 == 1)) {
      (**(code **)(*param_1 + 4))(0x366,local_3c);
    }
  }
  return;
}

