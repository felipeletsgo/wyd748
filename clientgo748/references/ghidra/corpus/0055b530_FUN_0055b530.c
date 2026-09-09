// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055b530 | Name: FUN_0055b530


undefined4 FUN_0055b530(char *param_1,int param_2)

{
  size_t sVar1;
  undefined4 local_c;
  undefined4 local_8;
  
  sVar1 = _strlen(param_1);
  local_8 = 0;
  FUN_0058ee20(param_2,param_1);
  for (local_c = 0; local_c < (int)sVar1; local_c = local_c + 1) {
    if (*(char *)(param_2 + local_c) == '_') {
      *(undefined1 *)(param_2 + local_c) = 0x2f;
      local_8 = 1;
    }
  }
  return local_8;
}

