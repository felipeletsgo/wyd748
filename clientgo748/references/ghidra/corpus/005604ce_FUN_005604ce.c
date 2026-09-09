// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005604ce | Name: FUN_005604ce


int FUN_005604ce(int param_1,undefined4 param_2,undefined4 param_3,int param_4,undefined4 param_5,
                undefined4 param_6,undefined4 param_7,undefined4 param_8)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined1 local_2c [40];
  
  FUN_0056a9a8();
  if ((param_1 == 0) || (param_4 == 0)) {
    iVar3 = -0x7789f794;
  }
  else {
    uVar1 = 1;
    if (param_1 == param_4) {
      uVar1 = 0x10001;
    }
    iVar2 = FUN_0056ade1(&local_54,param_4,param_5,param_6,0,uVar1);
    if (-1 < iVar2) {
      iVar2 = FUN_0055fe7b(param_1,param_2,param_3,local_54,local_50,local_4c,local_48,param_5,
                           local_2c,param_7,param_8);
      iVar3 = 0;
      if (-1 < iVar2) goto LAB_00560545;
    }
    iVar3 = iVar2;
  }
LAB_00560545:
  thunk_FUN_0056a9ae();
  return iVar3;
}

