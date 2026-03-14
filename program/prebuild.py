import os
import subprocess
import sys

# Path to your Python HTML conversion script
script_path = os.path.join("src/scripts", "html-to-headers.py")

# Run the script using Python
ret = subprocess.call([sys.executable, script_path])
if ret != 0:
    raise Exception("Failed to generate HTML headers")