// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b5b5b | Name: FUN_004b5b5b


void FUN_004b5b5b(int param_1,int param_2)

{
  undefined1 local_1c [12];
  undefined1 local_10 [12];
  
  if (param_1 != 0) {
    FUN_00591e87(local_10);
    FUN_00591e0e(local_1c);
    if (param_2 == 0) {
      FUN_0058f6a9(param_1,s__s__s_005ba560,local_10,local_1c);
    }
    else {
      FUN_0058f6a9(param_1,s_date___s__time___s__005ba568,local_10,local_1c);
    }
  }
  return;
}

