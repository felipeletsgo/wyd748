// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055a581 | Name: FUN_0055a581


void FUN_0055a581(int param_1,char *param_2)

{
  char local_8;
  
  for (local_8 = '\0'; local_8 < '\b'; local_8 = local_8 + '\x01') {
    FUN_0055a709(param_1,(int)&PTR_DAT_005a5f04 +
                         (int)(char)(param_2[2] * '\x04' + param_2[1] * '\b' + param_2[3] * '\x02' +
                                    param_2[4]) +
                         (char)(param_2[5] + *param_2 * '\x02') * 0x10 + local_8 * 0x40,4);
    param_2 = param_2 + 6;
    param_1 = param_1 + 4;
  }
  return;
}

