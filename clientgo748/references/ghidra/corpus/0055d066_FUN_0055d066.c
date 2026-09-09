// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055d066 | Name: FUN_0055d066


undefined4 __fastcall FUN_0055d066(int param_1)

{
  int local_8;
  
  for (local_8 = 0; local_8 < 100; local_8 = local_8 + 1) {
    FUN_0058ffd8(*(undefined4 *)(&DAT_0065a4a0 + local_8 * 900));
    FUN_0058ffd8(*(undefined4 *)(&DAT_0065a794 + local_8 * 900));
    FUN_0058ffd8(*(undefined4 *)(&DAT_0065a798 + local_8 * 900));
  }
  if (DAT_013b71e8 != (undefined4 *)0x0) {
    if (DAT_013b71e8 != (undefined4 *)0x0) {
      (**(code **)*DAT_013b71e8)(1);
    }
    DAT_013b71e8 = (undefined4 *)0x0;
  }
  if (*(int *)(param_1 + 0xf4) != 0) {
    if (*(undefined4 **)(param_1 + 0xf4) != (undefined4 *)0x0) {
      (**(code **)**(undefined4 **)(param_1 + 0xf4))(1);
    }
    *(undefined4 *)(param_1 + 0xf4) = 0;
  }
  if (*(int *)(param_1 + 0xe0) != 0) {
    if (*(undefined4 **)(param_1 + 0xe0) != (undefined4 *)0x0) {
      (**(code **)**(undefined4 **)(param_1 + 0xe0))(1);
    }
    *(undefined4 *)(param_1 + 0xe0) = 0;
  }
  if (*(int *)(param_1 + 0xe4) != 0) {
    if (*(int *)(param_1 + 0xe4) != 0) {
      FUN_0055f410(1);
    }
    *(undefined4 *)(param_1 + 0xe4) = 0;
  }
  if (*(int *)(param_1 + 0xfc) != 0) {
    if (*(int *)(param_1 + 0xfc) != 0) {
      FUN_0055f440(1);
    }
    *(undefined4 *)(param_1 + 0xfc) = 0;
  }
  if (*(int *)(param_1 + 0xec) != 0) {
    if (*(int *)(param_1 + 0xec) != 0) {
      FUN_00493cf0(1);
    }
    *(undefined4 *)(param_1 + 0xec) = 0;
  }
  if (*(int *)(param_1 + 0xe8) != 0) {
    if (*(undefined4 **)(param_1 + 0xe8) != (undefined4 *)0x0) {
      (**(code **)**(undefined4 **)(param_1 + 0xe8))(1);
    }
    *(undefined4 *)(param_1 + 0xe8) = 0;
  }
  if (*(int *)(param_1 + 0xf0) != 0) {
    if (*(undefined4 **)(param_1 + 0xf0) != (undefined4 *)0x0) {
      (**(code **)**(undefined4 **)(param_1 + 0xf0))(1);
    }
    *(undefined4 *)(param_1 + 0xf0) = 0;
  }
  DAT_013b71e4 = 0;
  DeleteObject(*(HGDIOBJ *)(param_1 + 0x110));
  if (DAT_013b736c == 1) {
    FUN_0059dfe0(DAT_013b7364);
  }
  return 1;
}

