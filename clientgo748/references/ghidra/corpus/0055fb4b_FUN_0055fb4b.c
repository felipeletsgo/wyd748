// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055fb4b | Name: FUN_0055fb4b


int FUN_0055fb4b(int *param_1,int param_2,int *param_3)

{
  int *piVar1;
  uint uVar2;
  uint uVar3;
  int *piVar4;
  
  uVar3 = 0xffffffff;
  piVar4 = &DAT_005a6138;
  do {
    if (*param_1 == 0) {
      return *piVar4;
    }
    piVar1 = (int *)FUN_0055fab2(*param_1);
    if ((*piVar1 != 0) && ((piVar1[1] != 1 || (param_2 != 0)))) {
      if (*param_3 == *piVar1) {
        return *param_3;
      }
      uVar2 = FUN_0055fad6();
      if (((uVar2 != 0xffffffff) && (uVar2 <= uVar3)) &&
         ((uVar2 != uVar3 || ((uint)piVar1[2] < (uint)piVar4[2])))) {
        uVar3 = uVar2;
        piVar4 = piVar1;
      }
    }
    param_1 = param_1 + 1;
  } while( true );
}

