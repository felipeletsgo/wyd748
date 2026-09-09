// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00429bcb | Name: FUN_00429bcb


void __thiscall FUN_00429bcb(int param_1,int param_2,undefined4 param_3)

{
  int iVar1;
  int *piVar2;
  int local_c;
  
  if ((((0 < param_2) && (param_2 < 0x200)) && (*(int *)(param_1 + 4) != -10000)) &&
     (*(int *)(param_1 + 8 + param_2 * 0x108) != 0)) {
    iVar1 = FUN_0042c140();
    for (local_c = 0; local_c < iVar1; local_c = local_c + 1) {
      piVar2 = (int *)FUN_0042ac98(local_c);
      (**(code **)(*piVar2 + 0x3c))(piVar2,param_3);
    }
  }
  return;
}

