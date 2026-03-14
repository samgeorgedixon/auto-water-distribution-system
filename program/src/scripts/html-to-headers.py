import os

HTML_FOLDER = os.path.join("src", "html")
OUTPUT_FOLDER = os.path.join("src", "html-includes")

os.makedirs(OUTPUT_FOLDER, exist_ok=True)

for filename in os.listdir(HTML_FOLDER):
    if filename.endswith(".html"):
        filepath = os.path.join(HTML_FOLDER, filename)
        var_name = filename.replace(".", "_")  # e.g., wifi_html

        with open(filepath, "r", encoding="utf-8") as f:
            html = f.read()

        header_content = f"""#pragma once
#include <Arduino.h>

const char {var_name}[] PROGMEM = R"rawliteral(
{html}
)rawliteral";
"""

        header_file = os.path.join(OUTPUT_FOLDER, var_name + ".h")
        with open(header_file, "w", encoding="utf-8") as out:
            out.write(header_content)

        print(f"Generated {header_file}")
