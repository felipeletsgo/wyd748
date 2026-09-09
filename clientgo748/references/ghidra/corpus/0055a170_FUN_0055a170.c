// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055a170 | Name: FUN_0055a170


uint FUN_0055a170(int param_1,int param_2,int param_3,int param_4,undefined4 param_5,char param_6)

{
  uint in_EAX;
  int iVar1;
  int iVar2;
  int local_10;
  int local_8;
  
  if (((param_1 != 0) && (param_2 != 0)) && (param_4 != 0)) {
    in_EAX = 0;
    if ((param_3 + 7U & 0xfffffff8) != 0) {
      FUN_0055a2a3(param_4,param_5);
      iVar2 = (int)(param_3 + 7U) >> 3;
      if (DAT_00e37f20 == '\0') {
        iVar1 = 0;
        for (local_8 = 0; local_8 < iVar2; local_8 = local_8 + 1) {
          FUN_0055a324(param_1,param_2,&DAT_00e37f24,CONCAT31((int3)((uint)iVar1 >> 8),param_6));
          iVar1 = param_1 + 8;
          param_2 = param_2 + 8;
          param_1 = iVar1;
        }
      }
      else {
        iVar1 = iVar2;
        for (local_10 = 0; local_10 < iVar2; local_10 = local_10 + 1) {
          FUN_0055a324(param_1,param_2,&DAT_00e37f24,param_6);
          FUN_0055a324(param_1,param_1,&DAT_00e38224,param_6 == '\0');
          FUN_0055a324(param_1,param_1,&DAT_00e37f24,param_6);
          param_1 = param_1 + 8;
          iVar1 = param_2 + 8;
          param_2 = iVar1;
        }
      }
      return CONCAT31((int3)((uint)iVar1 >> 8),1);
    }
  }
  return in_EAX & 0xffffff00;
}

