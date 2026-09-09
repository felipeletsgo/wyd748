// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00555790 | Name: FUN_00555790


int FUN_00555790(int param_1,int param_2,int param_3)

{
  return ((((param_3 / 2 + 100) * *(int *)(&DAT_0092ff00 + param_1 * 0x60)) / 100) * (100 - param_2)
         ) / 100;
}

