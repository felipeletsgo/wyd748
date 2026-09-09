// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00560f07 | Name: FUN_00560f07


int FUN_00560f07(int param_1,int *param_2)

{
  int iVar1;
  void *pvVar2;
  int iVar3;
  
  if (param_1 == 0) {
    iVar1 = -0x7789f794;
  }
  else if (param_2 == (int *)0x0) {
    iVar1 = -0x7789f794;
  }
  else {
    pvVar2 = operator_new(0x20);
    if (pvVar2 == (void *)0x0) {
      iVar3 = 0;
    }
    else {
      iVar3 = FUN_00571073();
    }
    if (iVar3 == 0) {
      iVar1 = -0x7ff8fff2;
    }
    else {
      iVar1 = FUN_00570627(param_1);
      if (iVar1 < 0) {
        FUN_00560eeb(1);
      }
      else {
        *param_2 = iVar3;
        iVar1 = 0;
      }
    }
  }
  return iVar1;
}

