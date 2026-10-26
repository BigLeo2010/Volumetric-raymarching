import os
import subprocess
import tkinter as tk
from tkinter import filedialog, messagebox, ttk
import numpy as np
from Converter import UniversalDicomParser

exe_path = r"engine.exe"

class DicomLauncherApp:

    def __init__(self, root):
        self.root = root
        self.root.title("DICOM Batch Launcher")
        self.root.geometry("640x450")
        self.root.minsize(550, 350)

        # Переменные теперь хранят списки для поддержки множества файлов
        self.dicom_paths = []
        self.all_metadata = []
        self.all_pixels = []

        self._build_ui()

    def _build_ui(self):
        style = ttk.Style()
        style.theme_use("clam")

        main_frame = ttk.Frame(self.root, padding="20")
        main_frame.pack(fill=tk.BOTH, expand=True)

        # 1. Секция выбора файлов
        file_frame = ttk.LabelFrame(
            main_frame, text=" Выберите DICOM файлы (можно несколько) ", padding="10"
        )
        file_frame.pack(fill=tk.X, pady=(0, 15))

        self.path_entry = ttk.Entry(file_frame)
        self.path_entry.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 10))

        browse_btn = ttk.Button(
            file_frame, text="Обзор...", command=self._browse_files
        )
        browse_btn.pack(side=tk.RIGHT)

        # 2. Секция информации об объеме
        self.info_frame = ttk.LabelFrame(
            main_frame, text=" Параметры суммарного объема ", padding="15"
        )
        self.info_frame.pack(fill=tk.BOTH, expand=True, pady=(0, 15))

        self.lbl_files_count = ttk.Label(
            self.info_frame, text="Выбрано файлов: 0", font=("Arial", 10, "bold")
        )
        self.lbl_files_count.pack(anchor=tk.W, pady=2)

        self.lbl_dims = ttk.Label(
            self.info_frame, text="Итоговые размеры (W x H x Суммарный D): —", font=("Arial", 10)
        )
        self.lbl_dims.pack(anchor=tk.W, pady=2)

        self.lbl_dtype = ttk.Label(
            self.info_frame, text="Тип данных (Dtype): —", font=("Arial", 10)
        )
        self.lbl_dtype.pack(anchor=tk.W, pady=2)

        self.lbl_patient = ttk.Label(
            self.info_frame, text="Пациент (первый файл): —", font=("Arial", 10)
        )
        self.lbl_patient.pack(anchor=tk.W, pady=2)

        self.lbl_modality = ttk.Label(
            self.info_frame, text="Модальность: —", font=("Arial", 10)
        )
        self.lbl_modality.pack(anchor=tk.W, pady=2)

        # 3. Кнопка подтверждения и запуска
        self.submit_btn = ttk.Button(
            main_frame,
            text="Запуск пакета",
            command=self._submit_data,
            state=tk.DISABLED,
        )
        self.submit_btn.pack(fill=tk.X, ipady=5)

    def _browse_files(self):
        # ИСПРАВЛЕНО: Добавлен фильтр * без расширения, чтобы видеть файлы типа image_0
        file_paths = filedialog.askopenfilenames(
            title="Открыть DICOM файлы",
            filetypes=[
                ("Все файлы", "*.*"),  # Теперь этот пункт будет главным или доступным
                ("DICOM файлы", "*.dcm *.dicom")
            ],
        )
        if file_paths:
            file_paths = list(file_paths)
            
            # Сортируем файлы по имени (важно для правильного порядка слоев: image_0, image_1...)
            # Обычная сортировка может выстроить их как image_0, image_10, image_11...
            # Если возникнут проблемы с порядком, отсортируем точнее. Пока делаем базовую:
            file_paths.sort()

            self.path_entry.delete(0, tk.END)
            self.path_entry.insert(0, "; ".join(file_paths))
            
            self._process_multiple_dicoms(file_paths)


    def _process_multiple_dicoms(self, file_paths):
        self._reset_ui_info()
        
        valid_paths = []
        errors = []

        for path in file_paths:
            if not os.path.exists(path):
                errors.append(f"Файл не найден: {path}")
                continue
            
            try:
                # Вызов вашего оригинального парсера для каждого файла отдельно
                parser = UniversalDicomParser(path, force=True)
                metadata, pixels = parser.parse()
                
                if pixels is not None:
                    valid_paths.append(path)
                    self.all_metadata.append(metadata)
                    self.all_pixels.append(pixels)
                else:
                    errors.append(f"В файле нет пикселей: {os.path.basename(path)}")
                    
            except Exception as e:
                errors.append(f"Ошибка в файле {os.path.basename(path)}: {str(e)}")

        self.dicom_paths = valid_paths

        if errors:
            error_msg = "\n".join(errors[:5]) # Показываем первые 5 ошибок
            if len(errors) > 5:
                error_msg += f"\n...и еще {len(errors) - 5} ошибок."
            messagebox.showwarning("Проблемы при парсинге", f"Некоторые файлы не удалось прочесть:\n\n{error_msg}")

        if self.dicom_paths:
            self._update_ui_info()
        else:
            messagebox.showerror("Ошибка", "Ни один корректный DICOM файл не был загружен.")
            self._reset_ui_info()

    def _update_ui_info(self):
        total_depth = 0
        final_width = 0
        final_height = 0
        dtype_str = "—"

        # Проверяем размеры всех файлов, чтобы они совпадали по W и H
        sizes_match = True
        
        for idx, pixels in enumerate(self.all_pixels):
            if len(pixels.shape) == 3:
                d, h, w = pixels.shape
            else:
                d = 1
                h, w = pixels.shape
            
            if idx == 0:
                final_width = w
                final_height = h
                dtype_str = str(pixels.dtype)
            elif w != final_width or h != final_height:
                sizes_match = False
            
            total_depth += d

        self.lbl_files_count.config(text=f"Успешно выбрано файлов: {len(self.dicom_paths)}")

        if not sizes_match:
            self.lbl_dims.config(text="Ошибка: Размеры файлов (W x H) различаются!")
            self.submit_btn.config(state=tk.DISABLED)
            return

        self.lbl_dims.config(
            text=f"Итоговые размеры (W x H x D): {final_width} x {final_height} x {total_depth}"
        )
        self.lbl_dtype.config(text=f"Тип данных (Dtype): {dtype_str}")

        # Берем данные пациента из первого успешного файла
        first_meta = self.all_metadata[0]
        patient = first_meta.get("(0010,0010) PatientName", "Не указан")
        modality = first_meta.get("(0008,0060) Modality", "Не указана")

        self.lbl_patient.config(text=f"Пациент: {patient}")
        self.lbl_modality.config(text=f"Модальность: {modality}")

        self.submit_btn.config(state=tk.NORMAL)

    def _reset_ui_info(self):
        self.lbl_files_count.config(text="Выбрано файлов: 0")
        self.lbl_dims.config(text="Итоговые размеры (W x H x Суммарный D): —")
        self.lbl_dtype.config(text="Тип данных (Dtype): —")
        self.lbl_patient.config(text="Пациент: —")
        self.lbl_modality.config(text="Модальность: —")
        self.submit_btn.config(state=tk.DISABLED)
        
        self.dicom_paths = []
        self.all_metadata = []
        self.all_pixels = []

    def _submit_data(self):
        if not self.all_pixels:
            messagebox.showerror("Ошибка", "Нет пиксельных данных для экспорта.")
            return

        try:
            raw_filename = "volume_data.raw"
            config_filename = "volume_config.txt"

            # Определяем базовые размеры по первому файлу
            first_shape = self.all_pixels[0].shape
            if len(first_shape) == 3:
                _, height, width = first_shape
            else:
                height, width = first_shape

            # Склеиваем массивы всех файлов в один общий массив по оси глубины
            # Для этого приводим все к 3D (добавляем ось, если файл был 2D)
            prepared_volumes = []
            for pixels in self.all_pixels:
                if len(pixels.shape) == 2:
                    prepared_volumes.append(pixels[np.newaxis, :, :]) # Из (H, W) делаем (1, H, W)
                else:
                    prepared_volumes.append(pixels)

            # Объединяем все слои в один большой монолитный массив
            full_volume = np.concatenate(prepared_volumes, axis=0)
            depth = full_volume.shape[0]

            # === Нормализация всего объема целиком в uint8 ===
            img_min, img_max = full_volume.min(), full_volume.max()
            
            if img_max - img_min != 0:
                normalized_pixels = ((full_volume - img_min) / (img_max - img_min) * 255).astype(np.uint8)
            else:
                normalized_pixels = np.zeros(full_volume.shape, dtype=np.uint8)

            # Записываем байты единого склеенного объема в .raw
            normalized_pixels.tofile(raw_filename)

            # Генерируем файл конфигурации
            with open(config_filename, "w", encoding="utf-8") as config_file:
                config_file.write(f"{width}\n")
                config_file.write(f"{height}\n")
                config_file.write(f"{depth}\n")
                config_file.write(f"uint8\n")
                
                # Записываем пути к файлам. 
                # Так как файлов много, мы запишем их через запятую или построчно. 
                # Ниже пишем пути построчно (начиная с первого главного файла)
                for path in self.dicom_paths:
                    config_file.write(f"{path}\n")

            print(f"[Успех] Объединенный объем {width}x{height}x{depth} сохранен в {raw_filename}")

            subprocess.run([exe_path])
            self.root.destroy()

        except Exception as e:
            messagebox.showerror("Ошибка экспорта", f"Не удалось объединить и сохранить файлы:\n{str(e)}")


if __name__ == "__main__":
    root = tk.Tk()
    app = DicomLauncherApp(root)
    root.mainloop()
