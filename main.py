import unicodedata
import streamlit as st
from PIL import Image
import io
from typing import NewType
import random
from google.cloud import texttospeech
import zipfile

import toml

####### UTILS #######

Int16 = NewType('Int16', int)
Int8 = NewType('Int8', int)


def get_high_and_low_bytes(num: Int16) -> list[Int8, Int8]:
    high_byte = (num >> 8) & 0xFF
    low_byte = num & 0xFF
    return [high_byte, low_byte]

def remove_accents(input_str: str) -> str:
    nfkd_form = unicodedata.normalize('NFKD', input_str)
    no_accents = "".join([c for c in nfkd_form if not unicodedata.combining(c)])
    return "".join([c for c in no_accents if c.isalnum() or c == '_']).replace(" ", "_")

API_KEY = st.secrets["api_key"]
client_options = {
        "api_endpoint": f"https://texttospeech.googleapis.com/v1/text:synthesize?key={API_KEY}"
    }
# client = texttospeech.TextToSpeechClient()


# def synthesize_speech(text, language_code="es-US", voice_name="es-US-Neural2-A"):
#     """
#     Convert text to speech using Google Text-to-Speech API with Spanish voice.

#     Args:
#         text (str): The text to convert to speech.
#         language_code (str): Language code for the voice (default is 'es-ES').
#         voice_name (str): Specific voice name (default is 'es-ES-Wavenet-A').

#     Returns:
#         The audio content.
#     """
#     client = texttospeech.TextToSpeechClient()

#     synthesis_input = texttospeech.SynthesisInput(text=text)

#     voice = texttospeech.VoiceSelectionParams(
#         language_code=language_code,
#         name=voice_name,
#     )

#     audio_config = texttospeech.AudioConfig(
#         audio_encoding=texttospeech.AudioEncoding.MP3
#     )

#     response = client.synthesize_speech(
#         input=synthesis_input, voice=voice, audio_config=audio_config
#     )

#     return response.audio_content


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
    if len(data_1) * 0.01 > len(data_2):
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

if "language" not in st.session_state:
    st.session_state.language = "es"  # Default language is Spanish


def toggle_language():
    st.session_state.language = "en" if st.session_state.language == "es" else "es"


translations = {
    "title": {"en": "Image Processor for Assistive Voices", "es": "Procesador de Imágenes para Voces Asistentes"},
    "subheader": {"en": "Provided by D-Lab Team Mexico", "es": "Proveído por D-Lab Grupo México"},
    "upload_label": {"en": "Upload an image", "es": "Sube una imagen"},
    "upload_metadata": {"en": "Upload the file metadata.txt", "es": "Sube el archivo metadata.txt"},
    "dimensions_label": {"en": "Select dimensions", "es": "Selecciona dimensiones"},
    "caption_label": {"en": "Enter a caption for the file (this text will appear under the image).", "es": "Introduce un título para la imagen (este texto aparecerá debajo de la imagen)."},
    "filename_label": {"en": "Enter a filename for the download file (this is just the file name, but should not contain spaces or typical Spanish accents like á, é, í, ó, ú, ñ. Should be 8 characters or less).",
                       "es": "Introduce un nombre para el archivo descargable (esto es solo el nombre del archivo, pero no debe contener espacios ni acentos típicos como á, é, í, ó, ú, ñ. Debe tener 8 caracteres o menos)."},
    "filename_error": {"en": "Filename must be 8 characters or fewer.", "es": "El nombre del archivo debe tener 8 caracteres o menos."},
    "caption_error": {"en": "Caption cannot have special characters. Typed: ", "es": "El título no puede tener caracteres especiales. Escribió: "},
    "process_button": {"en": "Process Files", "es": "Procesar Archivos"},
    "success_message": {"en": "Image processed successfully! Use the download button to save the file.",
                        "es": "¡Imagen procesada con éxito! Usa el botón de descarga para guardar el archivo."},
    "error_message": {"en": "An error occurred:", "es": "Ocurrió un error:"},
    "download_button": {"en": "Download Processed Image", "es": "Descargar Imagen Procesada"},
    "download_audio_button": {"en": "Download Audio", "es": "Descargar Audio"},
    "download_metadata_button": {"en": "Download Metadata", "es": "Descargar Metadata"},
    "language_button": {"en": "Switch to Spanish", "es": "Cambiar a Inglés"},
    "uploaded_metadata_success": {"en": "Metadata file uploaded successfully.", "es": "Archivo de metadata subido con éxito."},
    "download_zip_button": {"en": "Download All Files 📦", "es": "Descargar Todos los Archivos 📦"}
}


def create_zip_file():
    zip_buffer = io.BytesIO()
    with zipfile.ZipFile(zip_buffer, 'w', zipfile.ZIP_DEFLATED) as zip_file:
        if "processed_image" in st.session_state:
            zip_file.writestr(f"{st.session_state.filename}", st.session_state.processed_image.getvalue())
        
        # if "speech_file" in st.session_state:
        #     zip_file.writestr(st.session_state.audio_filename, st.session_state.speech_file)
        
        if "updated_metadata" in st.session_state:
            zip_file.writestr(st.session_state.metadata_filename, st.session_state.updated_metadata)
    
    return zip_buffer


def main():
    # Add clear check at start
    if st.session_state.get('clear_after_download', False):
        keys_to_clear = [
            'processed_image', 'filename', 'speech_file', 'audio_filename',
            'updated_metadata', 'metadata_filename', 'metadata_content',
            'clear_after_download', 'form_key'  # Add form_key to reset
        ]
        for key in keys_to_clear:
            if key in st.session_state:
                del st.session_state[key]
        st.rerun()

    # Add unique form key to force re-render of file uploaders
    if 'form_key' not in st.session_state:
        st.session_state.form_key = 0

    session_language = st.session_state.language
    st.set_page_config(
        page_title=translations["title"][st.session_state.language], page_icon=":smiley:")
    st.title(translations["title"][session_language] +
             " :smiley: :microphone:")
    st.subheader(translations["subheader"][session_language])

    st.button(translations["language_button"]
              [st.session_state.language], on_click=toggle_language)

    # Add form key to file uploaders
    uploaded_file = st.file_uploader(
        translations["upload_label"][st.session_state.language],
        type=["png", "jpg", "jpeg", "bmp", "gif"],
        key=f"image_uploader_{st.session_state.form_key}"
    )

    if uploaded_file:
        st.image(uploaded_file, caption="Uploaded Image",
                 use_container_width=True)

    metadata_file = st.file_uploader(
        translations["upload_metadata"][session_language],
        type=["txt"],
        key=f"metadata_uploader_{st.session_state.form_key}"
    )
    if metadata_file:
        # Store metadata content in session state when file is uploaded
        metadata_bytes = metadata_file.read()
        st.session_state.metadata_content = metadata_bytes.decode("437")
        st.write(translations["uploaded_metadata_success"][session_language])

    # dimensions = st.selectbox(
    #     translations["dimensions_label"][st.session_state.language],
    #     ["x".join(map(str, dim)) for dim in AVAILABLE_DIMENSIONS],
    #     help="Will redimension and not crop."
    # )
    dimensions = "320x320"

    # Add form key to caption input
    caption = st.text_input(
        translations["caption_label"][st.session_state.language],
        "",
        key=f"caption_{st.session_state.form_key}"
    )

    process_button = st.button(
        translations["process_button"][st.session_state.language])

    if process_button:
        try:
            if uploaded_file:
                image = Image.open(uploaded_file)
                data = process_image(image, tuple(
                    map(int, dimensions.split("x"))), caption=caption.strip())
                data_io = io.BytesIO(data)
                filename = remove_accents(caption)[:8]
                
                # Store in session state instead of showing download button
                st.session_state.processed_image = data_io
                st.session_state.filename = filename

            if "metadata_content" in st.session_state:
                metadata_content = st.session_state.metadata_content
                metadata_lines = len(metadata_content.splitlines())
                audio_filename = str(metadata_lines+1)
                while len(audio_filename) < 4:
                    audio_filename = '0' + audio_filename

                # speech = synthesize_speech(
                #     caption, language_code="es-US", voice_name="es-US-Neural2-A")
                speech = ""
                
                st.session_state.speech_file = speech
                st.session_state.audio_filename = f"{audio_filename}.mp3"

                metadata_content += caption + '\n'
                st.session_state.updated_metadata = metadata_content.encode("437")
                st.session_state.metadata_filename = "metadata.txt"

            st.success(translations["success_message"][st.session_state.language])

        except Exception as e:
            st.error(f"{translations['error_message'][st.session_state.language]} {e}")

    # Replace the download button section with this:
    if any(key in st.session_state for key in ["processed_image", "speech_file", "updated_metadata"]):
        download_container = st.container()
        
        with download_container:
            zip_buffer = create_zip_file()
            if st.download_button(
                label=translations["download_zip_button"][session_language],
                data=zip_buffer.getvalue(),
                file_name="processed_files.zip",
                mime="application/zip"
            ):
                st.session_state.clear_after_download = True
                st.rerun()


if __name__ == "__main__":
    main()
