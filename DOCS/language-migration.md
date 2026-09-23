# English-language migration

The project targets the global, English-language WYD client. English is the
only language for new or edited human-readable content. The repository is not
yet fully migrated; this document tracks the remaining work without treating
a partial translation as completion.

## Completed entry points

- Root repository rules and all active skill entry points.
- Root README and documentation README.
- Generated documentation inventory labels and layout-validator messages.
- `tmproject/Build-Client.ps1` help, diagnostics, and validation status.
- The staff portal's static and dynamic UI text, including forms, event controls,
  moderation, teleportation, status messages, and English date/number formatting.
- The in-source 7.48 client CC-mode control labels, the item position,
  and the selected server endpoint diagnostic.
- The runtime `tmproject/client748/sn.bin` group labels: `Channel` and
  `Featured` replace the Portuguese and Korean labels while retaining all
  eleven fixed-width slots, group orders, and the 143-byte file layout.

## Remaining work

1. Audit client-visible strings, resource text, and error paths in
   `tmproject/TMProject748/` and `tmproject/client748/`. Translate prose without
   changing resource IDs, binary formats, or protocol payloads. Validate
   affected UI flows in the actual 7.48 client when client testing is possible.
2. Audit remaining server-facing and administrator-facing messages in
   `wydgo748/`, then update corresponding tests and API documentation together.
3. Translate active guides in `DOCS/`, including their headings, examples, and
   cross-links. Keep established paths stable unless every reference is updated.
4. Translate scripts, diagnostics, and test descriptions in `tools/`,
   `tmproject/`, and `wydgo748/` as each affected workflow is validated.
5. Translate `.agents/research/` and `.agents/handoffs/` where they are still
   used for current decisions; preserve technical evidence and provenance.

Code identifiers, persisted values, packet bytes, resource keys, and fixture
data require compatibility review before renaming. English prose is the goal;
blind replacement of contract-bearing values is not.

Completion requires a repository-wide review of authored text plus validation
of affected client and server flows. A keyword search alone cannot distinguish
Portuguese prose from names, examples, and protocol data.
