// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00406fad | Name: FUN_00406fad


void __fastcall FUN_00406fad(int param_1)

{
  size_t sVar1;
  size_t sVar2;
  undefined4 local_10;
  
  sVar1 = _strlen((char *)(param_1 + 0xe4c));
  sVar2 = _strlen((char *)(param_1 + 0xf4c));
  if (*(int *)(param_1 + 0x1058) == 0) {
    FUN_0058ee20(param_1 + 0x76a,param_1 + 0xe4c);
    FUN_0058ee30(param_1 + 0x76a,param_1 + 0xf4c);
  }
  else {
    for (local_10 = 0; local_10 < (int)(sVar1 + sVar2); local_10 = local_10 + 1) {
      *(undefined1 *)(param_1 + local_10 + 0x76a) = 0x2a;
    }
    *(undefined1 *)(param_1 + sVar1 + 0x76a) = 0;
  }
  FUN_00421b8f(param_1 + 0x76a,0xffffffff,0);
  return;
}

