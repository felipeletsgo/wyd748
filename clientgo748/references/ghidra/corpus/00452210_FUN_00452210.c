// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00452210 | Name: FUN_00452210


undefined4 __thiscall FUN_00452210(int param_1,char param_2)

{
  undefined4 uVar1;
  undefined4 local_8;
  
  if ((param_2 == '-') || (param_2 == '_')) {
    if (*(char *)(param_1 + 0x26eb4) == '\x01') {
      local_8 = *(int *)(param_1 + 0x27ae4) + 1;
      if (10 < local_8) {
        local_8 = 1;
      }
      FUN_00447469(local_8);
    }
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

