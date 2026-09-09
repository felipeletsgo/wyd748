// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040dd00 | Name: FUN_0040dd00


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
FUN_0040dd00(int param_1,undefined4 param_2,float param_3,float param_4,undefined4 param_5,
            undefined4 param_6)

{
  float fVar1;
  float fVar2;
  size_t sVar3;
  
  if (((*(int *)(param_1 + 0x670) != 0) && (**(short **)(param_1 + 0x670) < 0x1965)) &&
     (-1 < **(short **)(param_1 + 0x670))) {
    if (*(short *)(&DAT_00d44990 + **(short **)(param_1 + 0x670) * 0x8c) < 0) {
      *(float *)(param_1 + 0x68) = param_3 + *(float *)(param_1 + 0x4c);
      *(float *)(param_1 + 0x6c) = param_4 + *(float *)(param_1 + 0x50);
      *(undefined4 *)(param_1 + 0x70) = *(undefined4 *)(param_1 + 0x54);
      *(undefined4 *)(param_1 + 0x74) = *(undefined4 *)(param_1 + 0x58);
      *(undefined4 *)(param_1 + 0x1ac) = param_5;
      FUN_0040c26d(param_2,param_1 + 100,param_5);
    }
    else {
      if (((DAT_005b892c == 2) && (*(int *)(param_1 + 0x1e4) != 0)) &&
         (*(int *)(*(int *)(param_1 + 0x1e4) + 0x1f8) != 0)) {
        fVar1 = (*(float *)(param_1 + 0x54) * _DAT_005a34a0) / DAT_005b490c;
        fVar2 = (*(float *)(param_1 + 0x58) * _DAT_005a34a0) / DAT_005b4910;
      }
      else {
        fVar1 = *(float *)(param_1 + 0x54) * _DAT_005a34a0;
        fVar2 = *(float *)(param_1 + 0x58) * _DAT_005a34a0;
      }
      param_4 = fVar2 + param_4;
      param_3 = fVar1 + param_3;
      FUN_00401c40(param_2,param_3,param_4,param_5,param_6);
      sVar3 = _strlen((char *)(param_1 + 0x39a));
      if (sVar3 != 0) {
        *(float *)(param_1 + 0x358) = param_3 + *(float *)(param_1 + 0x4c);
        *(float *)(param_1 + 0x35c) = param_4 + *(float *)(param_1 + 0x50);
        *(undefined4 *)(param_1 + 0x49c) = param_5;
        FUN_0040c26d(param_2,param_1 + 0x354,param_5);
      }
    }
  }
  return;
}

