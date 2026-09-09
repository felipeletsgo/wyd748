// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059884c | Name: FUN_0059884c


float10 FUN_0059884c(undefined8 param_1,short param_2)

{
  undefined8 local_c;
  
  local_c = (double)CONCAT26((param_2 + 0x3fe) * 0x10 | param_1._6_2_ & 0x800f,(undefined6)param_1);
  return (float10)local_c;
}

