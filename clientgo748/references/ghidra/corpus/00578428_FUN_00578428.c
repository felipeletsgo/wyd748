// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00578428 | Name: FUN_00578428


void __fastcall FUN_00578428(undefined4 param_1,undefined4 param_2)

{
  int *in_EAX;
  
  *(undefined4 *)(*in_EAX + 0x14) = 0x35;
  *(undefined4 *)(*in_EAX + 0x18) = param_2;
  (**(code **)*in_EAX)();
  return;
}

