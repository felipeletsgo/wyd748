// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b6be0 | Name: FUN_004b6be0


int __thiscall FUN_004b6be0(int param_1,undefined4 param_2,undefined4 param_3)

{
  char cVar1;
  
  cVar1 = FUN_004b6d70(param_3,1);
  if (cVar1 != '\0') {
    FUN_004b66f0(*(undefined4 *)(param_1 + 4),param_2,param_3);
    FUN_004b6d20(param_3);
  }
  return param_1;
}

