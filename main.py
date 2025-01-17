import streamlit as st
from PIL import Image
import io
from typing import NewType

####### UTILS #######

Int16 = NewType('Int16', int)
Int8 = NewType('Int8', int)

def get_high_and_low_bytes(num: Int16) -> list[Int8, Int8]:
    high_byte = (num >> 8) & 0xFF
    low_byte = num & 0xFF
    return [high_byte, low_byte]

####### IMAGE FUNCTIONS #######

def convert_image_to_rgb565(image, width, height):
    img = image.resize((width, height), Image.Resampling.BILINEAR)
    img = img.convert("RGB")
    rgb565_data = []

    img = Image.eval(img, lambda x: int(x * 1.1) if x * 1.1 < 255 else 255)

    for pixel in img.getdata():
        r, g, b = pixel
        rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
        rgb565_data.append(rgb565)

    return rgb565_data

def process_image(image, dimensions: tuple[int, int], caption: str):
    width, height = dimensions
    rgb565_data = convert_image_to_rgb565(image, width, height)

    byte_array = bytearray()
    byte_array.extend([width >> 8, width & 0xFF, height >> 8, height & 0xFF])
    byte_array.extend(caption.encode(encoding="437") + b'\0')

    data_1 = encoding_1(rgb565_data)
    data_2 = encoding_2(rgb565_data)

    typ = 0
    data = data_1
    if len(data_1) * 0.05 > len(data_2):
        typ = 1
        data = data_2

    byte_array.extend([typ])
    byte_array.extend(data)

    return byte_array

def encoding_1(rgb565_data: list[Int16]) -> list[Int8]:
    final_data: list[Int8] = []
    for color in rgb565_data:
        final_data.extend(get_high_and_low_bytes(color))
    return final_data

def encoding_2(rgb565_data: list[Int16]) -> list[Int8]:
    def count_contiguous(data: list[Int16], start: int) -> int:
        count = 1
        for i in range(start + 1, len(data)):
            if data[i] == data[start]:
                count += 1
            else:
                break
        return count

    final_data: list[Int8] = []
    ptr = 0
    while ptr < len(rgb565_data):
        count = count_contiguous(rgb565_data, ptr)
        pixel = get_high_and_low_bytes(rgb565_data[ptr])
        while count > 0:
            qtt = min(0xFF, count)
            final_data.extend([qtt, *pixel])
            count -= qtt
            ptr += qtt

    return final_data

###### Streamlit Interface ######
AVAILABLE_DIMENSIONS = [(320, 320)]

# Initialize session state for language
if "language" not in st.session_state:
    st.session_state.language = "es"  # Default language is Spanish

# Language toggle
def toggle_language():
    st.session_state.language = "en" if st.session_state.language == "es" else "es"

# Define text translations
translations = {
    "title": {"en": "Image Processor for Assistive Voices", "es": "Procesador de Imágenes para Voces Asistentes"},
    "subheader": {"en": "Provided by D-Lab Team Mexico", "es": "Proveído por D-Lab Grupo México"},
    "upload_label": {"en": "Upload an image", "es": "Sube una imagen"},
    "dimensions_label": {"en": "Select dimensions", "es": "Selecciona dimensiones"},
    "caption_label": {"en": "Enter a caption for the file (this text will appear under the image).", "es": "Introduce un título para la imagen (este texto aparecerá debajo de la imagen)."},
    "filename_label": {"en": "Enter a filename for the download file (this is just the file name, but should not contain spaces or typical Spanish accents like á, é, í, ó, ú, ñ. Should be 8 characters or less).", 
                       "es": "Introduce un nombre para el archivo descargable (esto es solo el nombre del archivo, pero no debe contener espacios ni acentos típicos como á, é, í, ó, ú, ñ. Debe tener 8 caracteres o menos)."},
    "filename_error": {"en": "Filename must be 8 characters or fewer.", "es": "El nombre del archivo debe tener 8 caracteres o menos."},
    "caption_error": {"en": "Caption cannot have special characters. Typed: ", "es": "El título no puede tener caracteres especiales. Escribió: "},
    "process_button": {"en": "Process Image", "es": "Procesar Imagen"},
    "success_message": {"en": "Image processed successfully! Use the download button to save the file.",
                        "es": "¡Imagen procesada con éxito! Usa el botón de descarga para guardar el archivo."},
    "error_message": {"en": "An error occurred:", "es": "Ocurrió un error:"},
    "download_button": {"en": "Download Processed Image", "es": "Descargar Imagen Procesada"},
    "language_button": {"en": "Switch to Spanish", "es": "Cambiar a Inglés"},
}

def main():
    session_language = st.session_state.language
    st.set_page_config(page_title=translations["title"][st.session_state.language], page_icon=":smiley:")
    st.title(translations["title"][session_language] + " :smiley: :microphone:")
    st.subheader(translations["subheader"][session_language])

    st.button(translations["language_button"][st.session_state.language], on_click=toggle_language)

    uploaded_file = st.file_uploader(translations["upload_label"][st.session_state.language], type=["png", "jpg", "jpeg", "bmp", "gif"])
    if uploaded_file:
        st.image(uploaded_file, caption="Uploaded Image", use_container_width=True)

    dimensions = st.selectbox(
        translations["dimensions_label"][st.session_state.language],
        ["x".join(map(str, dim)) for dim in AVAILABLE_DIMENSIONS],
        help="Will redimension and not crop."
    )

    caption = st.text_input(translations["caption_label"][st.session_state.language], "")
    
    filename = st.text_input(
        translations["filename_label"][st.session_state.language],
    )

    if len(filename) > 8:
        st.error(translations["filename_error"][st.session_state.language])

    process_button = st.button(translations["process_button"][st.session_state.language], disabled=not (uploaded_file and len(filename) <= 8))

    if process_button:
        try:
            image = Image.open(uploaded_file)
            data = process_image(image, tuple(map(int, dimensions.split("x"))), caption=caption.strip())
            data_io = io.BytesIO(data)

            st.download_button(
                label=translations["download_button"][st.session_state.language],
                data=data_io,
                file_name=f"{filename}",
                mime="application/octet-stream"
            )

            st.success(translations["success_message"][st.session_state.language])

        except Exception as e:
            st.error(f"{translations['error_message'][st.session_state.language]} {e}")

if __name__ == "__main__":
    main()