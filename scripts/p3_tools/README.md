# P3 audio format conversion and playback tool

This directory contains two Python scripts for processing P3 format audio files:

## 1. Audio conversion tool (convert_audio_to_p3.py)

Convert ordinary audio files to P3 format (4-byte header + streaming structure of Opus packets) and perform loudness normalization.

### How to use

```bash
python convert_audio_to_p3.py <input audio file> <output P3 file> [-l LUFS] [-d]
```

Among them, the optional option `-l` is used to specify the target loudness of loudness normalization, which defaults to -16 LUFS; the optional option `-d` can disable loudness normalization.

If the input audio file meets any of the following conditions, it is recommended to use `-d` to disable loudness normalization:
-Audio is too short
-Audio has been adjusted for loudness
-Audio comes from the default TTS (the default loudness of the TTS currently used by Xiaozhi is -16 LUFS)

For example:
```bash
python convert_audio_to_p3.py input.mp3 output.p3
```
## 2. P3 audio playback tool (play_p3.py)

Play audio files in P3 format.

### Features

-Decode and play audio files in P3 format
-Apply fade-out effect when playback ends or user interrupts to avoid audio breakage
-Supports specifying the file to be played through command line parameters

### How to use

```bash
python play_p3.py <P3 file path>
```

For example:
```bash
python play_p3.py output.p3
```

## 3. Audio conversion tool (convert_p3_to_audio.py)

Convert P3 format back to normal audio files.

### How to use

```bash
python convert_p3_to_audio.py <input P3 file> <output audio file>
```

Output audio files need to have an extension.

For example:
```bash
python convert_p3_to_audio.py input.p3 output.wav
```
## 4. Audio/P3 batch conversion tool
A graphical tool that supports batch conversion of audio to P3 and P3 to audio

![](./img/img.png)

### How to use:
```bash
python batch_convert_gui.py
```

## Dependency installation

Before using these scripts, make sure you have the required Python libraries installed:

```bash
pip install librosa opuslib numpy tqdm sounddevice pyloudnorm soundfile
```

Or use the provided requirements.txt file:

```bash
pip install -r requirements.txt
```

## P3 format description

The P3 format is a simple streaming audio format with the following structure:
-Each audio frame consists of a 4-byte header and an Opus encoded data packet
-Header format: [1 byte type, 1 byte reserved, 2 byte length]
-Sampling rate is fixed at 16000Hz, mono
-Each frame is 60ms long