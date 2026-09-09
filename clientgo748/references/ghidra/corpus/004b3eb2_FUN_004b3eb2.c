// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b3eb2 | Name: FUN_004b3eb2


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_004b3eb2(undefined4 param_1,int param_2,uint param_3)

{
  undefined4 uVar1;
  
  if (DAT_0092e5cc == 0) {
    if (param_3 < 4) {
      _DAT_0092e584 = param_3;
    }
    else {
      _DAT_0092e584 = 0;
    }
    if (param_2 == 0) {
      DAT_0092e58c = 0;
    }
    else {
      DAT_0092e58c = FUN_005917f7(param_2);
    }
    DAT_0092e588 = param_1;
    uVar1 = FUN_004b3f12();
  }
  else {
    uVar1 = 2;
  }
  return uVar1;
}

