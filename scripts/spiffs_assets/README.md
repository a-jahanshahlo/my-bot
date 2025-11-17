# SPIFFS Assets Builder

This script is used to build the SPIFFS resource partition of the ESP32 project, packaging various resource files into a format that can be used on the device.

## Features

-Handle WakeNet Model
-Integrated text font files
-Process emoji picture collections
-Automatically generate resource index files
-Package and generate the final `assets.bin` file

## Dependency requirements

-Python 3.6+
-Related resource files

## How to use

### Basic syntax

```bash
./build.py --wakenet_model <wakenet_model_dir> \
    --text_font <text_font_file> \
    --emoji_collection <emoji_collection_dir>
```

### Parameter description

| Parameters | Type | Required | Description |
|------|------|------|------|
| `--wakenet_model` | Directory path | No | Wakenet model directory path |
| `--text_font` | File path | No | Text font file path |
| `--emoji_collection` | Directory path | No | Emoji picture collection directory path |

### Usage examples

```bash
# Complete parameter example
./build.py \
    --wakenet_model ../../managed_components/espressif__esp-sr/model/wakenet_model/wn9_nihaoxiaozhi_tts \
    --text_font ../../components/xiaozhi-fonts/build/font_puhui_common_20_4.bin \
    --emoji_collection ../../components/xiaozhi-fonts/build/emojis_64/

# Only process font files
./build.py --text_font ../../components/xiaozhi-fonts/build/font_puhui_common_20_4.bin

# Only handle emoticons
./build.py --emoji_collection ../../components/xiaozhi-fonts/build/emojis_64/
```

## Workflow

1. **Create build directory structure**
   -`build/` -main build directory
   -`build/assets/` -resource file directory
   -`build/output/` -output file directory

2. **Processing wake-up network model**
   -Copy the model files to the build directory
   -Use `pack_model.py` to generate `srmodels.bin`
   -Copy the generated model files to the resource directory

3. **Handling text fonts**
   -Copy font files to resource directory
   -Support font files in `.bin` format

4. **Processing Emoji Collections**
   -Scan image files in the specified directory
-Supports `.png` and `.gif` formats
   -Automatically generate emoji index

5. **Generate configuration file**
   -`index.json` -resource index file
   -`config.json` -build configuration file

6. **Package final resources**
   -Use `spiffs_assets_gen.py` to generate `assets.bin`
   -Copy to build root directory

## Output file

After the build is completed, the following files will be generated in the `build/` directory:

-`assets/` -all asset files
-`assets.bin` -final SPIFFS resource file
-`config.json` -build configuration
-`output/` -intermediate output file

## Supported resource formats

-**Model file**: `.bin` (processed by pack_model.py)
-**Font file**: `.bin`
-**Image files**: `.png`, `.gif`
-**Configuration file**: `.json`

## Error handling
The script contains a complete error handling mechanism:

-Check if the source file/directory exists
-Verify child process execution results
-Provide detailed error messages and warnings

## Notes

1. Make sure all dependent Python scripts are in the same directory
2. Use an absolute path or a path relative to the script directory for the resource file path.
3. The build process will clean up the previous build files
4. The size of the generated `assets.bin` file is limited by the SPIFFS partition size