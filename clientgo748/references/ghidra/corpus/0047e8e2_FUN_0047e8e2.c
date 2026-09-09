// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0047e8e2 | Name: FUN_0047e8e2


void __thiscall FUN_0047e8e2(int param_1,int param_2,undefined4 param_3)

{
  undefined4 uVar1;
  
  uVar1 = FUN_0059e5fe(param_3,param_1);
  if (param_2 == 0) {
    FUN_0058f078(param_1 + 0x28c28,uVar1);
  }
  if (param_2 == 1) {
    FUN_0058f078(param_1 + 0x28c88,uVar1);
  }
  if (param_2 == 2) {
    FUN_0058f078(param_1 + 0x28ce8,uVar1);
  }
  if (param_2 == 3) {
    FUN_0058f078(param_1 + 0x28d48,uVar1);
  }
  if (param_2 == 4) {
    FUN_0058f078(param_1 + 0x28da8,uVar1);
  }
  return;
}

