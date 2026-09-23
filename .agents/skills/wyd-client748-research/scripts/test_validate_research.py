#!/usr/bin/env python3
"""Tests for the 7.48 research-record structure gate."""

from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from validate_research import LIFECYCLE_SUBSECTIONS, SECTION_ALIASES, validate


NATIVE_HASH = "8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593"


def research_record(status: str = "TRACED", lifecycle: bool = True) -> str:
    subsystem = "lifecycle" if lifecycle else "transport"
    record_id = "scene-transition-lifecycle" if lifecycle else "packet-dispatch-flow"
    record_title = "Scene transition lifecycle" if lifecycle else "Packet dispatch"
    lifecycle_sections = ""
    if lifecycle:
        lifecycle_sections = """
### Transition matrix

| Event/state | Precondition | Function/call | Resulting state | Side effects | Error/exit |
| --- | --- | --- | --- | --- | --- |
| state 7 | active scene | FUN_004B37C9 | state 5 | marks node | cleanup on next tick |

### Vtables, vptrs, and receivers

The receiver resolves to vptr `0x005A45FC`, with the relevant slot at `+0x54`.

### Ownership

The manager owns the tree; the scene remains owned by its node until cleanup.

### Partial failure

Initialization destroys the newly allocated scene before returning failure.

### Cleanup and teardown

`FUN_004B16C0` removes and destroys marked nodes on the next tick.

### Shutdown

N/A: this transition does not exit the process and uses manager teardown.

### Logout and relogin

N/A: this flow ends before login; the boundary excludes relogin.
"""
    real_client = (
        "executed in tmproject/client748/project.exe with a recorded artifact hash"
        if status == "CLIENT_TESTED"
        else "not run"
    )
    return f"""---
id: {record_id}
title: {record_title}
subsystem: {subsystem}
status: {status}
native_sha256: {NATIVE_HASH}
updated: 2026-09-23
---

# {record_title}

## Question

What observable transition occurs?

## Evidence boundary

- Executable/hash: verified

## Native 7.48 flow

### Observable entry

The state-7 request enters through `FUN_004B3500` and publishes the new scene.

### Callers

`FUN_004B37C9` is the confirmed caller.

### Main function

`FUN_004B3500` performs the transition.

### Callees

`FUN_0054AC09` attaches the scene to the tree.

### Outputs and errors

Success attaches the scene; failure destroys the partial allocation.

## State and lifecycle
{lifecycle_sections}

## Wire, ABI, and resources

N/A: this local transition has no wire contract.

## Current mapping

### Buildable source

ObjectManager is the adaptation candidate.

### WYD-Go

N/A: the scene transition is local to the client.

## Delta matrix

| Claim | Native 7.48 | Current source | TMProject | WYD-Go | Decision |
| --- | --- | --- | --- | --- | --- |
| transition | confirmed | divergent | hint | N/A | adapt |

## Decisions

Adapt only the confirmed claim.

## Gaps

None within this bounded transition.

## Validation

- Research: Ghidra inspected
- Automation: validator executed
- Real client: {real_client}
"""


class ValidateResearchTests(unittest.TestCase):
    def validate_text(self, text: str) -> list[str]:
        # Exercise the same UTF-8 path used by the real CLI.
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "flow.md"
            path.write_text(text, encoding="utf-8")
            errors, _ = validate(path)
            return errors

    def test_mature_lifecycle_record_passes(self) -> None:
        for status in ("TRACED", "CONTRACT", "CLIENT_TESTED"):
            with self.subTest(status=status):
                self.assertEqual([], self.validate_text(research_record(status)))

    def test_each_lifecycle_field_is_required(self) -> None:
        complete = research_record()
        for heading in LIFECYCLE_SUBSECTIONS:
            with self.subTest(heading=heading):
                start = complete.index(f"### {heading}")
                next_heading = complete.find("\n### ", start + 4)
                next_section = complete.find("\n## ", start + 4)
                candidates = [index for index in (next_heading, next_section) if index >= 0]
                end = min(candidates) if candidates else len(complete)
                broken = complete[:start] + f"### {heading}\n\nUNRESOLVED\n" + complete[end:]
                errors = self.validate_text(broken)
                self.assertTrue(
                    any(heading in error for error in errors),
                    msg=f"missing gate for {heading}: {errors}",
                )

    def test_transition_matrix_requires_a_data_row(self) -> None:
        text = research_record().replace(
            "| state 7 | active scene | FUN_004B37C9 | state 5 | marks node | cleanup on next tick |\n",
            "",
        )
        errors = self.validate_text(text)
        self.assertTrue(any("Transition matrix" in error for error in errors))

    def test_na_requires_a_justification(self) -> None:
        text = research_record().replace(
            "N/A: this transition does not exit the process and uses manager teardown.",
            "N/A",
        )
        errors = self.validate_text(text)
        self.assertTrue(any("Shutdown" in error for error in errors))

    def test_unresolved_with_explanation_does_not_close_gate(self) -> None:
        text = research_record().replace(
            "The manager owns the tree; the scene remains owned by its node until cleanup.",
            "UNRESOLVED: the owner still needs investigation.",
        )
        errors = self.validate_text(text)
        self.assertTrue(any("Ownership" in error for error in errors))

    def test_located_lifecycle_may_remain_incomplete(self) -> None:
        text = research_record("LOCATED").replace(
            "The receiver resolves to vptr `0x005A45FC`, with the relevant slot at `+0x54`.",
            "UNRESOLVED",
        )
        self.assertEqual([], self.validate_text(text))

    def test_non_lifecycle_traced_uses_general_gate(self) -> None:
        self.assertEqual([], self.validate_text(research_record(lifecycle=False)))

    def test_existing_portuguese_headings_remain_readable(self) -> None:
        text = research_record()
        for english, legacy in SECTION_ALIASES.items():
            text = text.replace(f"## {english}\n", f"## {legacy}\n")
            text = text.replace(f"### {english}\n", f"### {legacy}\n")
        self.assertEqual([], self.validate_text(text))

    def test_pending_real_client_cannot_be_promoted(self) -> None:
        text = research_record("CLIENT_TESTED").replace(
            "Real client: executed in tmproject/client748/project.exe with a recorded artifact hash",
            "Real client: not run",
        )
        self.assertTrue(any("real-client" in error for error in self.validate_text(text)))


if __name__ == "__main__":
    unittest.main()
