// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042d6fa | Name: FUN_0042d6fa


void __thiscall FUN_0042d6fa(int *param_1,int param_2,int param_3,int param_4,int param_5)

{
  int iVar1;
  
  param_1[0xa973] = param_2;
  param_1[0xa974] = param_3;
  param_1[0xa975] = param_4;
  param_1[0xa976] = param_5;
  param_1[0xa977] = 0;
  param_1[0xa978] = 0x3f800000;
  iVar1 = (**(code **)(*(int *)param_1[0xa8f9] + 0xa0))(param_1[0xa8f9],param_1 + 0xa973);
  if (iVar1 < 0) {
    FUN_00431129(0x10000006);
  }
  (**(code **)(*param_1 + 0x48))();
  return;
}

