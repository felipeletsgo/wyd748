// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055fccc | Name: FUN_0055fccc


int FUN_0055fccc(int param_1,undefined4 param_2,undefined4 param_3,int param_4,undefined4 param_5,
                undefined4 param_6,undefined4 param_7,undefined4 *param_8,int param_9,
                undefined4 param_10)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  undefined1 local_b8 [76];
  int local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c [4];
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44 [7];
  undefined4 local_28;
  undefined4 local_24;
  
  FUN_0056705a();
  FUN_0056a92f();
  if (param_1 == 0) {
    iVar2 = -0x7789f794;
  }
  else if ((param_4 == 0) || (param_8 == (undefined4 *)0x0)) {
    iVar2 = -0x7789f794;
  }
  else {
    if (param_9 == -1) {
      param_9 = 0x80004;
    }
    iVar1 = FUN_0056a9c9(local_b8,param_1,param_2,param_3,0,0);
    if (-1 < iVar1) {
      local_68 = param_5;
      local_6c = param_4;
      local_64 = param_6;
      local_60 = 0;
      local_5c[0] = *param_8;
      local_5c[1] = param_8[1];
      local_5c[2] = param_8[2];
      local_5c[3] = param_8[3];
      local_4c = 0;
      local_48 = 1;
      puVar3 = local_5c;
      puVar4 = local_44;
      for (iVar2 = 6; iVar2 != 0; iVar2 = iVar2 + -1) {
        *puVar4 = *puVar3;
        puVar3 = puVar3 + 1;
        puVar4 = puVar4 + 1;
      }
      local_28 = param_10;
      local_24 = param_7;
      iVar1 = FUN_0056a815(local_b8,&local_6c,param_9);
      iVar2 = 0;
      if (-1 < iVar1) goto LAB_0055fd91;
    }
    iVar2 = iVar1;
  }
LAB_0055fd91:
  FUN_0056a9c4();
  FUN_00567064();
  return iVar2;
}

