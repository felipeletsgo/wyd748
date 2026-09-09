// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00428fd7 | Name: FUN_00428fd7


int __fastcall FUN_00428fd7(int param_1)

{
  int iVar1;
  undefined4 local_10;
  undefined4 local_c;
  
  local_c = 0;
  for (local_10 = 0; local_10 < *(int *)(param_1 + 0x10); local_10 = local_10 + 1) {
    iVar1 = FUN_00429390();
    if (iVar1 != 0) {
      local_c = iVar1;
    }
  }
  return local_c;
}

