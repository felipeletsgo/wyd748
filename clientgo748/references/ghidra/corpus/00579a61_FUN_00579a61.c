// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00579a61 | Name: FUN_00579a61


undefined4 * FUN_00579a61(undefined4 param_1,undefined4 *param_2,undefined1 param_3,uint param_4)

{
  uint uVar1;
  undefined4 *puVar2;
  
  puVar2 = param_2;
  for (uVar1 = param_4 >> 2; uVar1 != 0; uVar1 = uVar1 - 1) {
    *puVar2 = CONCAT22(CONCAT11(param_3,param_3),CONCAT11(param_3,param_3));
    puVar2 = puVar2 + 1;
  }
  for (param_4 = param_4 & 3; param_4 != 0; param_4 = param_4 - 1) {
    *(undefined1 *)puVar2 = param_3;
    puVar2 = (undefined4 *)((int)puVar2 + 1);
  }
  return param_2;
}

