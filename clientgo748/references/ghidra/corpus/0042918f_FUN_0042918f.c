// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042918f | Name: FUN_0042918f


int __fastcall FUN_0042918f(int param_1)

{
  int iVar1;
  undefined4 local_10;
  undefined4 local_c;
  
  local_c = 0;
  for (local_10 = 1; local_10 < *(int *)(param_1 + 0x10); local_10 = local_10 + 1) {
    iVar1 = FUN_004293f0();
    if (iVar1 != 0) {
      local_c = iVar1;
    }
  }
  return local_c;
}

