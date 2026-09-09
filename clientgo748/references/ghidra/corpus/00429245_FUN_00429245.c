// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00429245 | Name: FUN_00429245


int __thiscall FUN_00429245(int param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 local_10;
  undefined4 local_c;
  
  local_c = 0;
  for (local_10 = 0; local_10 < *(int *)(param_1 + 0x10); local_10 = local_10 + 1) {
    iVar1 = FUN_00429360(param_2);
    if (iVar1 != 0) {
      local_c = iVar1;
    }
  }
  return local_c;
}

