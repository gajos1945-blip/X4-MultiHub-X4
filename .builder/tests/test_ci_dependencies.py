from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

def test_github_actions_installs_pytest_before_running_tests():
    workflow = (ROOT / ".github/workflows/build-x4-bin.yml").read_text(encoding="utf-8")
    install_pos = workflow.find("uv pip install --system pytest")
    run_pos = workflow.find("python -m pytest -q")
    assert install_pos != -1
    assert run_pos != -1
    assert install_pos < run_pos
