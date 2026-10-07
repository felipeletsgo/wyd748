// Exports the defined strings of the WYD 7.48 program that look like file
// paths or path formats (read-only), with the functions that reference them.
// A string is kept when it has a file extension, a directory separator or a
// format that builds a file name. The list is evidence of what the native
// client may open; the loader that consumes each path is confirmed per file.
//
// Arguments: <paths.tsv>
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.DataIterator;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;
import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.util.Set;
import java.util.TreeSet;
import java.util.regex.Pattern;

public class ExportWydPathStrings extends GhidraScript {
    private static final Pattern PATH = Pattern.compile(
            "(?i).*(\\.(txt|bin|dat|csv|ini|msh|wys|msa|wyt|trn|ani|bon|bmp|tga|jpg|dds|wav|mp3|ogg|mid|fx|psh|vsh|cfg|log|json|exe|dll)\\b|[\\\\/]).*");

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 1) {
            throw new IllegalArgumentException("usage: <paths.tsv>");
        }
        int kept = 0;
        try (PrintWriter writer = new PrintWriter(new OutputStreamWriter(
                new FileOutputStream(new File(args[0])), StandardCharsets.UTF_8))) {
            writer.println("address\tstring\tfunctions");
            writer.println("#program\t" + currentProgram.getExecutableSHA256() + "\t");
            DataIterator data = currentProgram.getListing().getDefinedData(true);
            while (data.hasNext() && !monitor.isCancelled()) {
                Data item = data.next();
                if (!item.hasStringValue()) {
                    continue;
                }
                Object value = item.getValue();
                if (value == null) {
                    continue;
                }
                String text = value.toString();
                if (text.length() < 4 || text.length() > 200 || !PATH.matcher(text).matches()) {
                    continue;
                }
                Set<String> functions = new TreeSet<>();
                for (Reference reference : getReferencesTo(item.getAddress())) {
                    Function function = getFunctionContaining(reference.getFromAddress());
                    functions.add(function == null ? reference.getFromAddress().toString() : function.getName());
                }
                writer.println(item.getAddress() + "\t" + text.replace("\t", "\\t").replace("\r", "\\r").replace("\n", "\\n")
                        + "\t" + String.join(" ", functions));
                kept++;
            }
        }
        println("path_strings=" + kept);
    }
}
