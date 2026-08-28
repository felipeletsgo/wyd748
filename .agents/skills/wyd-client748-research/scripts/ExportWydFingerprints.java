// Exporta fingerprints estruturais de todas as funcoes do programa atual.
// O resultado serve somente para correlacao diferencial: nomes e similaridade
// estrutural nao provam comportamento, ABI, ownership ou contrato do client 7.48.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressSetView;
import ghidra.program.model.block.BasicBlockModel;
import ghidra.program.model.block.CodeBlock;
import ghidra.program.model.block.CodeBlockIterator;
import ghidra.program.model.block.CodeBlockReference;
import ghidra.program.model.block.CodeBlockReferenceIterator;
import ghidra.program.model.lang.Register;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.FunctionManager;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Listing;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.mem.MemoryAccessException;
import ghidra.program.model.pcode.PcodeOp;
import ghidra.program.model.pcode.Varnode;
import ghidra.program.model.scalar.Scalar;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;
import ghidra.program.model.symbol.ReferenceManager;
import ghidra.program.model.symbol.Symbol;
import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.ArrayList;
import java.util.Base64;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeSet;

public class ExportWydFingerprints extends GhidraScript {
    private static final String SCHEMA_VERSION = "1";

    private static class BlockRecord {
        private final Address start;
        private final int instructionCount;
        private final List<Address> destinations = new ArrayList<>();

        BlockRecord(Address start, int instructionCount) {
            this.start = start;
            this.instructionCount = instructionCount;
        }
    }

    private static class FunctionRecord {
        private final Function function;
        private int instructionCount;
        private int blockCount;
        private int edgeCount;
        private int incomingDataRefs;
        private String bytesHash;
        private String mnemonicHash;
        private String normalizedHash;
        private String pcodeHash;
        private String cfgHash;
        private final Set<String> strings = new TreeSet<>();
        private final Set<String> imports = new TreeSet<>();
        private final Set<String> constants = new TreeSet<>();

        FunctionRecord(Function function) {
            this.function = function;
        }
    }

    private String safeValue(Object value) {
        return value == null ? "" : value.toString()
                .replace("\t", " ").replace("\r", " ").replace("\n", " ");
    }

    private String address(Address value) {
        return value == null ? "" : value.toString().toUpperCase();
    }

    private String sha256(String value) throws Exception {
        MessageDigest digest = MessageDigest.getInstance("SHA-256");
        byte[] bytes = digest.digest(value.getBytes(StandardCharsets.UTF_8));
        StringBuilder result = new StringBuilder(bytes.length * 2);
        for (byte item : bytes) {
            result.append(String.format("%02X", Byte.toUnsignedInt(item)));
        }
        return result.toString();
    }

    private String encodedToken(String value) {
        // Tokens codificados evitam que strings do jogo quebrem o delimitador
        // interno ou o TSV e continuam reversiveis para revisao manual.
        return Base64.getUrlEncoder().withoutPadding().encodeToString(
                value.getBytes(StandardCharsets.UTF_8));
    }

    private String join(Set<String> values) {
        return String.join("|", values);
    }

    private boolean isLoadedAddress(long value, Memory memory) {
        try {
            Address candidate = currentProgram.getAddressFactory()
                    .getDefaultAddressSpace().getAddress(value);
            return candidate != null && memory.contains(candidate);
        }
        catch (RuntimeException exception) {
            return false;
        }
    }

    private String normalizeScalar(Scalar scalar, Memory memory) {
        long unsigned = scalar.getUnsignedValue();
        long signed = scalar.getSignedValue();
        if (isLoadedAddress(unsigned, memory)) {
            return "<MEM>";
        }
        // Offsets e constantes pequenas carregam informacao de estruturas e
        // branches; enderecos absolutos carregados precisam ser relocaveis.
        if (signed >= -0x100000L && signed <= 0x100000L) {
            return signed < 0 ? Long.toString(signed)
                    : "0x" + Long.toUnsignedString(unsigned, 16).toUpperCase();
        }
        return "<IMM>";
    }

    private String normalizeAddress(Address value, Listing listing, Memory memory) {
        if (value == null) {
            return "<NULL>";
        }
        if (value.isExternalAddress()) {
            Symbol symbol = currentProgram.getSymbolTable().getPrimarySymbol(value);
            return "<EXT:" + (symbol == null ? "unknown" : safeValue(symbol.getName())) + ">";
        }
        if (memory.contains(value)) {
            Function target = currentProgram.getFunctionManager().getFunctionAt(value);
            if (target != null || listing.getInstructionAt(value) != null) {
                return "<CODE>";
            }
            return "<DATA>";
        }
        return "<ADDR>";
    }

    private String normalizeOperand(Object[] objects, Listing listing, Memory memory) {
        List<String> parts = new ArrayList<>();
        for (Object object : objects) {
            if (object instanceof Register) {
                parts.add("R:" + ((Register) object).getName().toUpperCase());
            }
            else if (object instanceof Scalar) {
                parts.add("S:" + normalizeScalar((Scalar) object, memory));
            }
            else if (object instanceof Address) {
                parts.add("A:" + normalizeAddress((Address) object, listing, memory));
            }
            else {
                parts.add("O:" + safeValue(object).toUpperCase());
            }
        }
        return String.join(",", parts);
    }

    private String normalizeVarnode(Varnode varnode, Memory memory,
            Map<String, Integer> uniqueIds) {
        if (varnode == null) {
            return "-";
        }
        if (varnode.isConstant()) {
            long value = varnode.getOffset();
            if (isLoadedAddress(value, memory)) {
                return "C:<MEM>:" + varnode.getSize();
            }
            long signed = value;
            if (varnode.getSize() < Long.BYTES) {
                int bits = varnode.getSize() * 8;
                long signBit = 1L << (bits - 1);
                long mask = (1L << bits) - 1;
                signed = (value & signBit) == 0 ? value : value | ~mask;
            }
            if (signed >= -0x100000L && signed <= 0x100000L) {
                return "C:" + signed + ":" + varnode.getSize();
            }
            return "C:<IMM>:" + varnode.getSize();
        }
        if (varnode.isUnique()) {
            String key = varnode.getAddress() + ":" + varnode.getSize();
            Integer id = uniqueIds.get(key);
            if (id == null) {
                id = uniqueIds.size();
                uniqueIds.put(key, id);
            }
            return "U:" + id + ":" + varnode.getSize();
        }
        if (varnode.isRegister()) {
            Register register = currentProgram.getRegister(varnode.getAddress(), varnode.getSize());
            return "R:" + (register == null ? varnode.getOffset() : register.getName())
                    + ":" + varnode.getSize();
        }
        String space = varnode.getAddress().getAddressSpace().getName().toUpperCase();
        if ("STACK".equals(space)) {
            return "STACK:" + varnode.getOffset() + ":" + varnode.getSize();
        }
        if (memory.contains(varnode.getAddress())) {
            return "MEM:<ADDR>:" + varnode.getSize();
        }
        return space + ":" + varnode.getSize();
    }

    private String functionList(Set<Function> functions) {
        List<String> labels = new ArrayList<>();
        for (Function function : functions) {
            labels.add(address(function.getEntryPoint()) + ":" + safeValue(function.getName(true)));
        }
        Collections.sort(labels);
        return String.join(";", labels);
    }

    private void collectReferenceEvidence(Instruction instruction, FunctionRecord record,
            Listing listing, ReferenceManager references) {
        for (Reference reference : references.getReferencesFrom(instruction.getAddress())) {
            Address target = reference.getToAddress();
            if (target == null) {
                continue;
            }
            if (target.isExternalAddress()) {
                Symbol symbol = currentProgram.getSymbolTable().getPrimarySymbol(target);
                String name = symbol == null ? target.toString() : symbol.getName(true);
                record.imports.add(encodedToken(safeValue(name)));
                continue;
            }
            Function targetFunction = currentProgram.getFunctionManager().getFunctionAt(target);
            if (targetFunction != null && targetFunction.isExternal()) {
                record.imports.add(encodedToken(safeValue(targetFunction.getName(true))));
            }
            if (!reference.getReferenceType().isData()) {
                continue;
            }
            Data data = listing.getDataAt(target);
            if (data == null) {
                data = listing.getDataContaining(target);
            }
            if (data != null && data.getValue() instanceof String) {
                record.strings.add(encodedToken((String) data.getValue()));
            }
        }
    }

    private List<BlockRecord> collectBlocks(Function function, Listing listing) throws Exception {
        BasicBlockModel model = new BasicBlockModel(currentProgram);
        List<BlockRecord> result = new ArrayList<>();
        CodeBlockIterator iterator = model.getCodeBlocksContaining(function.getBody(), monitor);
        while (iterator.hasNext() && !monitor.isCancelled()) {
            CodeBlock block = iterator.next();
            int instructions = 0;
            InstructionIterator blockInstructions = listing.getInstructions(
                    (AddressSetView) block, true);
            while (blockInstructions.hasNext()) {
                blockInstructions.next();
                instructions++;
            }
            BlockRecord record = new BlockRecord(block.getFirstStartAddress(), instructions);
            CodeBlockReferenceIterator destinations = block.getDestinations(monitor);
            while (destinations.hasNext()) {
                CodeBlockReference reference = destinations.next();
                CodeBlock destination = reference.getDestinationBlock();
                Address destinationStart = destination == null
                        ? null : destination.getFirstStartAddress();
                if (destinationStart != null && function.getBody().contains(destinationStart)) {
                    record.destinations.add(destinationStart);
                }
            }
            result.add(record);
        }
        result.sort(Comparator.comparing(item -> item.start));
        return result;
    }

    private FunctionRecord fingerprint(Function function, Listing listing, Memory memory,
            ReferenceManager references) throws Exception {
        FunctionRecord record = new FunctionRecord(function);
        StringBuilder rawBytes = new StringBuilder();
        StringBuilder mnemonics = new StringBuilder();
        StringBuilder normalized = new StringBuilder();
        StringBuilder pcode = new StringBuilder();
        Map<String, Integer> uniqueIds = new LinkedHashMap<>();

        InstructionIterator instructions = listing.getInstructions(function.getBody(), true);
        while (instructions.hasNext() && !monitor.isCancelled()) {
            Instruction instruction = instructions.next();
            record.instructionCount++;
            try {
                for (byte value : instruction.getBytes()) {
                    rawBytes.append(String.format("%02X", Byte.toUnsignedInt(value)));
                }
            }
            catch (MemoryAccessException exception) {
                rawBytes.append("<UNREADABLE>");
            }

            String mnemonic = instruction.getMnemonicString().toUpperCase();
            mnemonics.append(mnemonic).append(';');
            normalized.append(mnemonic);
            for (int operand = 0; operand < instruction.getNumOperands(); operand++) {
                normalized.append(operand == 0 ? ' ' : ',')
                        .append(normalizeOperand(instruction.getOpObjects(operand), listing, memory));
                for (Object object : instruction.getOpObjects(operand)) {
                    if (object instanceof Scalar) {
                        String value = normalizeScalar((Scalar) object, memory);
                        if (!"<MEM>".equals(value) && !"<IMM>".equals(value)) {
                            record.constants.add(value);
                        }
                    }
                }
            }
            normalized.append(';');

            for (PcodeOp operation : instruction.getPcode()) {
                pcode.append(operation.getMnemonic()).append('(')
                        .append(normalizeVarnode(operation.getOutput(), memory, uniqueIds));
                for (int input = 0; input < operation.getNumInputs(); input++) {
                    pcode.append(',').append(normalizeVarnode(
                            operation.getInput(input), memory, uniqueIds));
                }
                pcode.append(");");
            }
            collectReferenceEvidence(instruction, record, listing, references);
        }

        List<BlockRecord> blocks = collectBlocks(function, listing);
        Map<Address, Integer> blockIds = new HashMap<>();
        for (int index = 0; index < blocks.size(); index++) {
            blockIds.put(blocks.get(index).start, index);
        }
        StringBuilder cfg = new StringBuilder();
        for (int index = 0; index < blocks.size(); index++) {
            BlockRecord block = blocks.get(index);
            cfg.append('B').append(index).append(':').append(block.instructionCount).append(';');
            List<Integer> destinations = new ArrayList<>();
            for (Address destination : block.destinations) {
                Integer destinationId = blockIds.get(destination);
                if (destinationId != null) {
                    destinations.add(destinationId);
                }
            }
            Collections.sort(destinations);
            for (Integer destination : destinations) {
                cfg.append('E').append(index).append('>').append(destination).append(';');
                record.edgeCount++;
            }
        }
        record.blockCount = blocks.size();

        ReferenceIterator incoming = references.getReferencesTo(function.getEntryPoint());
        while (incoming.hasNext()) {
            if (incoming.next().getReferenceType().isData()) {
                record.incomingDataRefs++;
            }
        }

        record.bytesHash = sha256(rawBytes.toString());
        record.mnemonicHash = sha256(mnemonics.toString());
        record.normalizedHash = sha256(normalized.toString());
        record.pcodeHash = sha256(pcode.toString());
        record.cfgHash = sha256(cfg.toString());
        return record;
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length != 1) {
            throw new IllegalArgumentException("uso: <arquivo-saida.tsv>");
        }

        File output = new File(args[0]);
        if (output.getParentFile() != null) {
            output.getParentFile().mkdirs();
        }
        FunctionManager functions = currentProgram.getFunctionManager();
        Listing listing = currentProgram.getListing();
        Memory memory = currentProgram.getMemory();
        ReferenceManager references = currentProgram.getReferenceManager();
        List<Function> allFunctions = new ArrayList<>();
        FunctionIterator iterator = functions.getFunctions(true);
        while (iterator.hasNext() && !monitor.isCancelled()) {
            allFunctions.add(iterator.next());
        }
        allFunctions.sort(Comparator.comparing(Function::getEntryPoint));

        try (PrintWriter writer = new PrintWriter(new OutputStreamWriter(
                new FileOutputStream(output), StandardCharsets.UTF_8))) {
            writer.println("# schema_version=" + SCHEMA_VERSION);
            writer.println("# program_name=" + safeValue(currentProgram.getName()));
            writer.println("# program_sha256=" + safeValue(currentProgram.getExecutableSHA256()).toUpperCase());
            writer.println("# image_base=" + address(currentProgram.getImageBase()));
            writer.println("entry\tsymbol\tfull_symbol\tnamespace\tcalling_convention\tsignature"
                    + "\tbody_min\tbody_max\tbody_address_count\tinstruction_count"
                    + "\tbasic_block_count\tcfg_edge_count\tincoming_data_refs\tis_thunk"
                    + "\tthunk_target\tbytes_sha256\tmnemonic_sha256\tnormalized_sha256"
                    + "\tpcode_sha256\tcfg_sha256\tstrings\timports\tconstants"
                    + "\tdirect_callers_count\tdirect_callers\tdirect_callees_count\tdirect_callees");

            int exported = 0;
            for (Function function : allFunctions) {
                if (monitor.isCancelled()) {
                    break;
                }
                FunctionRecord record = fingerprint(function, listing, memory, references);
                Set<Function> callers = function.getCallingFunctions(monitor);
                Set<Function> callees = function.getCalledFunctions(monitor);
                Function thunkTarget = function.isThunk()
                        ? function.getThunkedFunction(true) : null;
                writer.println(address(function.getEntryPoint()) + "\t"
                        + safeValue(function.getName()) + "\t"
                        + safeValue(function.getName(true)) + "\t"
                        + safeValue(function.getParentNamespace().getName(true)) + "\t"
                        + safeValue(function.getCallingConventionName()) + "\t"
                        + safeValue(function.getSignature()) + "\t"
                        + address(function.getBody().getMinAddress()) + "\t"
                        + address(function.getBody().getMaxAddress()) + "\t"
                        + function.getBody().getNumAddresses() + "\t"
                        + record.instructionCount + "\t" + record.blockCount + "\t"
                        + record.edgeCount + "\t" + record.incomingDataRefs + "\t"
                        + function.isThunk() + "\t"
                        + (thunkTarget == null ? "" : address(thunkTarget.getEntryPoint())) + "\t"
                        + record.bytesHash + "\t" + record.mnemonicHash + "\t"
                        + record.normalizedHash + "\t" + record.pcodeHash + "\t"
                        + record.cfgHash + "\t" + join(record.strings) + "\t"
                        + join(record.imports) + "\t" + join(record.constants) + "\t"
                        + callers.size() + "\t" + functionList(callers) + "\t"
                        + callees.size() + "\t" + functionList(callees));
                exported++;
            }
            println("Fingerprints exportados: " + exported + "/" + allFunctions.size()
                    + " em " + output.getAbsolutePath());
        }
    }
}
