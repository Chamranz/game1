#!/usr/bin/env python3
"""
NEON REQUIEM — Post-Build Script
Copies firmware binary and generates build info.
"""

import os
import sys
import json
from datetime import datetime

Import("env")

def post_build(source, target, env):
    """Post-build hook: copy firmware and generate metadata."""
    # Get firmware path
    firmware_path = os.path.join(env.subst("$BUILD_DIR"), "firmware.bin")
    
    if not os.path.exists(firmware_path):
        print(f"[POST_BUILD] firmware.bin not found at {firmware_path}")
        return
    
    # Create output directory
    output_dir = os.path.join(env.subst("$PROJECT_DIR"), "dist")
    os.makedirs(output_dir, exist_ok=True)
    
    # Copy firmware
    dest_path = os.path.join(output_dir, "neon_requiem_firmware.bin")
    with open(firmware_path, "rb") as src:
        with open(dest_path, "wb") as dst:
            dst.write(src.read())
    
    # Generate build metadata
    metadata = {
        "project": "NEON REQUIEM",
        "version": env.subst("$GAME_VERSION_STR") or "1.0.0",
        "build_date": datetime.now().isoformat(),
        "board": env.subst("$BOARD"),
        "platform": env.subst("$PLATFORM"),
        "firmware_size": os.path.getsize(firmware_path),
        "flash_size": "16MB",
        "psram_size": "8MB"
    }
    
    meta_path = os.path.join(output_dir, "build_info.json")
    with open(meta_path, "w") as f:
        json.dump(metadata, f, indent=2)
    
    print(f"[POST_BUILD] Firmware copied to {dest_path}")
    print(f"[POST_BUILD] Build info written to {meta_path}")
    print(f"[POST_BUILD] Firmware size: {metadata['firmware_size']} bytes")

env.AddPostAction("buildprog", post_build)