from gtts import gTTS
from io import BytesIO
import streamlit as st
sound_file = BytesIO()
tts = gTTS('Alacrán', lang='es', tld="com.mx")
tts.write_to_fp(sound_file)
st.audio(sound_file)