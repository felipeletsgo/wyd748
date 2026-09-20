from __future__ import annotations

import csv
import io
import tempfile
import unittest
from pathlib import Path

import source_inventory


PROJECT_TEMPLATE = """<?xml version="1.0" encoding="utf-8"?>
<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
    {items}
  </ItemGroup>
</Project>
"""


class SourceInventoryTests(unittest.TestCase):
    def create_repo(self, base: Path) -> Path:
        repo = base / "repo"
        source = repo / "tmproject" / "TMProject748"
        (source / "tests").mkdir(parents=True)
        (source / "Dependencies" / "Vendor").mkdir(parents=True)
        (source / "internal").mkdir()
        (source / "internal" / "sample.cpp").write_text(
            "#include <vector>\n"
            "// visible declaration\n"
            "int declared(int value);\n"
            "#define DOUBLE(v) ((v) * 2)\n"
            "int declared(int value) {\n"
            "    auto plus = [value](int add) { return value + add; };\n"
            "    return plus(DOUBLE(2));\n"
            "}\n",
            encoding="utf-8",
        )
        (source / "internal" / "excluded.cpp").write_text("int excluded() { return 0; }\n", encoding="utf-8")
        (source / "internal" / "sample.h").write_text(
            "void header_only();\n"
            "class First { void update(); };\n"
            "class Second { void update(); };\n",
            encoding="utf-8",
        )
        (source / "internal" / "legacy.cpp").write_text(
            "DWORD WINAPI worker(void* value) { return value != nullptr; }\n"
            "const char* text = _T(\"first\") _T(\"second\");\n"
            "void allocate(int count) { auto values = new Widget*[count]; }\n"
            "void release(Texture* value) {\n"
            "    SAFE_RELEASE(value)\n"
            "}\n"
            "bool retry() {\n"
            "    return attempt() || (tries += 10, state.port = htons(tries + 5000), attempt());\n"
            "}\n",
            encoding="utf-8",
        )
        (source / "tests" / "sample_test.cpp").write_text("int test_case() { return 1; }\n", encoding="utf-8")
        (source / "Dependencies" / "Vendor" / "vendor.h").write_text(
            "// third-party boundary\nint vendor();\n", encoding="utf-8"
        )
        (source / "TMProject748.vcxproj").write_text(
            PROJECT_TEMPLATE.format(items="""
    <ClCompile Include="internal\\sample.cpp" />
    <ClCompile Include="internal\\legacy.cpp" />
    <ClCompile Include="internal\\excluded.cpp">
      <ExcludedFromBuild Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">true</ExcludedFromBuild>
    </ClCompile>
    <ClInclude Include="internal\\sample.h" />
    <ClInclude Include="Dependencies\\Vendor\\vendor.h" />"""),
            encoding="utf-8",
        )
        (source / "tests" / "ArchitectureTests.vcxproj").write_text(
            PROJECT_TEMPLATE.format(items='<ClCompile Include="sample_test.cpp" />'),
            encoding="utf-8",
        )
        return repo

    @staticmethod
    def rows(text: str) -> list[dict[str, str]]:
        return list(csv.DictReader(io.StringIO(text), delimiter="\t"))

    def test_inventory_is_deterministic_and_covers_lines_and_roles(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            repo = self.create_repo(Path(temporary))
            source = repo / "tmproject" / "TMProject748"
            inputs = (source, source / "TMProject748.vcxproj", source / "tests" / "ArchitectureTests.vcxproj")
            first = source_inventory.build_inventory(*inputs)
            second = source_inventory.build_inventory(*inputs)
            self.assertEqual(first, second)

            files = {row["path"]: row for row in self.rows(first[0])}
            self.assertEqual(files["internal/sample.cpp"]["main_release_win32"], "compile")
            self.assertEqual(files["internal/legacy.cpp"]["main_release_win32"], "compile")
            self.assertGreater(int(files["internal/legacy.cpp"]["raw_parse_errors"]), 0)
            self.assertGreater(int(files["internal/legacy.cpp"]["parser_normalizations"]), 0)
            self.assertEqual(int(files["internal/legacy.cpp"]["parse_errors"]), 0)
            self.assertEqual(files["internal/excluded.cpp"]["main_release_win32"], "excluded")
            self.assertEqual(files["tests/sample_test.cpp"]["test_release_win32"], "compile")
            self.assertEqual(files["Dependencies/Vendor/vendor.h"]["ownership"], "THIRD_PARTY")
            self.assertEqual(files["Dependencies/Vendor/vendor.h"]["parser"], "boundary-only")
            for row in files.values():
                self.assertEqual(int(row["lines"]), int(row["classified_lines"]))

            symbols = [row for row in self.rows(first[1]) if row["path"] == "internal/sample.cpp"]
            kinds = {row["kind"] for row in symbols}
            self.assertTrue({"function", "lambda", "macro", "declaration"}.issubset(kinds))
            self.assertIn("declared", {row["symbol"] for row in symbols})
            all_symbols = self.rows(first[1])
            self.assertIn("First::update", {row["symbol"] for row in all_symbols})
            self.assertIn("Second::update", {row["symbol"] for row in all_symbols})
            self.assertEqual(len(all_symbols), len({row["source_id"] for row in all_symbols}))
            self.assertEqual(first[2]["parse_errors"], 0)

    def test_cli_write_and_check_detect_stale_output(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            repo = self.create_repo(Path(temporary))
            files_output = repo / "files.tsv"
            symbols_output = repo / "symbols.tsv"
            common = [
                "--repo", str(repo),
                "--files-output", str(files_output),
                "--symbols-output", str(symbols_output),
            ]
            self.assertEqual(source_inventory.main([*common, "--write"]), 0)
            self.assertEqual(source_inventory.main([*common, "--check"]), 0)
            files_output.write_text("stale\n", encoding="utf-8")
            self.assertEqual(source_inventory.main([*common, "--check"]), 1)

    def test_strict_mode_rejects_unrecognized_parse_error(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            repo = self.create_repo(Path(temporary))
            source = repo / "tmproject" / "TMProject748"
            (source / "internal" / "sample.cpp").write_text("int broken( {\n", encoding="utf-8")
            self.assertEqual(source_inventory.main(["--repo", str(repo), "--fail-on-parse-errors"]), 1)


if __name__ == "__main__":
    unittest.main()
