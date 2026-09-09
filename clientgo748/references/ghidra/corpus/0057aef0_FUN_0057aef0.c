// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057aef0 | Name: FUN_0057aef0


undefined8 * FUN_0057aef0(undefined8 *param_1,float *param_2,float *param_3)

{
  float fVar1;
  float fVar2;
  undefined1 in_XMM0 [16];
  undefined1 auVar3 [16];
  float fVar4;
  float fVar5;
  float fVar6;
  
  fVar1 = *param_2;
  fVar2 = param_2[1];
  fVar4 = fVar1 * *param_3 + param_3[0xc] + fVar2 * param_3[4];
  fVar5 = fVar1 * param_3[1] + param_3[0xd] + fVar2 * param_3[5];
  fVar6 = fVar1 * param_3[3] + param_3[0xf] + fVar2 * param_3[7];
  auVar3._4_4_ = fVar5;
  auVar3._0_4_ = fVar4;
  auVar3._8_4_ = fVar1 * param_3[2] + param_3[0xe] + fVar2 * param_3[6];
  auVar3._12_4_ = fVar6;
  auVar3 = rcpps(in_XMM0,auVar3);
  fVar6 = auVar3._12_4_ * auVar3._12_4_ * fVar6;
  *param_1 = CONCAT44(fVar5 * fVar6,fVar4 * fVar6);
  return param_1;
}

