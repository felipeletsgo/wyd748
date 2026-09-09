// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00573d16 | Name: FUN_00573d16


undefined4
FUN_00573d16(int param_1,uint *param_2,uint *param_3,uint *param_4,uint *param_5,uint *param_6,
            uint *param_7,uint *param_8,uint *param_9)

{
  uint uVar1;
  uint uVar2;
  undefined4 uVar3;
  
  if ((((param_1 == 0) || (param_2 == (uint *)0x0)) || (param_3 == (uint *)0x0)) ||
     (((param_4 == (uint *)0x0 || (param_5 == (uint *)0x0)) || (param_6 == (uint *)0x0)))) {
    uVar3 = 0;
  }
  else {
    *param_3 = *param_2;
    *param_4 = param_2[1];
    *param_5 = (uint)(byte)param_2[6];
    *param_6 = (uint)*(byte *)((int)param_2 + 0x19);
    if (param_8 != (uint *)0x0) {
      *param_8 = (uint)*(byte *)((int)param_2 + 0x1a);
    }
    if (param_9 != (uint *)0x0) {
      *param_9 = (uint)*(byte *)((int)param_2 + 0x1b);
    }
    if (param_7 != (uint *)0x0) {
      *param_7 = (uint)(byte)param_2[7];
    }
    uVar1 = *param_6;
    if (uVar1 == 3) {
      uVar2 = 1;
    }
    else {
      uVar2 = (int)(char)uVar1 & 2U | 1;
    }
    if ((uVar1 & 4) != 0) {
      uVar2 = uVar2 + 1;
    }
    if ((uint)(0x7fffffff / (ulonglong)(uint)((int)(*param_5 * uVar2 + 7) >> 3)) < *param_3) {
      FUN_005733ef(param_1,"Width too large for libpng to process image data.");
    }
    uVar3 = 1;
  }
  return uVar3;
}

