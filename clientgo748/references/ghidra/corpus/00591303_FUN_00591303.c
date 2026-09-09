// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00591303 | Name: FUN_00591303


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 FUN_00591303(undefined4 param_1,undefined4 param_2)

{
  undefined4 uVar1;
  int iVar2;
  float10 fVar3;
  
  uVar1 = FUN_005989ad(0x133f,0xffff);
  if ((param_2._2_2_ & 0x7ff0) == 0x7ff0) {
    iVar2 = FUN_00598875(param_1,(int)(CONCAT26(param_2._2_2_,CONCAT24((undefined2)param_2,param_1))
                                      >> 0x20));
    if (iVar2 == 1) {
      FUN_005989ad(uVar1,0xffff);
      fVar3 = (float10)(double)CONCAT26(param_2._2_2_,CONCAT24((undefined2)param_2,param_1));
    }
    else if (iVar2 == 2) {
      FUN_005989ad(uVar1,0xffff);
      fVar3 = -(float10)(double)CONCAT26(param_2._2_2_,CONCAT24((undefined2)param_2,param_1));
    }
    else if (iVar2 == 3) {
      fVar3 = (float10)FUN_00598194();
    }
    else {
      fVar3 = (float10)FUN_005981e8(8,0x15,CONCAT26(param_2._2_2_,
                                                    CONCAT24((undefined2)param_2,param_1)),
                                    (double)CONCAT26(param_2._2_2_,
                                                     CONCAT24((undefined2)param_2,param_1)) +
                                    _DAT_005a4c28,uVar1);
    }
  }
  else {
    FUN_005989ad(uVar1,0xffff);
    fVar3 = (float10)ABS((double)CONCAT44(param_2,param_1));
  }
  return fVar3;
}

