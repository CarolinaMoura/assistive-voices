import streamlit as st
from PIL import Image
import io
from typing import NewType

####### UTILS #######

Int16 = NewType('Int16', int)
Int8 = NewType('Int8', int)

def get_high_and_low_bytes(num: Int16) -> list[Int8, Int8]:
    """
    Given a 16-bit integer, returns a tuple containing the high and low bytes.
    """
    high_byte = (num >> 8) & 0xFF
    low_byte = num & 0xFF
    return [high_byte, low_byte]

####### IMAGE FUNCTIONS #######

def convert_image_to_rgb565(image, width, height):
    img = image.resize((width, height), Image.Resampling.BILINEAR)
    img = img.convert("RGB")
    rgb565_data = []

    # Normalize colors (optional, to enhance colors)
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

    # Dimensions
    byte_array.extend([ width >> 8, width & 0xFF, height >> 8, height & 0xFF])

    # Size of the caption (maximum of 256 characters, so 1 byte)
    # byte_array.extend([len(caption.encode())])
    byte_array.extend(caption.encode() + b'\0')

    data_1 = encoding_1(rgb565_data)
    data_2 = encoding_2(rgb565_data)
    
    typ = 0
    data = data_1

    if len(data_1) > len(data_2):
        typ = 1
        data = data_2

    byte_array.extend([typ])
    byte_array.extend(data)
    
    return byte_array

def encoding_1(rgb565_data: list[Int16]) -> list[Int8]:
    """
    Encodes a list of 16-bit RGB565 color values into a list of 8-bit values.
    """
    final_data: list[Int8] = []
    for color in rgb565_data:
        final_data.extend(get_high_and_low_bytes(color))
    return final_data

def encoding_2(rgb565_data: list[Int16]) -> list[Int8]:
    """
    Encodes a list of RGB565 data using a more efficient method.
    Args:
        rgb565_data (list[Int16]): A list of 16-bit integers representing RGB565 encoded data.
    Returns:
        list[Int8]: A list of 8-bit integers representing the encoded RGB565 data according
                    to encoding algorithm 2.
    """
    def count_contiguous(data: list[Int16], start: int) -> int:
        """
        Counts the number of contiguous elements in the list starting from the given index.
        """
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

def main():
    st.set_page_config(page_title="Assistive Voices", page_icon=":smiley:")
    st.title("Image Processor for Assistive Voices :smiley: :microphone:")
    st.subheader("Provided by D-Lab Team Mexico")

    # File uploader
    uploaded_file = st.file_uploader("Upload an image", type=["png", "jpg", "jpeg", "bmp", "gif"])
    if uploaded_file:
        st.image(uploaded_file, caption="Uploaded Image", use_container_width=True)

    # Dimensions selection
    dimensions = st.selectbox(
        "Select dimensions",
        ["x".join(map(str, dim)) for dim in AVAILABLE_DIMENSIONS],
        help="Will redimension and not crop."
    )

    # Caption input
    caption = st.text_input("Enter a caption for the file", "")

    # Caption input
    filename = st.text_input("Enter a filename for the download file (max of 8 characters)", "")

    if len(filename) > 8:
        st.error("Filename must be 8 characters or fewer.")

    process_button = st.button("Process Image", disabled=not (uploaded_file and len(filename) <= 8 ))

    if process_button:
        try:
            # Open uploaded image
            image = Image.open(uploaded_file)

            # Process the image
            data = process_image(image, tuple(map(int, dimensions.split("x"))), caption=caption.strip())
            data_io = io.BytesIO(data)

            # Display download button
            st.download_button(
                label="Download Processed Image",
                data=data_io,
                file_name=f"{filename}",
                mime="application/octet-stream"
            )

            st.success("Image processed successfully! Use the download button to save the file.")

        except Exception as e:
            st.error(f"An error occurred: {e}")

if __name__ == "__main__":
    main()