from pathlib import Path
import ast

ROOT = Path(__file__).resolve().parents[2]

def test_every_required_runtime_marker_exists_in_overlay_source():
    guard_path = ROOT / ".builder/tools/release_guard.py"
    tree = ast.parse(guard_path.read_text(encoding="utf-8"))

    required = None
    for node in tree.body:
        if isinstance(node, ast.Assign):
            for target in node.targets:
                if isinstance(target, ast.Name) and target.id == "REQUIRED_MARKERS":
                    required = ast.literal_eval(node.value)
                    break

    assert required is not None

    overlay = ROOT / ".builder/overlay"
    blobs = []
    for path in overlay.rglob("*"):
        if path.is_file():
            blobs.append(path.read_bytes())
    joined = b"\n".join(blobs)

    missing = [marker.decode("utf-8", errors="replace")
               for marker in required if marker not in joined]
    assert not missing, f"Release markers absent from source overlay: {missing}"
