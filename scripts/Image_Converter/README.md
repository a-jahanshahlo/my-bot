# LVGL image conversion tool

This directory contains two Python scripts for processing and converting images to LVGL format:

## 1. LVGLImage (LVGLImage.py)

Quoted from the conversion script [LVGLImage.py](https://github.com/lvgl/lvgl/blob/master/scripts/LVGLImage.py) of LVGL[official repo](https://github.com/lvgl/lvgl)

## 2. LVGL image conversion tool (lvgl_tools_gui.py)

Call `LVGLImage.py` to batch convert images to LVGL image format
It can be used to modify Xiaozhi’s default expression. The detailed modification tutorial is [here](https://www.bilibili.com/video/BV12FQkYeEJ3/)

### Features

-Graphical operation, more friendly interface
-Support batch conversion of pictures
-Automatically identify picture formats and select the best color format for conversion
-Multi-resolution support

### How to use

Create a virtual environment
```bash
#Create venv
python -m venv venv
#Activate environment
source venv/bin/activate # Linux/Mac
venv\Scripts\activate # Windows
```

Install dependencies
```bash
pip install -r requirements.txt
```

Run the conversion tool

```bash
#Activate environment
source venv/bin/activate # Linux/Mac
venv\Scripts\activate # Windows
# run
python lvgl_tools_gui.py
```
