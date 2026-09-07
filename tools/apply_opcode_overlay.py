#!/usr/bin/env python3
"""Add the curated opcode X-macro relationships Graphify cannot extract."""

import json
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
OVERLAY_PATH = ROOT / "graphify-out/opcode-overlay.json"
GRAPH_PATH = ROOT / "graphify-out/graph.json"


def line_number(path, pattern):
    match = re.search(pattern, path.read_text(encoding="utf-8"), re.MULTILINE)
    if match is None:
        raise ValueError(f"Missing {pattern!r} in {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")[: match.start()].count("\n") + 1


def overlay_link(source, target, context, source_file, source_line):
    return {
        "source": source,
        "target": target,
        "relation": "references",
        "_origin": "overlay",
        "confidence": "EXTRACTED",
        "confidence_score": 1.0,
        "context": context,
        "source_file": source_file,
        "source_location": f"L{source_line}",
        "weight": 1.0,
    }


def main():
    overlay = json.loads(OVERLAY_PATH.read_text(encoding="utf-8"))
    graph = json.loads(GRAPH_PATH.read_text(encoding="utf-8"))
    enum_file = ROOT / overlay["enum_file"]
    macro = re.search(
        rf"#define {overlay['macro']}\(OPCODE\)(.*?)(?=\n\n#define)",
        enum_file.read_text(encoding="utf-8"),
        re.DOTALL,
    )
    if macro is None:
        raise ValueError(f"Missing {overlay['macro']} definition")
    opcodes = re.findall(r"OPCODE\((\w+)\)", macro.group(1))
    if not opcodes:
        raise ValueError("No opcode definitions found")

    nodes = [node for node in graph["nodes"] if node.get("_origin") != "overlay"]
    links = [link for link in graph["links"] if link.get("_origin") != "overlay"]
    base_link_count = len(links)
    enum_id = overlay["enum_id"]
    nodes.append({
        "id": enum_id,
        "label": overlay["enum_label"],
        "_origin": "overlay",
        "file_type": "code",
        "norm_label": overlay["enum_label"].lower(),
        "source_file": overlay["enum_file"],
        "source_location": "L71",
    })

    for index, opcode in enumerate(opcodes):
        opcode_id = f"include_opcode_h_{opcode.lower()}"
        line = line_number(enum_file, rf"^\s*OPCODE\({opcode}\)")
        nodes.append({
            "id": opcode_id,
            "label": opcode,
            "_origin": "overlay",
            "file_type": "code",
            "norm_label": opcode.lower(),
            "source_file": overlay["enum_file"],
            "source_location": f"L{line}",
        })
        links.append(overlay_link(enum_id, opcode_id, "enum member", overlay["enum_file"], line))
        isa_line = line_number(ROOT / overlay["documentation"]["file"], rf"^\| {index} \| `{opcode}`")
        links.append(overlay_link(overlay["documentation"]["id"], opcode_id, "ISA opcode", overlay["documentation"]["file"], isa_line))
        for handler in overlay["handlers"]:
            handler_line = line_number(ROOT / handler["file"], rf"^\s*case {opcode}:")
            links.append(overlay_link(handler["id"], opcode_id, handler["context"], handler["file"], handler_line))

    for step in overlay["pipeline"]:
        links.append(overlay_link(enum_id, step["id"], step["context"], step["file"], step["line"]))

    overlay_link_count = len(links) - base_link_count
    graph["nodes"] = nodes
    graph["links"] = links
    GRAPH_PATH.write_text(json.dumps(graph, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(f"Applied {len(opcodes)} opcode nodes and {overlay_link_count} overlay links")


if __name__ == "__main__":
    main()
