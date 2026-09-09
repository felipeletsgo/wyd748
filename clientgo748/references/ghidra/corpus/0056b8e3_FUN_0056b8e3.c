// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056b8e3 | Name: FUN_0056b8e3


void __fastcall FUN_0056b8e3(undefined4 param_1)

{
  undefined2 in_FPUControlWord;
  undefined4 local_8;
  
  local_8 = CONCAT22((short)((uint)param_1 >> 0x10),in_FPUControlWord);
  DAT_013b7390 = local_8;
  return;
}

