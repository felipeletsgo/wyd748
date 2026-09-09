// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00404dd8 | Name: FUN_00404dd8


undefined4 __thiscall
FUN_00404dd8(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  undefined4 uVar1;
  int *piVar2;
  
  if (*(int *)(param_1 + 0x3c) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_00401000(param_4,param_5,*(undefined4 *)(param_1 + 0x4c),
                         *(undefined4 *)(param_1 + 0x50),*(undefined4 *)(param_1 + 0x54),
                         *(undefined4 *)(param_1 + 0x58));
    *(undefined4 *)(param_1 + 0x34) = uVar1;
    if ((param_2 == 0x201) && (*(int *)(param_1 + 0x34) == 1)) {
      if (DAT_0067cf38 != (int *)0x0) {
        piVar2 = (int *)(**(code **)(*DAT_0067cf38 + 0x38))(param_1);
        (**(code **)(*piVar2 + 0x40))();
        if (DAT_005b892c == 2) {
          uVar1 = FUN_00401726(0x201,param_3,param_4,param_5);
          return uVar1;
        }
      }
      uVar1 = 1;
    }
    else {
      uVar1 = FUN_00401726(param_2,param_3,param_4,param_5);
    }
  }
  return uVar1;
}

