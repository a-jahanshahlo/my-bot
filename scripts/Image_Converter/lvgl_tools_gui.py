import tkinter as tk
from tkinter import ttk, filedialog, messagebox
from PIL import Image
import os
import tempfile
import sys
from LVGLImage import LVGLImage, ColorFormat, CompressMethod

HELP_TEXT = """Instructions for using the LVGL image conversion tool:

1. Add files: Click the "Add Files" button to select the pictures that need to be converted. Batch import is supported.

2. Remove files: Select the checkbox "[ ]" in front of the file in the list (it will turn into "[√]" after selection), and click "Remove Selected" to delete the selected file.

3. Set resolution: select the required resolution, such as 128x128
   It is recommended to choose according to the screen resolution of your device. Either too large or too small will affect the display effect.

4. Color format: Select "Automatic Recognition" to automatically select the image based on whether it is transparent, or specify it manually.
   Unless you understand this option, it is recommended to use automatic recognition, otherwise some unexpected problems may occur...

5. Compression method: select NONE or RLE compression
   Unless you understand this option, it is recommended to keep the default NONE for no compression

6. Output directory: Set the saving path of the converted files
   The default is the output folder in the directory where the program is located.

7. Convert: Click "Convert All" or "Convert Selected" to start conversion
"""

class ImageConverterApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Lvgl image conversion tool")
        self.root.geometry("750x650")
        
        # initialize variables

        self.output_dir = tk.StringVar(value=os.path.abspath("output"))
        self.resolution = tk.StringVar(value="128x128")
        self.color_format = tk.StringVar(value="automatic recognition")
        self.compress_method = tk.StringVar(value="NONE")

        # Create ui components

        self.create_widgets()
        self.redirect_output()

    def create_widgets(self):
        # Parameter setting framework

        settings_frame = ttk.LabelFrame(self.root, text="Conversion settings")
        settings_frame.grid(row=0, column=0, padx=10, pady=5, sticky="ew")

        # Resolution settings

        ttk.Label(settings_frame, text="resolution:").grid(row=0, column=0, padx=2)
        ttk.Combobox(settings_frame, textvariable=self.resolution, 
                    values=["512x512", "256x256", "128x128", "64x64", "32x32"], width=8).grid(row=0, column=1, padx=2)

        # color format

        ttk.Label(settings_frame, text="color format:").grid(row=0, column=2, padx=2)
        ttk.Combobox(settings_frame, textvariable=self.color_format,
                    values=["automatic recognition", "RGB565", "RGB565A8"], width=10).grid(row=0, column=3, padx=2)

        # Compression method

        ttk.Label(settings_frame, text="Compression method:").grid(row=0, column=4, padx=2)
        ttk.Combobox(settings_frame, textvariable=self.compress_method,
                    values=["NONE", "RLE"], width=8).grid(row=0, column=5, padx=2)

        # File operation framework

        file_frame = ttk.LabelFrame(self.root, text="Select file")
        file_frame.grid(row=1, column=0, padx=10, pady=5, sticky="nsew")

        # File operation button

        btn_frame = ttk.Frame(file_frame)
        btn_frame.pack(fill=tk.X, pady=2)
        ttk.Button(btn_frame, text="Add files", command=self.select_files).pack(side=tk.LEFT, padx=2)
        ttk.Button(btn_frame, text="Remove selection", command=self.remove_selected).pack(side=tk.LEFT, padx=2)
        ttk.Button(btn_frame, text="Clear list", command=self.clear_files).pack(side=tk.LEFT, padx=2)

        # File list (treeview)

        self.tree = ttk.Treeview(file_frame, columns=("selected", "filename"), 
                               show="headings", height=10)
        self.tree.heading("selected", text="choose", anchor=tk.W)
        self.tree.heading("filename", text="file name", anchor=tk.W)
        self.tree.column("selected", width=60, anchor=tk.W)
        self.tree.column("filename", width=600, anchor=tk.W)
        self.tree.pack(fill=tk.BOTH, expand=True)
        self.tree.bind("<ButtonRelease-1>", self.on_tree_click)

        # Output directory

        output_frame = ttk.LabelFrame(self.root, text="Output directory")
        output_frame.grid(row=2, column=0, padx=10, pady=5, sticky="ew")
        ttk.Entry(output_frame, textvariable=self.output_dir, width=60).pack(side=tk.LEFT, padx=5)
        ttk.Button(output_frame, text="Browse", command=self.select_output_dir).pack(side=tk.RIGHT, padx=5)

        # Convert button and help button

        convert_frame = ttk.Frame(self.root)
        convert_frame.grid(row=3, column=0, padx=10, pady=10)
        ttk.Button(convert_frame, text="Convert all files", command=lambda: self.start_conversion(True)).pack(side=tk.LEFT, padx=5)
        ttk.Button(convert_frame, text="Convert selected files", command=lambda: self.start_conversion(False)).pack(side=tk.LEFT, padx=5)
        ttk.Button(convert_frame, text="help", command=self.show_help).pack(side=tk.RIGHT, padx=5)

        # Log area (new clear button section)

        log_frame = ttk.LabelFrame(self.root, text="log")
        log_frame.grid(row=4, column=0, padx=10, pady=5, sticky="nsew")
        
        # Add button frame

        log_btn_frame = ttk.Frame(log_frame)
        log_btn_frame.pack(fill=tk.X, side=tk.BOTTOM)
        
        # Clear log button

        ttk.Button(log_btn_frame, text="Clear log", command=self.clear_log).pack(side=tk.RIGHT, padx=5, pady=2)
        
        self.log_text = tk.Text(log_frame, height=15)
        self.log_text.pack(fill=tk.BOTH, expand=True)

        # layout configuration

        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(1, weight=1)
        self.root.rowconfigure(4, weight=1)

    def clear_log(self):
        """Clear log content"""
        self.log_text.delete(1.0, tk.END)

    def show_help(self):
        messagebox.showinfo("help", HELP_TEXT)

    def redirect_output(self):
        class StdoutRedirector:
            def __init__(self, text_widget):
                self.text_widget = text_widget
                self.original_stdout = sys.stdout

            def write(self, message):
                self.text_widget.insert(tk.END, message)
                self.text_widget.see(tk.END)
                self.original_stdout.write(message)

            def flush(self):
                self.original_stdout.flush()

        sys.stdout = StdoutRedirector(self.log_text)

    def on_tree_click(self, event):
        region = self.tree.identify("region", event.x, event.y)
        if region == "cell":
            col = self.tree.identify_column(event.x)
            item = self.tree.identify_row(event.y)
            if col == "#1":  # Click on the selected column

                current_val = self.tree.item(item, "values")[0]
                new_val = "[√]" if current_val == "[ ]" else "[ ]"
                self.tree.item(item, values=(new_val, self.tree.item(item, "values")[1]))

    def select_output_dir(self):
        path = filedialog.askdirectory()
        if path:
            self.output_dir.set(path)

    def select_files(self):
        files = filedialog.askopenfilenames(filetypes=[("Image files", "*.png;*.jpg;*.jpeg;*.bmp;*.gif")])
        for f in files:
            self.tree.insert("", tk.END, values=("[ ]", os.path.basename(f)), tags=(f,))

    def remove_selected(self):
        to_remove = []
        for item in self.tree.get_children():
            if self.tree.item(item, "values")[0] == "[√]":
                to_remove.append(item)
        for item in reversed(to_remove):
            self.tree.delete(item)

    def clear_files(self):
        for item in self.tree.get_children():
            self.tree.delete(item)

    def start_conversion(self, convert_all):
        input_files = [
            self.tree.item(item, "tags")[0]
            for item in self.tree.get_children()
            if convert_all or self.tree.item(item, "values")[0] == "[√]"
        ]
        
        if not input_files:
            msg = "No convertible files found" if convert_all else "No files selected"
            messagebox.showwarning("warn", msg)
            return
        
        os.makedirs(self.output_dir.get(), exist_ok=True)
        
        # Parse conversion parameters

        width, height = map(int, self.resolution.get().split('x'))
        compress = CompressMethod.RLE if self.compress_method.get() == "RLE" else CompressMethod.NONE

        # perform conversion

        self.convert_images(input_files, width, height, compress)

    def convert_images(self, input_files, width, height, compress):
        success_count = 0
        total_files = len(input_files)
        
        for idx, file_path in enumerate(input_files):
            try:
                print(f"Processing: {os.path.basename(file_path)}")
                
                with Image.open(file_path) as img:
                    # Resize image

                    img = img.resize((width, height), Image.Resampling.LANCZOS)
                    
                    # Handle color formats

                    color_format_str = self.color_format.get()
                    if color_format_str == "automatic recognition":
                        # Detect transparent channels

                        has_alpha = img.mode in ('RGBA', 'LA') or (img.mode == 'P' and 'transparency' in img.info)
                        if has_alpha:
                            img = img.convert('RGBA')
                            cf = ColorFormat.RGB565A8
                        else:
                            img = img.convert('RGB')
                            cf = ColorFormat.RGB565
                    else:
                        if color_format_str == "RGB565A8":
                            img = img.convert('RGBA')
                            cf = ColorFormat.RGB565A8
                        else:
                            img = img.convert('RGB')
                            cf = ColorFormat.RGB565

                    # Save the adjusted image

                    base_name = os.path.splitext(os.path.basename(file_path))[0]
                    output_image_path = os.path.join(self.output_dir.get(), f"{base_name}_{width}x{height}.png")
                    img.save(output_image_path, 'PNG')

                    # Create temporary files

                    with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as tmpfile:
                        temp_path = tmpfile.name
                        img.save(temp_path, 'PNG')

                    # Convert to LVGL C array

                    lvgl_img = LVGLImage().from_png(temp_path, cf=cf)
                    output_c_path = os.path.join(self.output_dir.get(), f"{base_name}.c")
                    lvgl_img.to_c_array(output_c_path, compress=compress)

                    success_count += 1
                    os.unlink(temp_path)
                    print(f"Successfully converted: {base_name}.c\n")

            except Exception as e:
                print(f"Conversion failed: {str(e)}\n")

        print(f"Conversion completed! success {success_count}/{total_files} files\n")

if __name__ == "__main__":
    root = tk.Tk()
    app = ImageConverterApp(root)
    root.mainloop()
