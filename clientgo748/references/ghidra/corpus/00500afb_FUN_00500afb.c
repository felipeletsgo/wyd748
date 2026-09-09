// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00500afb | Name: FUN_00500afb


void __thiscall FUN_00500afb(int *param_1,undefined4 param_2,int param_3,undefined4 param_4)

{
  int iVar1;
  undefined4 uVar2;
  int local_8;
  
  if (param_1[0x85] == 0) {
    (**(code **)(*param_1 + 0x44))(param_2,param_3,param_4);
    param_1[0x8a] = param_3;
    FUN_0040bd30(param_1 + 10);
    FUN_0040bd30(param_1 + 10);
    iVar1 = __ftol();
    param_1[0x11f] = iVar1;
    iVar1 = __ftol();
    param_1[0x120] = iVar1;
    uVar2 = FUN_00430f20(0,0);
    FUN_0040bd30(uVar2);
    for (local_8 = 0; local_8 < 0x30; local_8 = local_8 + 1) {
      FUN_0040bd30(param_1 + 10);
    }
    param_1[0x121] = param_1[0x11f];
    param_1[0x122] = param_1[0x120];
  }
  return;
}

