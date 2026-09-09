// Exporta call graph e referências de uma função do WYD 7.48 sem alterar o
// programa analisado. O relatório complementa, mas não substitui, a revisão no
// projeto Ghidra para chamadas indiretas, tipos e estruturas.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressOverflowException;
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
import ghidra.program.model.mem.MemoryBlock;
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
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

public class ExportWydFlow extends GhidraScript {
    // Uma vtable 7.48 cabe normalmente nesta vizinhanca para fins de orientacao.
    // Xrefs decisivos devem ser pedidos pelo endereco exato do slot, evitando
    // exports massivos que escondem a evidencia relevante.
    private static final int DATA_WINDOW_BYTES = 0x80;
    private static final int TABLE_SLOT_BYTES = 4;
    private static final int MAX_TABLE_SLOTS = 4096;

    private static class BodyReferenceHit {
        private final Reference reference;
        private final Address target;

        BodyReferenceHit(Reference reference, Address target) {
            this.reference = reference;
            this.target = target;
        }
    }

    private String describeFunction(Function function) {
        return function.getEntryPoint() + "\t" + function.getName();
    }

    private String safeValue(Object value) {
        return value == null ? "" : value.toString()
                .replace("\t", " ").replace("\r", " ").replace("\n", " ");
    }

    private String symbolName(Address address) {
        Symbol symbol = currentProgram.getSymbolTable().getPrimarySymbol(address);
        return symbol == null ? "" : symbol.getName();
    }

    private String bytesHex(Instruction instruction) {
        // Bytes e texto precisam aparecer juntos: o texto recupera registradores
        // e indirections, enquanto os bytes tornam o export auditavel mesmo se a
        // formatacao da listing mudar entre revisoes do projeto.
        try {
            byte[] bytes = instruction.getBytes();
            StringBuilder result = new StringBuilder(bytes.length * 2);
            for (byte value : bytes) {
                result.append(String.format("%02X", Byte.toUnsignedInt(value)));
            }
            return result.toString();
        }
        catch (MemoryAccessException exception) {
            return "ERROR:" + safeValue(exception.getMessage());
        }
    }

    private String addressList(Address[] addresses) {
        StringBuilder result = new StringBuilder();
        for (Address address : addresses) {
            if (result.length() != 0) {
                result.append(',');
            }
            result.append(address);
        }
        return result.toString();
    }

    private String objectList(Object[] objects) {
        StringBuilder result = new StringBuilder();
        for (Object object : objects) {
            if (result.length() != 0) {
                result.append(',');
            }
            result.append(safeValue(object));
        }
        return result.toString();
    }

    private String registerList(Object[] objects) {
        StringBuilder result = new StringBuilder();
        for (Object object : objects) {
            if (!(object instanceof Register)) {
                continue;
            }
            if (result.length() != 0) {
                result.append(',');
            }
            result.append(((Register) object).getName());
        }
        return result.toString();
    }

    private String scalarList(Object[] objects) {
        StringBuilder result = new StringBuilder();
        for (Object object : objects) {
            if (!(object instanceof Scalar)) {
                continue;
            }
            Scalar scalar = (Scalar) object;
            if (result.length() != 0) {
                result.append(',');
            }
            result.append("0x").append(Long.toUnsignedString(
                    scalar.getUnsignedValue(), 16));
        }
        return result.toString();
    }

    private void writeIncoming(PrintWriter writer, String record, Address subject,
            String subjectName, ReferenceManager references, FunctionManager functions) {
        ReferenceIterator incoming = references.getReferencesTo(subject);
        while (incoming.hasNext() && !monitor.isCancelled()) {
            Reference reference = incoming.next();
            Address from = reference.getFromAddress();
            Function owner = functions.getFunctionContaining(from);
            String source = owner != null ? owner.getName() : symbolName(from);
            writer.println(record + "\t" + subject + "\t" + subjectName + "\t"
                    + from + "\t" + subject + "\t" + reference.getReferenceType()
                    + "\t" + source + "\t");
        }
    }

    private void writeOutgoing(PrintWriter writer, Address subject,
            ReferenceManager references) {
        for (Reference reference : references.getReferencesFrom(subject)) {
            writer.println("slot_outgoing\t" + subject + "\t" + symbolName(subject)
                    + "\t" + subject + "\t" + reference.getToAddress() + "\t"
                    + reference.getReferenceType() + "\t"
                    + symbolName(reference.getToAddress()) + "\t");
        }
    }

    private byte[] pointerBytes(Address target) {
        // O client 7.48 e Win32. Codificar o VA segundo o endian do programa
        // evita procurar a representacao textual do endereco ou assumir host.
        byte[] result = new byte[4];
        long value = target.getOffset();
        boolean bigEndian = currentProgram.getLanguage().isBigEndian();
        for (int index = 0; index < result.length; index++) {
            int shift = bigEndian ? (result.length - index - 1) * 8 : index * 8;
            result[index] = (byte) (value >>> shift);
        }
        return result;
    }

    private void writePointerHits(PrintWriter writer, Address target,
            ReferenceManager references, FunctionManager functions, Memory memory) {
        // Uma callback sem xref tipado pode existir somente como VA bruto em
        // vtable ou tabela de dispatch. A busca e pelo alvo exato e registra o
        // contexto; cada hit ainda precisa ser classificado no projeto Ghidra.
        Address cursor = currentProgram.getMinAddress();
        Address end = currentProgram.getMaxAddress();
        byte[] pattern = pointerBytes(target);
        int hits = 0;
        while (cursor.compareTo(end) <= 0 && !monitor.isCancelled()) {
            Address hit = memory.findBytes(cursor, end, pattern, null, true, monitor);
            if (hit == null) {
                break;
            }
            hits++;
            Function owner = functions.getFunctionContaining(hit);
            Data data = currentProgram.getListing().getDefinedDataContaining(hit);
            String source = owner != null ? owner.getName() : symbolName(hit);
            String block = memory.getBlock(hit) == null ? "" : memory.getBlock(hit).getName();
            writer.println("pointer_hit\t" + target + "\t" + symbolName(target) + "\t"
                    + hit + "\t" + target + "\tpointer\t" + source + "\t"
                    + block + (data == null ? "" : ":" + safeValue(data.getValue())));
            writeIncoming(writer, "pointer_incoming", hit, symbolName(hit),
                    references, functions);
            writeOutgoing(writer, hit, references);
            if (hit.equals(end)) {
                break;
            }
            cursor = hit.add(1);
        }
        // Registrar zero explicitamente torna o relatório reproduzível sem
        // transformar a ausência de hit bruto em prova de ausência de caller.
        writer.println("pointer_search\t" + target + "\t" + symbolName(target)
                + "\t\t" + target + "\tpointer_scan\t\thits=" + hits);
    }

    private long readSignedRel32(Address instructionAddress, Memory memory)
            throws MemoryAccessException {
        // E8/E9 guardam um displacement assinado de 32 bits no endian do
        // programa. A extensao de sinal precisa ocorrer antes de somar o VA da
        // proxima instrucao, inclusive para chamadas que apontam para tras.
        long raw = 0;
        boolean bigEndian = currentProgram.getLanguage().isBigEndian();
        for (int index = 0; index < 4; index++) {
            int shift = bigEndian ? (3 - index) * 8 : index * 8;
            raw |= (long) Byte.toUnsignedInt(
                    memory.getByte(instructionAddress.add(index + 1))) << shift;
        }
        return (long) (int) raw;
    }

    private void writeRelativeHits(PrintWriter writer, Address target,
            FunctionManager functions, Listing listing, Memory memory) {
        // O scan por VA bruto nao cobre CALL/JMP rel32, pois o binario armazena
        // somente o deslocamento a partir da proxima instrucao. Percorrer a
        // listing evita promover bytes E8/E9 encontrados em dados a callers.
        InstructionIterator instructions = listing.getInstructions(true);
        int decoded = 0;
        int candidates = 0;
        int errors = 0;
        int hits = 0;
        while (instructions.hasNext() && !monitor.isCancelled()) {
            Instruction instruction = instructions.next();
            decoded++;
            if (instruction.getLength() != 5) {
                continue;
            }

            Address from = instruction.getAddress();
            int opcode;
            long displacement;
            try {
                opcode = Byte.toUnsignedInt(memory.getByte(from));
                if (opcode != 0xE8 && opcode != 0xE9) {
                    continue;
                }
                candidates++;
                displacement = readSignedRel32(from, memory);
            }
            catch (MemoryAccessException exception) {
                errors++;
                continue;
            }

            Address next = from.add(5);
            // O executavel analisado e x86 de 32 bits; preservar o wrap de VA
            // torna o calculo equivalente ao branch relativo do processador.
            long resolvedOffset = (next.getOffset() + displacement) & 0xffffffffL;
            Address resolved = next.getAddressSpace().getAddress(resolvedOffset);
            if (!resolved.equals(target)) {
                continue;
            }

            hits++;
            Function owner = functions.getFunctionContaining(from);
            String ownerName = owner == null ? symbolName(from) : owner.getName();
            String ownerEntry = owner == null ? "" : owner.getEntryPoint().toString();
            String type = opcode == 0xE8 ? "CALL_REL32" : "JMP_REL32";
            writer.println("relative_hit\t" + target + "\t" + symbolName(target)
                    + "\t" + from + "\t" + resolved + "\t" + type + "\t"
                    + ownerName + "\towner_entry=" + ownerEntry + ";next=" + next
                    + ";displacement=" + displacement);
        }
        // Zero hits elimina apenas branches E8/E9 que o Ghidra decodificou na
        // listing; chamadas indiretas, bytes nao analisados e tabelas runtime
        // continuam exigindo revisao manual no projeto.
        writer.println("relative_search\t" + target + "\t" + symbolName(target)
                + "\t\t" + target + "\trel32_scan\t\thits=" + hits
                + ";candidates=" + candidates + ";instructions=" + decoded
                + ";errors=" + errors);
    }

    private void writeRawRelativeHits(PrintWriter writer, Address target,
            FunctionManager functions, Listing listing, Memory memory) {
        // Este segundo passe cobre bytes em blocos executaveis que ainda nao
        // viraram instrucoes na listing. Um hit bruto precisa de classificacao
        // manual; zero hits, porem, fecha a lacuna especifica de E8/E9 rel32
        // escondido em regiao executavel nao analisada.
        long scannedBytes = 0;
        int executableBlocks = 0;
        int candidates = 0;
        int errors = 0;
        int hits = 0;
        for (MemoryBlock block : memory.getBlocks()) {
            if (!block.isExecute() || !block.isInitialized() || block.getSize() < 5) {
                continue;
            }
            executableBlocks++;
            Address cursor = block.getStart();
            Address lastStart = block.getEnd().subtract(4);
            while (cursor.compareTo(lastStart) <= 0 && !monitor.isCancelled()) {
                scannedBytes++;
                int opcode;
                long displacement;
                try {
                    opcode = Byte.toUnsignedInt(memory.getByte(cursor));
                    if (opcode != 0xE8 && opcode != 0xE9) {
                        cursor = cursor.add(1);
                        continue;
                    }
                    candidates++;
                    displacement = readSignedRel32(cursor, memory);
                }
                catch (MemoryAccessException exception) {
                    errors++;
                    cursor = cursor.add(1);
                    continue;
                }

                Address next = cursor.add(5);
                long resolvedOffset = (next.getOffset() + displacement) & 0xffffffffL;
                Address resolved = next.getAddressSpace().getAddress(resolvedOffset);
                if (resolved.equals(target)) {
                    hits++;
                    Instruction instruction = listing.getInstructionAt(cursor);
                    Data data = listing.getDefinedDataContaining(cursor);
                    Function owner = functions.getFunctionContaining(cursor);
                    String kind = instruction != null ? "instruction"
                            : data != null ? "data" : "undefined";
                    String ownerName = owner == null ? symbolName(cursor) : owner.getName();
                    writer.println("raw_relative_hit\t" + target + "\t"
                            + symbolName(target) + "\t" + cursor + "\t" + resolved
                            + "\t" + (opcode == 0xE8 ? "CALL_REL32_RAW" : "JMP_REL32_RAW")
                            + "\t" + ownerName + "\tclassification=" + kind
                            + ";block=" + block.getName() + ";next=" + next
                            + ";displacement=" + displacement);
                }
                cursor = cursor.add(1);
            }
        }
        writer.println("raw_relative_search\t" + target + "\t" + symbolName(target)
                + "\t\t" + target + "\traw_rel32_scan\t\thits=" + hits
                + ";candidates=" + candidates + ";bytes=" + scannedBytes
                + ";executable_blocks=" + executableBlocks + ";errors=" + errors);
    }

    private String switchTableContext(Address source, Set<Address> dataSources,
            Listing listing) {
        // O analisador de switches nomeia a base, mas as entradas seguintes da
        // mesma tabela normalmente ficam sem simbolo. Agrupar somente dwords
        // contiguos que tambem referenciam labels do corpo permite propagar a
        // classificacao sem chamar qualquer ponteiro de dados de jump table.
        Address start = source;
        while (true) {
            // addNoWrap impede que uma tabela na borda do address space se una
            // artificialmente a dados do extremo oposto por wrap de endereco.
            Address previous = offsetNoWrap(start, -4);
            if (previous == null || !dataSources.contains(previous)) {
                break;
            }
            start = previous;
        }

        Address end = start;
        int entries = 0;
        boolean namedSwitchTable = false;
        while (dataSources.contains(end)) {
            entries++;
            String symbol = symbolName(end).toLowerCase();
            Data data = listing.getDefinedDataContaining(end);
            String dataSymbol = data == null
                    ? "" : symbolName(data.getMinAddress()).toLowerCase();
            if (symbol.startsWith("switchdata") || dataSymbol.startsWith("switchdata")) {
                namedSwitchTable = true;
            }
            Address next = offsetNoWrap(end, 4);
            if (next == null || !dataSources.contains(next)) {
                break;
            }
            end = next;
        }

        if (!namedSwitchTable) {
            return null;
        }
        return "table_start=" + start + ";table_end=" + end
                + ";table_entries=" + entries;
    }

    private Address offsetNoWrap(Address address, long displacement) {
        try {
            return address.addNoWrap(displacement);
        }
        catch (AddressOverflowException exception) {
            // Limite de address space encerra a busca contigua; nao e erro do
            // export nem autoriza continuar a varredura com endereco enrolado.
            return null;
        }
    }

    private void writeBodyIncoming(PrintWriter writer, Address entry,
            FunctionManager functions, Listing listing, ReferenceManager references,
            Memory memory) {
        Function function = functions.getFunctionAt(entry);
        if (function == null) {
            writer.println("body_search\t" + entry + "\t" + symbolName(entry)
                    + "\t\t" + entry + "\tbody_refs\t\tfunction=missing");
            return;
        }

        // Um xref de fluxo para um bloco interno pode alcancar a rotina sem
        // apontar para seu entry point. Xrefs DATA de jump tables, por outro
        // lado, descrevem o CFG interno e nao sao callers. Colete primeiro para
        // que as tabelas possam ser agrupadas e classificadas separadamente.
        int instructions = 0;
        List<BodyReferenceHit> externalReferences = new ArrayList<>();
        Set<Address> externalDataSources = new TreeSet<>();
        InstructionIterator bodyInstructions = listing.getInstructions(function.getBody(), true);
        while (bodyInstructions.hasNext() && !monitor.isCancelled()) {
            Instruction instruction = bodyInstructions.next();
            instructions++;
            ReferenceIterator incoming = references.getReferencesTo(instruction.getAddress());
            while (incoming.hasNext() && !monitor.isCancelled()) {
                Reference reference = incoming.next();
                Address from = reference.getFromAddress();
                if (function.getBody().contains(from)) {
                    continue;
                }
                externalReferences.add(new BodyReferenceHit(
                        reference, instruction.getAddress()));
                if (reference.getReferenceType().isData()) {
                    externalDataSources.add(from);
                }
            }
        }

        int externalFlowHits = 0;
        int externalDataHits = 0;
        int switchTableHits = 0;
        int externalOtherHits = 0;
        for (BodyReferenceHit hit : externalReferences) {
            Reference reference = hit.reference;
            Address from = reference.getFromAddress();
            Function owner = functions.getFunctionContaining(from);
            String source = owner == null ? symbolName(from) : owner.getName();
            String record;
            String context = "owner_entry="
                    + (owner == null ? "" : owner.getEntryPoint());
            if (reference.getReferenceType().isFlow()) {
                externalFlowHits++;
                record = "body_flow_incoming";
            }
            else if (reference.getReferenceType().isData()) {
                externalDataHits++;
                String table = switchTableContext(from, externalDataSources, listing);
                if (table == null) {
                    record = "body_data_incoming";
                }
                else {
                    switchTableHits++;
                    record = "body_switch_table";
                    context += ";" + table;
                }
            }
            else {
                externalOtherHits++;
                record = "body_other_incoming";
            }
            MemoryBlock sourceBlock = memory.getBlock(from);
            context += ";source_block="
                    + (sourceBlock == null ? "" : sourceBlock.getName());
            writer.println(record + "\t" + entry + "\t" + function.getName()
                    + "\t" + from + "\t" + hit.target + "\t"
                    + reference.getReferenceType() + "\t" + source + "\t" + context);
        }

        // Um thunk reconhecido pelo Ghidra e uma entrada estatica mesmo quando
        // o xref original aponta ao wrapper. Registre-o explicitamente para que
        // "sem caller" nao esconda essa forma de alcance.
        int thunkHits = 0;
        FunctionIterator allFunctions = functions.getFunctions(true);
        while (allFunctions.hasNext() && !monitor.isCancelled()) {
            Function candidate = allFunctions.next();
            if (!candidate.isThunk()) {
                continue;
            }
            Function thunked = candidate.getThunkedFunction(true);
            if (thunked == null || !thunked.getEntryPoint().equals(entry)) {
                continue;
            }
            thunkHits++;
            writer.println("body_thunk\t" + entry + "\t" + function.getName()
                    + "\t" + candidate.getEntryPoint() + "\t" + entry
                    + "\tTHUNK\t" + candidate.getName() + "\t");
        }

        Instruction previous = listing.getInstructionBefore(entry);
        Address previousFallThrough = previous == null ? null : previous.getFallThrough();
        writer.println("entry_boundary\t" + entry + "\t" + function.getName() + "\t"
                + (previous == null ? "" : previous.getAddress()) + "\t" + entry
                + "\t" + (previous == null ? "" : previous.getFlowType()) + "\t"
                + (previous == null ? "" : safeValue(previous)) + "\tfallthrough="
                + (previousFallThrough == null ? "" : previousFallThrough));

        MemoryBlock block = memory.getBlock(entry);
        String blockValue = block == null ? ""
                : "block=" + block.getName() + ";execute=" + block.isExecute()
                + ";initialized=" + block.isInitialized();
        writer.println("body_search\t" + entry + "\t" + function.getName()
                + "\t\t" + entry + "\tbody_refs\t\t" + "external_flow_hits="
                + externalFlowHits + ";external_data_hits=" + externalDataHits
                + ";switch_table_hits=" + switchTableHits + ";external_other_hits="
                + externalOtherHits + ";thunk_hits=" + thunkHits
                + ";instructions=" + instructions + ";body_min="
                + function.getBody().getMinAddress() + ";body_max="
                + function.getBody().getMaxAddress() + ";" + blockValue);
    }

    private void writeDataWindow(PrintWriter writer, Address requested,
            ReferenceManager references, FunctionManager functions, Memory memory) {
        // Vtable slots are usually the only xref to a virtual handler. The bounded
        // window exposes adjacent slots and the constructor reference to the table
        // without pretending that a raw pointer proves the owning class by itself.
        Address start = requested.subtract(DATA_WINDOW_BYTES);
        Address end = requested.add(DATA_WINDOW_BYTES);
        for (Address cursor = start; cursor.compareTo(end) <= 0 && !monitor.isCancelled();
                cursor = cursor.add(4)) {
            if (!memory.contains(cursor)) {
                continue;
            }
            long raw;
            try {
                raw = Integer.toUnsignedLong(memory.getInt(cursor));
            }
            catch (MemoryAccessException exception) {
                writer.println("nearby_error\t" + requested + "\t\t" + cursor
                        + "\t\tread\t\t" + safeValue(exception.getMessage()));
                continue;
            }

            Address target = currentProgram.getAddressFactory().getDefaultAddressSpace()
                    .getAddress(raw);
            Function targetFunction = functions.getFunctionAt(target);
            String targetName = targetFunction != null
                    ? targetFunction.getName() : symbolName(target);
            writer.println("nearby\t" + requested + "\t" + symbolName(requested) + "\t"
                    + cursor + "\t" + target + "\tpointer\t" + targetName + "\t0x"
                    + String.format("%08X", raw));

            // Nao expanda incoming xrefs de cada dword vizinho. O endereco
            // solicitado ja recebe seus incoming acima; passe slots adicionais
            // como argumentos quando eles fizerem parte do contrato investigado.
            writeOutgoing(writer, cursor, references);
        }
    }

    private void writeInstructions(PrintWriter writer, Address requested,
            FunctionManager functions, Listing listing) {
        Function function = functions.getFunctionAt(requested);
        if (function == null) {
            function = functions.getFunctionContaining(requested);
        }
        if (function == null) {
            writer.println("instruction_search\t" + requested + "\t"
                    + symbolName(requested) + "\t\t" + requested
                    + "\tinstructions\t\tfunction=missing");
            return;
        }

        int instructionCount = 0;
        int referenceCount = 0;
        InstructionIterator instructions = listing.getInstructions(function.getBody(), true);
        while (instructions.hasNext() && !monitor.isCancelled()) {
            Instruction instruction = instructions.next();
            instructionCount++;
            Address address = instruction.getAddress();
            Address fallThrough = instruction.getFallThrough();
            Address[] flows = instruction.getFlows();
            writer.println("instruction\t" + requested + "\t" + function.getName()
                    + "\t" + address + "\t"
                    + (fallThrough == null ? "" : fallThrough) + "\t"
                    + instruction.getFlowType() + "\t" + function.getName()
                    + "\tbytes=" + bytesHex(instruction)
                    + ";mnemonic=" + safeValue(instruction.getMnemonicString())
                    + ";instruction=" + safeValue(instruction)
                    + ";inputs=" + objectList(instruction.getInputObjects())
                    + ";results=" + objectList(instruction.getResultObjects())
                    + ";flows=" + addressList(flows)
                    + ";fallthrough=" + (fallThrough == null ? "" : fallThrough));

            // Referencias ficam em linhas proprias para que calls diretos, dados
            // e slots possam ser filtrados sem parsear o texto da instrucao.
            for (Reference reference : instruction.getReferencesFrom()) {
                referenceCount++;
                Address target = reference.getToAddress();
                writer.println("instruction_reference\t" + requested + "\t"
                        + function.getName() + "\t" + address + "\t" + target
                        + "\t" + reference.getReferenceType() + "\t"
                        + symbolName(target) + "\toperand_index="
                        + reference.getOperandIndex() + ";primary="
                        + reference.isPrimary() + ";source=" + reference.getSource());
            }
        }
        writer.println("instruction_search\t" + requested + "\t" + function.getName()
                + "\t" + function.getEntryPoint() + "\t" + requested
                + "\tinstructions\t" + function.getName() + "\tinstructions="
                + instructionCount + ";references=" + referenceCount
                + ";body_min=" + function.getBody().getMinAddress()
                + ";body_max=" + function.getBody().getMaxAddress());
    }

    private long parseVirtualSlotOffset(String raw) {
        String normalized = raw;
        if (normalized.startsWith("0x") || normalized.startsWith("0X")) {
            normalized = normalized.substring(2);
        }
        if (normalized.isEmpty() || normalized.startsWith("-")
                || normalized.startsWith("+")) {
            throw new IllegalArgumentException("offset de slot virtual invalido: " + raw);
        }

        long offset;
        try {
            // Slots sao enderecados em hexadecimal nos exports e na listing;
            // aceitar 54 e 0x54 evita que a mesma prova dependa de notacao.
            offset = Long.parseUnsignedLong(normalized, 16);
        }
        catch (NumberFormatException exception) {
            throw new IllegalArgumentException(
                    "offset de slot virtual invalido: " + raw, exception);
        }
        if (offset > 0xffffffffL) {
            throw new IllegalArgumentException(
                    "offset de slot virtual fora do limite de 32 bits: " + raw);
        }
        return offset;
    }

    private void writeVirtualSlotHits(PrintWriter writer, long slotOffset,
            FunctionManager functions, Listing listing) {
        // CALL [reg+offset] prova somente o uso estrutural daquele slot. Classes
        // distintas podem compartilhar o mesmo offset; o tipo do receptor deve
        // ser fechado pelo fluxo de dados, vptr e lifecycle no projeto Ghidra.
        InstructionIterator instructions = listing.getInstructions(true);
        long instructionCount = 0;
        long candidateCount = 0;
        long hitCount = 0;
        String requestedSlot = "0x" + Long.toUnsignedString(slotOffset, 16);

        while (instructions.hasNext() && !monitor.isCancelled()) {
            Instruction instruction = instructions.next();
            instructionCount++;
            if (!instruction.getFlowType().isCall()
                    || !instruction.getFlowType().isComputed()) {
                continue;
            }
            candidateCount++;

            int matchingOperand = -1;
            Object[] matchingObjects = null;
            for (int operandIndex = 0;
                    operandIndex < instruction.getNumOperands(); operandIndex++) {
                Object[] operandObjects = instruction.getOpObjects(operandIndex);
                boolean hasRegister = false;
                boolean hasRequestedScalar = false;
                for (Object object : operandObjects) {
                    if (object instanceof Register) {
                        hasRegister = true;
                    }
                    else if (object instanceof Scalar
                            && ((Scalar) object).getUnsignedValue() == slotOffset) {
                        hasRequestedScalar = true;
                    }
                }
                if (hasRegister && hasRequestedScalar) {
                    matchingOperand = operandIndex;
                    matchingObjects = operandObjects;
                    break;
                }
            }
            if (matchingOperand < 0) {
                continue;
            }

            hitCount++;
            Address address = instruction.getAddress();
            Address fallThrough = instruction.getFallThrough();
            Function owner = functions.getFunctionContaining(address);
            String ownerName = owner == null ? symbolName(address) : owner.getName();
            String ownerEntry = owner == null ? "" : owner.getEntryPoint().toString();
            writer.println("virtual_slot_hit\t" + requestedSlot + "\t" + ownerName
                    + "\t" + address + "\t"
                    + (fallThrough == null ? "" : fallThrough) + "\t"
                    + instruction.getFlowType() + "\t" + ownerName
                    + "\towner_entry=" + ownerEntry
                    + ";bytes=" + bytesHex(instruction)
                    + ";mnemonic=" + safeValue(instruction.getMnemonicString())
                    + ";instruction=" + safeValue(instruction)
                    + ";operand_index=" + matchingOperand
                    + ";operand=" + safeValue(
                            instruction.getDefaultOperandRepresentation(matchingOperand))
                    + ";objects=" + objectList(matchingObjects)
                    + ";registers=" + registerList(matchingObjects)
                    + ";scalars=" + scalarList(matchingObjects)
                    + ";slot=" + requestedSlot
                    + ";inputs=" + objectList(instruction.getInputObjects())
                    + ";results=" + objectList(instruction.getResultObjects())
                    + ";flows=" + addressList(instruction.getFlows())
                    + ";fallthrough=" + (fallThrough == null ? "" : fallThrough));
        }

        // O resumo permanece presente com zero hits. Isso torna a busca
        // reproduzivel sem converter ausencia estrutural em prova de classe.
        writer.println("virtual_slot_search\t" + requestedSlot
                + "\t\t\t\tcomputed_call\t\thits=" + hitCount
                + ";candidates=" + candidateCount
                + ";instructions=" + instructionCount);
    }

    private int parseTableSlotCount(String raw) {
        int count;
        try {
            count = Integer.decode(raw);
        }
        catch (NumberFormatException exception) {
            throw new IllegalArgumentException("quantidade de slots invalida: " + raw,
                    exception);
        }
        // A tabela e sempre solicitada de forma focada. O limite evita que um
        // argumento incorreto transforme o relatorio auxiliar em dump de memoria.
        if (count <= 0 || count > MAX_TABLE_SLOTS) {
            throw new IllegalArgumentException("quantidade de slots fora do limite 1.."
                    + MAX_TABLE_SLOTS + ": " + raw);
        }
        return count;
    }

    private void writeTable(PrintWriter writer, Address base, int slotCount,
            ReferenceManager references, FunctionManager functions, Listing listing,
            Memory memory) {
        int slotsRead = 0;
        int errors = 0;
        for (int index = 0; index < slotCount && !monitor.isCancelled(); index++) {
            long byteOffset = (long) index * TABLE_SLOT_BYTES;
            Address slot = offsetNoWrap(base, byteOffset);
            if (slot == null || !memory.contains(slot)) {
                errors++;
                writer.println("table_slot_error\t" + base + "\t" + symbolName(base)
                        + "\t" + (slot == null ? "" : slot)
                        + "\t\tread\t\tindex=" + index + ";offset=0x"
                        + String.format("%X", byteOffset) + ";reason=outside_memory");
                continue;
            }

            long raw;
            try {
                raw = Integer.toUnsignedLong(memory.getInt(slot));
            }
            catch (MemoryAccessException exception) {
                errors++;
                writer.println("table_slot_error\t" + base + "\t" + symbolName(base)
                        + "\t" + slot + "\t\tread\t\tindex=" + index
                        + ";offset=0x" + String.format("%X", byteOffset)
                        + ";reason=" + safeValue(exception.getMessage()));
                continue;
            }

            slotsRead++;
            Address target = currentProgram.getAddressFactory().getDefaultAddressSpace()
                    .getAddress(raw);
            Function targetFunction = functions.getFunctionAt(target);
            String targetName = targetFunction == null
                    ? symbolName(target) : targetFunction.getName();
            Data data = listing.getDefinedDataContaining(slot);
            String slotContext = "index=" + index + ";offset=0x"
                    + String.format("%X", byteOffset) + ";raw=0x"
                    + String.format("%08X", raw) + ";data="
                    + safeValue(data == null ? null : data.getValue());
            writer.println("table_slot\t" + base + "\t" + symbolName(base) + "\t"
                    + slot + "\t" + target + "\tpointer\t" + targetName + "\t"
                    + slotContext);

            ReferenceIterator incoming = references.getReferencesTo(slot);
            while (incoming.hasNext() && !monitor.isCancelled()) {
                Reference reference = incoming.next();
                Address from = reference.getFromAddress();
                Function owner = functions.getFunctionContaining(from);
                String source = owner == null ? symbolName(from) : owner.getName();
                writer.println("table_slot_incoming\t" + base + "\t"
                        + symbolName(base) + "\t" + from + "\t" + slot + "\t"
                        + reference.getReferenceType() + "\t" + source + "\t"
                        + slotContext + ";owner_entry="
                        + (owner == null ? "" : owner.getEntryPoint()));
            }
            for (Reference reference : references.getReferencesFrom(slot)) {
                Address referenceTarget = reference.getToAddress();
                writer.println("table_slot_outgoing\t" + base + "\t"
                        + symbolName(base) + "\t" + slot + "\t" + referenceTarget
                        + "\t" + reference.getReferenceType() + "\t"
                        + symbolName(referenceTarget) + "\t" + slotContext);
            }
        }
        writer.println("table_search\t" + base + "\t" + symbolName(base) + "\t"
                + base + "\t\ttable\t\trequested_slots=" + slotCount
                + ";slots_read=" + slotsRead + ";errors=" + errors
                + ";slot_bytes=" + TABLE_SLOT_BYTES);
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 2) {
            throw new IllegalArgumentException(
                    "uso: <arquivo-saida.tsv> "
                    + "<entry|exact:entry|pointers:entry|relatives:entry|rawrelatives:entry|"
                    + "bodyrefs:entry|instructions:entry|virtualslot:offset|"
                    + "table:base:slot-count> [entry ...]");
        }

        File output = new File(args[0]);
        FunctionManager functions = currentProgram.getFunctionManager();
        Listing listing = currentProgram.getListing();
        ReferenceManager references = currentProgram.getReferenceManager();
        Memory memory = currentProgram.getMemory();

        try (PrintWriter writer = new PrintWriter(new OutputStreamWriter(
                new FileOutputStream(output), StandardCharsets.UTF_8))) {
            writer.println("record\tentry\tname\tfrom\tto\ttype\tsymbol\tvalue");
            writer.println("program\t\t\t\t\tsha256\t\t" + currentProgram.getExecutableSHA256());
            writer.println("program\t\t\t\t\timage_base\t\t" + currentProgram.getImageBase());

            for (int i = 1; i < args.length && !monitor.isCancelled(); i++) {
                String argument = args[i];
                if (argument.startsWith("virtualslot:")) {
                    String rawOffset = argument.substring("virtualslot:".length());
                    writeVirtualSlotHits(writer, parseVirtualSlotOffset(rawOffset),
                            functions, listing);
                    continue;
                }
                if (argument.startsWith("table:")) {
                    String[] tableParts = argument.split(":", -1);
                    if (tableParts.length != 3 || tableParts[1].isEmpty()
                            || tableParts[2].isEmpty()) {
                        throw new IllegalArgumentException(
                                "uso do modo table: table:<base>:<slot-count>");
                    }
                    Address base = currentProgram.getAddressFactory()
                            .getDefaultAddressSpace().getAddress(tableParts[1]);
                    writeTable(writer, base, parseTableSlotCount(tableParts[2]),
                            references, functions, listing, memory);
                    continue;
                }
                boolean pointerSearch = args[i].startsWith("pointers:");
                boolean relativeSearch = args[i].startsWith("relatives:");
                boolean rawRelativeSearch = args[i].startsWith("rawrelatives:");
                boolean bodyReferenceSearch = args[i].startsWith("bodyrefs:");
                boolean instructionSearch = args[i].startsWith("instructions:");
                // O prefixo exact preserva somente o endereco decisivo. Ele evita
                // transformar o contexto vizinho de uma vtable em prova de xref.
                boolean exact = args[i].startsWith("exact:");
                String rawEntry = pointerSearch
                        ? args[i].substring("pointers:".length())
                        : relativeSearch ? args[i].substring("relatives:".length())
                        : rawRelativeSearch ? args[i].substring("rawrelatives:".length())
                        : bodyReferenceSearch ? args[i].substring("bodyrefs:".length())
                        : instructionSearch ? args[i].substring("instructions:".length())
                        : exact ? args[i].substring("exact:".length()) : args[i];
                Address entry = currentProgram.getAddressFactory()
                        .getDefaultAddressSpace().getAddress(rawEntry);
                if (pointerSearch) {
                    writePointerHits(writer, entry, references, functions, memory);
                    continue;
                }
                if (relativeSearch) {
                    writeRelativeHits(writer, entry, functions, listing, memory);
                    continue;
                }
                if (rawRelativeSearch) {
                    writeRawRelativeHits(writer, entry, functions, listing, memory);
                    continue;
                }
                if (bodyReferenceSearch) {
                    writeBodyIncoming(writer, entry, functions, listing, references, memory);
                    continue;
                }
                if (instructionSearch) {
                    writeInstructions(writer, entry, functions, listing);
                    continue;
                }
                Function function = functions.getFunctionAt(entry);
                if (function == null) {
                    Function owner = functions.getFunctionContaining(entry);
                    Instruction instruction = listing.getInstructionContaining(entry);
                    Data data = listing.getDefinedDataContaining(entry);
                    String kind = instruction != null ? "instruction"
                            : data != null ? "data" : memory.contains(entry) ? "memory" : "missing";
                    writer.println("address\t" + entry + "\t" + symbolName(entry) + "\t"
                            + (owner == null ? "" : owner.getEntryPoint()) + "\t\t" + kind
                            + "\t" + (owner == null ? "" : owner.getName()) + "\t"
                            + safeValue(data == null ? null : data.getValue()));
                    writeIncoming(writer, "incoming", entry, symbolName(entry),
                            references, functions);
                    if (exact) {
                        // Um slot exato precisa ser reproduzivel sem depender de
                        // outra janela que por acaso inclua o mesmo endereco.
                        writeOutgoing(writer, entry, references);
                    }
                    else if (memory.contains(entry)) {
                        writeDataWindow(writer, entry, references, functions, memory);
                    }
                    continue;
                }
                writer.println("function\t" + describeFunction(function)
                        + "\t\t\t" + function.getCallingConventionName() + "\t\t"
                        + function.getSignature());

                // Incoming data xrefs are essential for callbacks and dispatch tables,
                // where getCallingFunctions() legitimately reports no direct caller.
                writeIncoming(writer, "incoming", entry, function.getName(),
                        references, functions);

                Set<Function> callers = new TreeSet<>((a, b) ->
                        a.getEntryPoint().compareTo(b.getEntryPoint()));
                callers.addAll(function.getCallingFunctions(monitor));
                for (Function caller : callers) {
                    writer.println("caller\t" + describeFunction(function) + "\t"
                            + caller.getEntryPoint() + "\t" + entry + "\tcall\t"
                            + caller.getName() + "\t");
                }

                Set<Function> callees = new TreeSet<>((a, b) ->
                        a.getEntryPoint().compareTo(b.getEntryPoint()));
                callees.addAll(function.getCalledFunctions(monitor));
                for (Function callee : callees) {
                    writer.println("callee\t" + describeFunction(function) + "\t"
                            + entry + "\t" + callee.getEntryPoint() + "\tcall\t"
                            + callee.getName() + "\t");
                }

                InstructionIterator instructions = listing.getInstructions(function.getBody(), true);
                while (instructions.hasNext() && !monitor.isCancelled()) {
                    Instruction instruction = instructions.next();
                    for (Reference reference : instruction.getReferencesFrom()) {
                        Address target = reference.getToAddress();
                        // Callers/callees ja representam o fluxo. O relatorio de
                        // referencias fica restrito a dados, globals e strings em
                        // memoria para nao esconder side effects em ruido de CFG.
                        if (reference.getReferenceType().isFlow() || !memory.contains(target)) {
                            continue;
                        }
                        Symbol symbol = currentProgram.getSymbolTable().getPrimarySymbol(target);
                        Data data = listing.getDefinedDataAt(target);
                        Object value = data == null ? null : data.getValue();
                        writer.println("reference\t" + describeFunction(function) + "\t"
                                + instruction.getAddress() + "\t" + target + "\t"
                                + reference.getReferenceType() + "\t"
                                + (symbol == null ? "" : symbol.getName()) + "\t"
                                + safeValue(value));
                    }
                }
            }
        }
        println("Fluxo exportado para " + output.getAbsolutePath());
    }
}
