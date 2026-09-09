// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b6560 | Name: FUN_004b6560


int __fastcall FUN_004b6560(int param_1)

{
  undefined4 local_c;
  
  if (*(int *)(param_1 + 4) == 0) {
    local_c = 0;
  }
  else {
    local_c = (*(int *)(param_1 + 8) - *(int *)(param_1 + 4)) / 0x28;
  }
  return local_c;
}

