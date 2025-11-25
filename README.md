<div style="
    border: 2px solid #ddd;
    border-radius: 10px;
    padding: 20px;
    background: #fafafa;
">

  <h1 align="center">Groq-Powered ESP32 Voice Assistant</h1>

  <p align="center">
    <b>A hybrid Edge + Cloud voice assistant using ESP32, OLED display, Groq LLM, and browser-based microphone input.</b>
  </p>

  <hr />

  <h2>📌 Project Overview</h2>
  <p>
    This project is a real-time voice assistant built using an ESP32 and Groq’s ultra-fast LLM API. 
    Users connect to a local ESP32-hosted webpage through their phone hotspot—no microphone hardware required. 
    Audio is captured through the browser and streamed to the ESP32 over the local network, requiring only minimal cloud connectivity to query Groq.
  </p>

  <h2>✨ Key Features</h2>
  <ul>
    <li>🔊 Browser-based audio capture (no mic module required)</li>
    <li>⚡ Groq-powered real-time AI responses</li>
    <li>📡 ESP32-hosted local web server for seamless phone interaction</li>
    <li>🌐 Hybrid edge/cloud architecture for low-bandwidth environments</li>
    <li>📺 OLED display for quick visual feedback</li>
  </ul>

  <h2>🏗 Architecture</h2>
  <pre>
Phone → Local Hotspot → ESP32 Web App
   ↓ Audio Stream               ↑ Response
 Browser mic -----→ ESP32 -----→ Groq LLM Cloud
  </pre>

  <h2>🚀 How It Works</h2>
  <ol>
    <li>ESP32 hosts a local webpage.</li>
    <li>User connects via phone hotspot.</li>
    <li>Browser captures microphone audio.</li>
    <li>ESP32 processes + sends minimal data to Groq.</li>
    <li>Groq returns the AI response instantly.</li>
  </ol>

  <h2>📦 Installation</h2>
  <pre>
1. Clone the repository
2. Flash the ESP32 with the provided firmware
3. Connect to the ESP32 WiFi server
4. Open the web interface</pre>

</div>
