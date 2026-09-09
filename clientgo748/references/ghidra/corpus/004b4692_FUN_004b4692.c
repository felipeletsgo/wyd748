// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b4692 | Name: FUN_004b4692


char * FUN_004b4692(uint param_1)

{
  if (param_1 < 0xc000008e) {
    if (param_1 == 0xc000008d) {
      return s_FLT_DENORMAL_OPERAND_005b9fb4;
    }
    if (param_1 < 0xc0000007) {
      if (param_1 == 0xc0000006) {
        return s_IN_PAGE_ERROR_005ba050;
      }
      if (param_1 < 0x80000004) {
        if (param_1 == 0x80000003) {
          return s_BREAKPOINT_005b9f90;
        }
        if (param_1 == 0x40010005) {
          return s_DBG_CONTROL_C_005ba0e4;
        }
        if (param_1 == 0x80000002) {
          return s_DATATYPE_MISALIGNMENT_005b9f9c;
        }
      }
      else {
        if (param_1 == 0x80000004) {
          return s_SINGLE_STEP_005ba0c8;
        }
        if (param_1 == 0xc0000005) {
          return s_ACCESS_VIOLATION_005b9f64;
        }
      }
    }
    else {
      switch(param_1) {
      case 0xc000001d:
        return s_ILLEGAL_INSTRUCTION_005ba03c;
      case 0xc0000025:
        return s_NONCONTINUABLE_EXCEPTION_005ba098;
      case 0xc0000026:
        return s_INVALID_DISPOSITION_005ba084;
      case 0xc000008c:
        return s_ARRAY_BOUNDS_EXCEEDED_005b9f78;
      }
    }
  }
  else {
    switch(param_1) {
    case 0xc000008e:
      return s_FLT_DIVIDE_BY_ZERO_005b9fcc;
    case 0xc000008f:
      return s_FLT_INEXACT_RESULT_005b9fe0;
    case 0xc0000090:
      return s_FLT_INVALID_OPERATION_005b9ff4;
    case 0xc0000091:
      return s_FLT_OVERFLOW_005ba00c;
    case 0xc0000092:
      return s_FLT_STACK_CHECK_005ba01c;
    case 0xc0000093:
      return s_FLT_UNDERFLOW_005ba02c;
    case 0xc0000094:
      return s_INT_DIVIDE_BY_ZERO_005ba060;
    case 0xc0000095:
      return s_INT_OVERFLOW_005ba074;
    case 0xc0000096:
      return s_PRIV_INSTRUCTION_005ba0b4;
    case 0xc00000fd:
      return s_STACK_OVERFLOW_005ba0d4;
    }
  }
  return s_unkown_exception_005ba0f4;
}

