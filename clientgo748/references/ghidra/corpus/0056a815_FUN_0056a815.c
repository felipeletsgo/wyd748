// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056a815 | Name: FUN_0056a815


int __thiscall FUN_0056a815(int *param_1,int param_2,undefined4 param_3,uint param_4)

{
  int iVar1;
  
  param_1[1] = 0;
  *param_1 = 0;
  param_1[2] = param_4;
  if ((((param_4 & 0xffff) == 0) || (5 < (param_4 & 0xffff))) || ((param_4 & 0xfff00000) != 0)) {
    return -0x7789f794;
  }
  *(uint *)(param_2 + 0x40) = param_4 & 0x80000;
  iVar1 = FUN_0056fd19(param_2);
  param_1[1] = iVar1;
  if (iVar1 != 0) {
    iVar1 = FUN_0056fd19(param_3);
    *param_1 = iVar1;
    if (iVar1 != 0) {
      iVar1 = FUN_0056bb6e(iVar1);
      if (iVar1 < 0) goto LAB_0056a901;
      iVar1 = FUN_00568978();
      if ((((-1 < iVar1) || (iVar1 = FUN_00568ab0(), -1 < iVar1)) ||
          ((iVar1 = FUN_00568ba1(), -1 < iVar1 ||
           ((iVar1 = FUN_00568da3(), -1 < iVar1 || (iVar1 = FUN_00568fa7(), -1 < iVar1)))))) ||
         ((iVar1 = FUN_005693ca(), -1 < iVar1 ||
          ((((iVar1 = FUN_0056995c(), -1 < iVar1 || (iVar1 = FUN_00569d42(), -1 < iVar1)) ||
            (iVar1 = FUN_0056a473(), -1 < iVar1)) || (iVar1 = FUN_005673a1(), -1 < iVar1)))))) {
        iVar1 = 0;
        goto LAB_0056a901;
      }
    }
  }
  iVar1 = -0x7fffbffb;
LAB_0056a901:
  if ((undefined4 *)param_1[1] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[1])(1);
    param_1[1] = 0;
  }
  if ((undefined4 *)*param_1 != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)*param_1)(1);
    *param_1 = 0;
  }
  return iVar1;
}

