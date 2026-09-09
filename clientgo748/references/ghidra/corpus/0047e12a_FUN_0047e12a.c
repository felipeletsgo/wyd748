// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0047e12a | Name: FUN_0047e12a


void FUN_0047e12a(int param_1,undefined4 param_2,int param_3)

{
  if (param_1 != 0) {
    *(undefined4 *)(param_1 + 0x1c4) = param_2;
    if (param_3 == 0x1fd) {
      *(undefined4 *)(param_1 + 0x1c8) = 1;
    }
    else if ((param_3 < 0x20e) || (0x213 < param_3)) {
      *(undefined4 *)(param_1 + 0x1c8) = 3;
    }
    else {
      *(undefined4 *)(param_1 + 0x1c8) = 2;
    }
  }
  return;
}

