# ogg_covertor Xiaozhi AI OGG sound effect batch converter

This script is an OGG batch conversion tool that supports converting input audio files into the OGG format that Xiaozhi can use.

Implemented based on Python third-party library `ffmpeg-python`, **requires**`ffmpeg` environment

You can go [here](https://ffmpeg.org/download.html) to download the ffmpeg distribution corresponding to your own system, and add it to the environment variable or place it in the directory where the script is located

Supports functions such as mutual conversion between OGG and audio, loudness adjustment, etc.

#Create and activate virtual environment

```bash
# Create virtual environment
python -m venv venv
# Activate virtual environment
source venv/bin/activate # Mac/Linux
venv\Scripts\activate # Windows
```
# Download FFmpeg
Go [here](https://ffmpeg.org/download.html) to download ffmpeg
Download the corresponding version according to your current system and place the executable file of `ffmpeg` in the directory where the script is located or add the directory where the executable file is located to the environment variable

# Install dependencies
Please execute in a virtual environment

```bash
pip install ffmpeg-python
```

# run script
```bash
python ogg_covertor.py
```