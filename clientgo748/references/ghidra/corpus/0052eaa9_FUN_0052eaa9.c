// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052eaa9 | Name: FUN_0052eaa9


undefined4 __thiscall FUN_0052eaa9(int param_1,undefined4 param_2,int param_3)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x214) == 0) {
    if (param_3 == 0) {
      uVar1 = 0;
    }
    else if ((uint)*(ushort *)(param_3 + 6) == *(uint *)(param_1 + 0x20)) {
      if (((*(short *)(param_3 + 4) == 0x366) || (*(short *)(param_3 + 4) == 0x367)) ||
         (*(short *)(param_3 + 4) == 0x368)) {
        if ((5000 < *(ushort *)(param_3 + 0x18)) || (5000 < *(ushort *)(param_3 + 0x18))) {
          FUN_00431159(s_Error_Position__X__d_Y__d__MSG_T_005bb8e8,*(undefined2 *)(param_3 + 0x18),
                       *(undefined2 *)(param_3 + 0x1a),*(undefined2 *)(param_3 + 4));
        }
        if ((*(int *)(param_3 + 0x14) == 0) || (*(int *)(param_3 + 0x14) == 2)) {
          uVar1 = FUN_00527dd9(param_3);
        }
        else if (*(int *)(param_3 + 0x14) == 7) {
          uVar1 = FUN_00528325(param_3);
        }
        else if (*(int *)(param_3 + 0x14) < 1) {
          uVar1 = 1;
        }
        else {
          uVar1 = FUN_005284a0(param_3);
        }
      }
      else if (*(short *)(param_3 + 4) == 0x3ca) {
        uVar1 = FUN_00529b07(param_3);
      }
      else if (*(short *)(param_3 + 4) == 0x36a) {
        uVar1 = FUN_005296e8(param_3);
      }
      else if (*(short *)(param_3 + 4) == 0x165) {
        uVar1 = FUN_00529bf8(param_3);
      }
      else if (*(short *)(param_3 + 4) == 0x182) {
        uVar1 = FUN_0052a737(param_3);
      }
      else if (*(short *)(param_3 + 4) == 0x36b) {
        uVar1 = FUN_0052b116(param_3);
      }
      else if (*(short *)(param_3 + 4) == 0x3b9) {
        uVar1 = FUN_0052b72a(param_3);
      }
      else if (*(short *)(param_3 + 4) == 0x336) {
        uVar1 = FUN_013c0000(param_3);
      }
      else if (*(short *)(param_3 + 4) == 0x181) {
        uVar1 = FUN_013c0800(param_3);
      }
      else if (*(short *)(param_3 + 4) == 0x18a) {
        uVar1 = FUN_0052cbaa(param_3);
      }
      else if (*(short *)(param_3 + 4) == 0x333) {
        uVar1 = FUN_0052d060(param_3);
      }
      else if (*(short *)(param_3 + 4) == 0x334) {
        uVar1 = FUN_0052d2c8(param_3);
      }
      else if (*(short *)(param_3 + 4) == 0x337) {
        uVar1 = FUN_013c0b00(param_3);
      }
      else if (*(short *)(param_3 + 4) == 0x3af) {
        uVar1 = FUN_0052dc00(param_3);
      }
      else if (*(short *)(param_3 + 4) == 899) {
        uVar1 = FUN_0052dc5d(param_3);
      }
      else if (*(short *)(param_3 + 4) == 900) {
        uVar1 = FUN_0052e2f6(param_3);
      }
      else if (*(short *)(param_3 + 4) == 0x185) {
        uVar1 = FUN_0052e3c8(param_3);
      }
      else if (*(short *)(param_3 + 4) == 0x386) {
        uVar1 = FUN_0052e684(param_3);
      }
      else if (*(short *)(param_3 + 4) == 0x193) {
        uVar1 = FUN_0052e6c6(param_3);
      }
      else if (*(short *)(param_3 + 4) == 0x39f) {
        uVar1 = FUN_0052e722(param_3);
      }
      else if (*(short *)(param_3 + 4) == 0x3ad) {
        uVar1 = FUN_0052e85d(param_3);
      }
      else {
        uVar1 = 0;
      }
    }
    else {
      uVar1 = 0;
    }
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

