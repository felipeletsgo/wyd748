// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00403e45 | Name: FUN_00403e45


void __thiscall
FUN_00403e45(int *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6)

{
  int iVar1;
  
  iVar1 = (**(code **)(*DAT_0092e654 + 8))();
  if ((param_1[0x7e] != 0) && ((uint)param_1[0x7e] < (uint)(iVar1 - param_1[0x7d]))) {
    (**(code **)(*param_1 + 0x88))(0,1);
  }
  FUN_00401637(param_2,param_3,param_4,param_5,param_6);
  return;
}

