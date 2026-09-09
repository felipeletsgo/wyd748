// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b6400 | Name: FUN_004b6400


undefined4 __fastcall FUN_004b6400(int param_1)

{
  undefined4 local_c;
  
  if (*(int *)(param_1 + 4) == 0) {
    local_c = FUN_004b6970();
  }
  else {
    local_c = *(undefined4 *)(param_1 + 4);
  }
  return local_c;
}

