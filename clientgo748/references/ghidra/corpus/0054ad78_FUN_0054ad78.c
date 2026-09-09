// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054ad78 | Name: FUN_0054ad78


int __thiscall FUN_0054ad78(int param_1,int param_2)

{
  int local_8;
  
  for (local_8 = *(int *)(param_1 + 0x10); (local_8 != 0 && (*(int *)(local_8 + 0x18) != param_2));
      local_8 = *(int *)(local_8 + 0xc)) {
  }
  return local_8;
}

