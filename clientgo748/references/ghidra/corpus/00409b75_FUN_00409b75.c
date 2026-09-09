// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00409b75 | Name: FUN_00409b75


undefined4 __thiscall
FUN_00409b75(int *param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  int iVar1;
  undefined4 uVar2;
  
  if (param_1[0xf] == 0) {
    return 0;
  }
  iVar1 = FUN_00401000(param_4,param_5,param_1[0x13],param_1[0x14],param_1[0x15],param_1[0x16]);
  param_1[0x7a] = iVar1;
  if (param_2 != 0x200) {
    if (param_2 != 0x201) {
      uVar2 = FUN_00401726(param_2,param_3,param_4,param_5);
      return uVar2;
    }
    if (param_1[0x7a] == 1) {
      param_1[0x79] = (uint)(param_1[0x79] == 0);
      param_1[0xc] = 1;
      (**(code **)(*param_1 + 0x54))();
      if (param_1[0x17] != 0) {
        (*(code *)**(undefined4 **)param_1[0x17])(param_1[0x11],0);
      }
      return 1;
    }
  }
  return 0;
}

