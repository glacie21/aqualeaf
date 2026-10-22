"""AquaLeaf pre-build check: fail early with a clear message when
include/Secrets.h is missing, instead of an obscure compiler error."""

import os

Import("env")  # noqa: F821 — provided by PlatformIO/SCons

project_dir = env.subst("$PROJECT_DIR")  # noqa: F821
secrets = os.path.join(project_dir, "include", "Secrets.h")

if not os.path.isfile(secrets):
    print("")
    print("ERROR: include/Secrets.h not found.")
    print("  Create it from the template and fill in your credentials:")
    print("    cp include/Secrets.example.h include/Secrets.h        (Linux/macOS)")
    print("    Copy-Item include/Secrets.example.h include/Secrets.h (PowerShell)")
    print("")
    env.Exit(1)  # noqa: F821
