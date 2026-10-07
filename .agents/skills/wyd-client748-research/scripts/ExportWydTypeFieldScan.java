// Instruction-level scan of the WYD 7.48 program (read-only) for packet Type
// field use, independent of the decompiler. It records:
//   cmp_mem     CMP word ptr [reg + 0x4], imm
//   cmp_reg     CMP reg, imm where reg was loaded from word ptr [x + 0x4]
//   switch_base SUB/ADD on such a register (the base of a compiled switch)
//   case_label  switch case labels (caseD_<value>) in functions that load Type
//   store       MOV word ptr [x + off], imm (header or prebuilt packet write)
// Values outside 0x100..0xFFFF are skipped. The scan is evidence to compare
// with the decompiler-based census, not a contract by itself.
//
// Arguments: <scan.tsv>
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.AddressSetView;
import ghidra.program.model.lang.Register;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.scalar.Scalar;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.symbol.SymbolIterator;
import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.util.HashSet;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class ExportWydTypeFieldScan extends GhidraScript {
    private static final Pattern TYPE_OPERAND = Pattern.compile("word ptr \\[[A-Z]+ \\+ 0x4\\]");
    private static final Pattern WORD_STORE = Pattern.compile("word ptr \\[[^\\]]+\\]");
    private static final Pattern CASE_LABEL = Pattern.compile("caseD_([0-9a-fA-F]+)$");

    private Register base(Register register) {
        return register == null ? null : register.getBaseRegister();
    }

    private Long scalar(Instruction instruction, int operand) {
        for (Object object : instruction.getOpObjects(operand)) {
            if (object instanceof Scalar) {
                return ((Scalar) object).getUnsignedValue();
            }
        }
        return null;
    }

    private Register register(Instruction instruction, int operand) {
        Object[] objects = instruction.getOpObjects(operand);
        if (objects.length == 1 && objects[0] instanceof Register) {
            return base((Register) objects[0]);
        }
        return null;
    }

    private boolean inRange(Long value) {
        return value != null && value >= 0x100 && value <= 0xFFFF;
    }

    private void write(PrintWriter writer, Function function, Instruction instruction, String kind, long value) {
        writer.println(function.getEntryPoint() + "\t" + function.getName() + "\t" + instruction.getAddress()
                + "\t" + kind + "\t0x" + Long.toHexString(value) + "\t" + instruction.toString().replace("\t", " "));
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 1) {
            throw new IllegalArgumentException("usage: <scan.tsv>");
        }
        int functions = 0;
        long instructions = 0;
        try (PrintWriter writer = new PrintWriter(new OutputStreamWriter(
                new FileOutputStream(new File(args[0])), StandardCharsets.UTF_8))) {
            writer.println("entry\tname\taddress\tkind\tvalue\tinstruction");
            writer.println("#program\t" + currentProgram.getExecutableSHA256() + "\t\t\t\t");
            FunctionIterator iterator = currentProgram.getFunctionManager().getFunctions(true);
            while (iterator.hasNext() && !monitor.isCancelled()) {
                Function function = iterator.next();
                functions++;
                AddressSetView body = function.getBody();
                Set<Register> typeRegisters = new HashSet<>();
                boolean loadsType = false;
                InstructionIterator listing = currentProgram.getListing().getInstructions(body, true);
                while (listing.hasNext()) {
                    Instruction instruction = listing.next();
                    instructions++;
                    String mnemonic = instruction.getMnemonicString();
                    int operands = instruction.getNumOperands();
                    String first = operands > 0 ? instruction.getDefaultOperandRepresentation(0) : "";
                    String second = operands > 1 ? instruction.getDefaultOperandRepresentation(1) : "";
                    Register destination = operands > 0 ? register(instruction, 0) : null;
                    if ((mnemonic.equals("MOV") || mnemonic.startsWith("MOVZX") || mnemonic.startsWith("MOVSX"))
                            && destination != null && TYPE_OPERAND.matcher(second).matches()) {
                        typeRegisters.add(destination);
                        loadsType = true;
                        continue;
                    }
                    if (mnemonic.equals("CMP") && TYPE_OPERAND.matcher(first).matches()) {
                        Long value = scalar(instruction, 1);
                        loadsType = true;
                        if (inRange(value)) {
                            write(writer, function, instruction, "cmp_mem", value);
                        }
                        continue;
                    }
                    if (mnemonic.equals("CMP") && destination != null && typeRegisters.contains(destination)) {
                        Long value = scalar(instruction, 1);
                        if (inRange(value)) {
                            write(writer, function, instruction, "cmp_reg", value);
                        }
                        continue;
                    }
                    if ((mnemonic.equals("SUB") || mnemonic.equals("ADD")) && destination != null
                            && typeRegisters.contains(destination)) {
                        Long value = scalar(instruction, 1);
                        if (inRange(value)) {
                            write(writer, function, instruction, "switch_base", value);
                        }
                        continue;
                    }
                    if (mnemonic.equals("MOV") && WORD_STORE.matcher(first).matches()) {
                        Long value = scalar(instruction, 1);
                        if (inRange(value) && instruction.getOpObjects(1).length == 1) {
                            write(writer, function, instruction, "store", value);
                        }
                    }
                    // Any other write to a tracked register ends its Type meaning.
                    if (destination != null && !mnemonic.equals("CMP") && !mnemonic.equals("TEST")
                            && !mnemonic.startsWith("PUSH")) {
                        typeRegisters.remove(destination);
                    }
                }
                if (!loadsType) {
                    continue;
                }
                SymbolIterator symbols = currentProgram.getSymbolTable().getSymbols(body, ghidra.program.model.symbol.SymbolType.LABEL, true);
                while (symbols.hasNext()) {
                    Symbol symbol = symbols.next();
                    Matcher matcher = CASE_LABEL.matcher(symbol.getName());
                    if (matcher.find()) {
                        long value = Long.parseLong(matcher.group(1), 16);
                        if (value >= 0x100 && value <= 0xFFFF) {
                            writer.println(function.getEntryPoint() + "\t" + function.getName() + "\t"
                                    + symbol.getAddress() + "\tcase_label\t0x" + Long.toHexString(value)
                                    + "\t" + symbol.getName());
                        }
                    }
                }
            }
        }
        println("functions=" + functions + " instructions=" + instructions);
    }
}
