// ---------- Libraries ----------
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------- WiFi & GROQ ----------
const char* WIFI_SSID = "Add your Wifi SSID here";
const char* WIFI_PASS = "Add your Wifi password here";
String GROQ_API_KEY = "Add your GROQ API key here";
const char* GROQ_ENDPOINT = "https://api.groq.com/openai/v1/chat/completions";
String GROQ_MODEL = "llama-3.1-8b-instant";

WebServer server(80);

// ---------- LEDs ----------
const uint8_t GREEN_LED_PIN = 14; // idle
const uint8_t RED_LED_PIN   = 13; // busy

// ---------- Greetings ----------
const char* greetings[] = {
  "Hey! What's up? How can I help?",
  "Yo! Your GROQ buddy is here!",
  "Hey hey! Ask me anything!",
  "Hey, I’m your GROQ assistant. How can I help?",
  "Hi! I’m here to assist you with anything.",
  "Hello! Feel free to ask me anything."
};
const int NUM_GREETINGS = sizeof(greetings)/sizeof(greetings[0]);

// ---------- Utility ----------
void centerText(const String &txt, int y) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(txt, 0, y, &x1, &y1, &w, &h);
  int x = max(0, (SCREEN_WIDTH - (int)w) / 2);
  display.setCursor(x, y);
  display.println(txt);
}

// ---------- Hacker Animation ----------
void hackerAnimation(int duration_ms) {
  unsigned long endAt = millis() + duration_ms;
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  while (millis() < endAt) {
    display.clearDisplay();
    for (int i = 0; i < 8; i++) {
      String line = "";
      for (int j = 0; j < 16; j++) {
        char c = random(33, 126); // printable ASCII
        line += c;
      }
      display.setCursor(0, i*8);
      display.println(line);
    }
    display.display();
    delay(100);
  }
}

// ---------- Intro Animation ----------
void introAnimation() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  for(int i=0;i<SCREEN_HEIGHT/4;i++){
    display.fillRect(0, SCREEN_HEIGHT-i*4, SCREEN_WIDTH, 4, SSD1306_WHITE);
    display.display();
    delay(60);
  }
  delay(300);
  display.clearDisplay();
}

// ---------- Fade-in Greeting ----------
void fadeInGreeting(const String &line) {
  for (int step = 0; step < 3; ++step) {
    display.clearDisplay();
    centerText("GROQ AI Assistant", 0);
    centerText(line, 22);
    display.display();
    delay(200);
  }
  // Final
  display.clearDisplay();
  centerText("GROQ AI Assistant", 0);
  centerText(line, 22);
  display.display();
}

// ---------- Thinking Animation ----------
void thinkingAnimation(int duration_ms) {
  const int y = SCREEN_HEIGHT - 14;
  unsigned long endAt = millis() + duration_ms;
  int frame = 0;
  while(millis() < endAt){
    display.fillRect(0, y, SCREEN_WIDTH, 14, SSD1306_BLACK);
    for(int i=0;i<3;i++){
      int x = 36 + i*20;
      int h = 4 + ((frame + i) % 3)*4;
      display.fillRect(x, y + (14 - h)-2, 6, h, SSD1306_WHITE);
    }
    display.display();
    frame++;
    delay(200);
  }
}

// ---------- Cursor Blinking AI Response ----------
void cursorBlinkReveal(const String &text){
  const int lineHeight = 10;
  const int maxChars = 22;
  const int startY = 14;
  int y = startY;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  centerText("GROQ AI Assistant",0);
  display.display();

  String lines[SCREEN_HEIGHT/lineHeight];
  int lineCount = 0;
  String curLine = "";

  for(size_t i=0;i<text.length();i++){
    char c = text[i];
    if(c=='\r') continue;
    curLine += c;

    bool commit = (c=='\n' || curLine.length()>=maxChars || i==text.length()-1);

    if(commit){
      // Scroll lines if needed
      if(y > SCREEN_HEIGHT - lineHeight){
        for(int l=0;l<lineCount-1;l++) lines[l]=lines[l+1];
        lines[lineCount-1] = curLine;
      } else {
        lines[lineCount] = curLine;
        lineCount++;
        y += lineHeight;
      }

      // Display lines with cursor
      for(int reveal=0;reveal<=curLine.length();reveal++){
        display.clearDisplay();
        centerText("GROQ AI Assistant",0);
        int drawY=startY;
        int startLine = max(0,lineCount-(SCREEN_HEIGHT-startY)/lineHeight);
        for(int l=startLine;l<lineCount;l++){
          display.setCursor(0, drawY);
          if(l==lineCount-1){
            display.print(curLine.substring(0,reveal));
            display.print('_'); // underscore cursor
          } else {
            display.println(lines[l]);
          }
          drawY+=lineHeight;
        }
        display.display();
        delay(40);
      }
      curLine="";
    }
  }
}

// ---------- GROQ API Call ----------
String callGroq(const String &question){
  HTTPClient http;
  http.begin(GROQ_ENDPOINT);
  http.addHeader("Content-Type","application/json");
  http.addHeader("Authorization","Bearer "+GROQ_API_KEY);

  String q = question;
  q.replace("\\","\\\\");
  q.replace("\"","\\\"");

  String payload = "{\"model\":\""+GROQ_MODEL+"\",\"messages\":[{\"role\":\"user\",\"content\":\""+q+"\"}],\"max_tokens\":256}";
  int code = http.POST(payload);
  if(code<200 || code>=300){
    http.end();
    return "API Error "+String(code);
  }
  String body = http.getString();
  http.end();

  StaticJsonDocument<12000> doc;
  DeserializationError derr = deserializeJson(doc, body);
  if(derr) return "Parse Error";

  String ans="";
  if(doc.containsKey("choices") && doc["choices"].size()>0){
    if(doc["choices"][0].containsKey("message") && doc["choices"][0]["message"].containsKey("content"))
      ans = doc["choices"][0]["message"]["content"].as<String>();
    else if(doc["choices"][0].containsKey("text"))
      ans = doc["choices"][0]["text"].as<String>();
  }
  if(ans.length()==0) ans="No reply";
  return ans;
}

// ---------- Web UI ----------
String webPage(){
  return R"rawliteral(
<!doctype html>
<html>
  <head>
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <style>
      body{background:#0d0d0d;color:#fff;font-family:Arial,Helvetica,sans-serif;text-align:center;padding:30px}
      .card{max-width:420px;margin:0 auto;background:rgba(255,255,255,0.03);padding:24px;border-radius:12px}
      button{background:#1f6feb;border:none;color:#fff;padding:16px 26px;border-radius:10px;font-size:18px}
      #status{margin-top:12px;color:#bbb}
    </style>
  </head>
  <body>
    <div class="card">
      <h2>ESP32 Voice Assistant</h2>
      <button id="btn">Speak</button>
      <div id="status">Ready</div>
    </div>
    <script>
      const btn = document.getElementById('btn'), status = document.getElementById('status');
      const Rec = window.SpeechRecognition || window.webkitSpeechRecognition;
      if(!Rec){ status.innerText = 'Speech not supported'; btn.disabled = true; }
      else {
        const rec = new Rec();
        rec.lang = 'en-US';
        rec.interimResults = false;
        btn.onclick = ()=>{ status.innerText='Listening...'; rec.start(); };
        rec.onresult = (e)=> {
          const txt = e.results[0][0].transcript;
          status.innerText = 'Processing...';
          fetch('/ask?text=' + encodeURIComponent(txt)).then(()=>{ status.innerText='Sent'; setTimeout(()=>status.innerText='Ready',800); }).catch(()=>status.innerText='Network error');
        };
        rec.onend = ()=> { if(status.innerText==='Listening...') status.innerText='Ready'; };
      }
    </script>
  </body>
</html>
)rawliteral";
}

// ---------- Handlers ----------
void handleRoot(){ server.send(200,"text/html",webPage()); }

void handleAsk(){
  if(!server.hasArg("text")){ server.send(400,"text/plain","Missing text"); return;}
  String question = server.arg("text");

  // Busy: RED ON
  digitalWrite(RED_LED_PIN,HIGH);
  digitalWrite(GREEN_LED_PIN,LOW);

  // Thinking animation
  display.clearDisplay();
  centerText("GROQ AI Assistant", SCREEN_HEIGHT/2 - 20);
  display.display();
  thinkingAnimation(1500);

  // Cursor blinking response
  cursorBlinkReveal(callGroq(question));

  // Done: GREEN ON
  digitalWrite(RED_LED_PIN,LOW);
  digitalWrite(GREEN_LED_PIN,HIGH);

  server.send(200,"text/plain","OK");
}

// ---------- Setup ----------
void setup(){
  Serial.begin(115200);
  pinMode(GREEN_LED_PIN,OUTPUT);
  pinMode(RED_LED_PIN,OUTPUT);

  digitalWrite(GREEN_LED_PIN,LOW);
  digitalWrite(RED_LED_PIN,HIGH);

  if(!display.begin(SSD1306_SWITCHCAPVCC,0x3C)){while(1) delay(10);}
  display.clearDisplay();

  // Connect WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long start = millis();
  while(WiFi.status() != WL_CONNECTED && millis()-start<20000){ delay(300); }

  // Startup sequence: RED ON
  digitalWrite(RED_LED_PIN,HIGH);
  digitalWrite(GREEN_LED_PIN,LOW);

  hackerAnimation(3500);
  introAnimation();

  // Random greeting
  randomSeed(analogRead(34));
  int gi=random(NUM_GREETINGS);
  fadeInGreeting(String(greetings[gi]));

  // Idle GREEN ON
  digitalWrite(RED_LED_PIN,LOW);
  digitalWrite(GREEN_LED_PIN,HIGH);

  // Start server
  server.on("/",handleRoot);
  server.on("/ask",handleAsk);
  server.begin();
  Serial.print("Server started. IP: "); Serial.println(WiFi.localIP());
}

// ---------- Loop ----------
void loop(){ server.handleClient(); }