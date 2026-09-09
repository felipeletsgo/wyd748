// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055fe7b | Name: FUN_0055fe7b


int FUN_0055fe7b(int param_1,undefined4 param_2,undefined4 param_3,int param_4,undefined4 param_5,
                undefined4 param_6,undefined4 param_7,undefined4 param_8,undefined4 *param_9,
                int param_10,undefined4 param_11)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  undefined1 local_ac [76];
  int local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50 [6];
  undefined4 local_38 [7];
  undefined4 local_1c;
  undefined4 local_18;
  
  FUN_0056705a();
  FUN_0056a9a8();
  if (((param_1 == 0) || (param_4 == 0)) || (param_9 == (undefined4 *)0x0)) {
    iVar1 = -0x7789f794;
  }
  else {
    if (param_10 == -1) {
      param_10 = 0x80004;
    }
    iVar1 = FUN_0056ade1(local_ac,param_1,param_2,param_3,0,0);
    if (-1 < iVar1) {
      local_5c = param_5;
      local_58 = param_6;
      local_60 = param_4;
      local_54 = param_7;
      puVar2 = param_9;
      puVar3 = local_50;
      for (iVar1 = 6; iVar1 != 0; iVar1 = iVar1 + -1) {
        *puVar3 = *puVar2;
        puVar2 = puVar2 + 1;
        puVar3 = puVar3 + 1;
      }
      puVar2 = local_38;
      for (iVar1 = 6; iVar1 != 0; iVar1 = iVar1 + -1) {
        *puVar2 = *param_9;
        param_9 = param_9 + 1;
        puVar2 = puVar2 + 1;
      }
      local_1c = param_11;
      local_18 = param_8;
      iVar1 = FUN_0056a815(local_ac,&local_60,param_10);
      if (-1 < iVar1) {
        iVar1 = 0;
      }
    }
  }
  thunk_FUN_0056a9ae();
  FUN_00567064();
  return iVar1;
}

