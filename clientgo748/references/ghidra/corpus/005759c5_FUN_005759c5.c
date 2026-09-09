// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005759c5 | Name: FUN_005759c5


void __fastcall FUN_005759c5(undefined4 param_1)

{
  undefined2 in_FPUControlWord;
  undefined4 local_8;
  
  local_8 = CONCAT22((short)((uint)param_1 >> 0x10),in_FPUControlWord);
  DAT_013b7f74 = local_8;
  return;
}

