import pydicom
from pydicom.dataset import Dataset
from pydicom.sequence import Sequence
import numpy as np

class UniversalDicomParser:
    def __init__(self, file_path: str, force: bool = False):
        """
        force=True позволяет читать файлы без стандартного префикса 'DICM'
        """
        self.file_path = file_path
        self.force = force
        self.dataset = None

    def parse(self) -> tuple[dict, np.ndarray | None]:
        """
        Основной метод парсинга.
        Возвращает кортеж: (словарь_метаданных, массив_пикселей)
        """
        try:
            self.dataset = pydicom.dcmread(self.file_path, force=self.force)
        except Exception as e:
            raise RuntimeError(f"Ошибка чтения DICOM файла: {e}")

        metadata = self._extract_all_metadata()
        pixel_array = self._extract_pixel_data()

        return metadata, pixel_array

    def _convert_value(self, value):
        """Рекурсивно преобразует внутренние типы pydicom в стандартные типы Python."""
        if isinstance(value, Sequence):
            return [
                {f"({elem.tag.group:04X},{elem.tag.element:04X}) {elem.keyword or 'Unknown'}": self._convert_value(elem.value)
                 for elem in item}
                for item in value if isinstance(item, Dataset)
            ]
        
        if isinstance(value, pydicom.valuerep.PersonName):
            return str(value)
        if isinstance(value, (pydicom.multival.MultiValue, list)):
            return [self._convert_value(v) for v in value]
        if isinstance(value, bytes):
            try:
                return value.decode('utf-8', errors='ignore').strip()
            except:
                return f"Raw Bytes ({len(value)} bytes)"
        
        return value

    def _extract_all_metadata(self) -> dict:
        """Собирает файловые метаданные и теги датасета в один плоский словарь."""
        result = {}

        # 1. Чтение File Meta Information (группа 0002)
        for elem in self.dataset.file_meta:
            key = f"(0002,{elem.tag.element:04X}) {elem.keyword or 'Unknown'}"
            result[key] = self._convert_value(elem.value)

        # 2. Чтение основных тегов датасета
        for elem in self.dataset:
            # Пропускаем сам массив пикселей, чтобы не дублировать огромный объем данных текстом
            if elem.keyword == "PixelData" or elem.tag == 0x7FE00010:
                result[f"({elem.tag.group:04X},{elem.tag.element:04X}) PixelData"] = "[Pixel Data Payload]"
                continue

            tag_str = f"({elem.tag.group:04X},{elem.tag.element:04X})"
            key = f"{tag_str} {elem.keyword or 'PrivateTag'}"
            result[key] = self._convert_value(elem.value)

        return result

    def _extract_pixel_data(self) -> np.ndarray | None:
        """Извлекает массив пикселей в виде numpy array с учетом Rescale Slope/Intercept."""
        if 'PixelData' not in self.dataset:
            return None

        try:
            # pydicom автоматически распаковывает сжатые форматы (JPEG, JPEG-LS, JPEG 2000, RLE)
            img_array = self.dataset.pixel_array

            # Применяем коэффициенты трансформации для КТ/МРТ (HU единицы), если они есть
            if 'RescaleSlope' in self.dataset and 'RescaleIntercept' in self.dataset:
                slope = float(self.dataset.RescaleSlope)
                intercept = float(self.dataset.RescaleIntercept)
                # Переводим во float, чтобы не потерять точность при умножении
                img_array = img_array.astype(np.float32) * slope + intercept

            return img_array
        except Exception as e:
            print(f"[Warning] Не удалось извлечь пиксельные данные: {e}")
            return None
