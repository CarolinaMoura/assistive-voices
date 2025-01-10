import streamlit as st
from PIL import Image
import io

AVAILABLE_DIMENSIONS = [(320, 320)]

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

    # Add the dimensions of the image as the first four bytes
    byte_array.extend([ width >> 8, width & 0xFF, height >> 8, height & 0xFF])

    # Next byte is the size of the caption (maximum of 256 characters, so 1 byte)
    byte_array.extend([len(caption)])

    # Add the caption as bytes
    byte_array.extend(caption.encode())

    for color in rgb565_data:
        high_byte = (color >> 8) & 0xFF
        low_byte = color & 0xFF
        byte_array.extend([high_byte, low_byte])

    return byte_array

###### Streamlit Interface ######
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
