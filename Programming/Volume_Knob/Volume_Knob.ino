/**
 * Volume_Knob
 * Author: Camilo Morrice
 */
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <BleKeyboard.h>

#define NOT_FOUND -1

// networking constants
const char* ssid = "Fios-KpG3D";
const char* password = "buzz43hid63beep";

// hardware constants
const uint8_t led = 13;
const uint8_t aPin = 27;
const uint8_t bPin = 26;
const uint8_t buttonPin = 25;
const uint8_t vibrationMotorPin = 33;

// software variables
bool aState;
bool aLastState;
bool bState;
bool buttonState;
bool buttonLastState;

// hardware objects
WebServer server(80);
BleKeyboard bleKeyboard;

/** helpers
 * getArgValue() will return the argument from the URL
 * sendPage() will send the html webpage
 */
String getArgValue(String name)
{
    for (uint8_t index = 0; index < server.args(); index++) // loops through server arguments
    {
        if(server.argName(index) == name) // if argument is found
        {
            String temp = "argument: " + name + " " + server.arg(index);
            Serial.println(temp);
            return server.arg(index);
        }
    }
    return String(NOT_FOUND); 
} // getArgValue()

void printArgs()
{
    for (uint8_t index = 0; index < server.args(); index++) // loops through server arguments
    {
        String temp = "argument: " + server.argName(index) + " " + server.arg(index);
        Serial.println(temp);
    }
}

void sendPage()
{
    printArgs();
    String webPage = "<html><head><title>Bluetooth + WiFi Server</Title><style>html { font-family: sans-serif; font-size: 18px; text-align: center; background-color: #eee;}p { font-size: 24px; color: #555555 }label { font-size: 24px; color: #555555}.button { background-color: #0DBAB1; border: none; border-radius: 12px; color: white; padding: 16px 40px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer; }.button:hover { background-color: #12ddd3;}";
    String webPage2 = ".button2 { background-color: #555555; border: none; border-radius: 12px; color: white; padding: 16px 40px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer; }</style></head><body><!-- Volume Up Button --><p> Volume Up </p><p> <a href = \"/?volumeUp=1\"> <button class = \"button\"> Volume Up </button> </a> </p><hr><!-- Volume Down Button --><p> Volume Down </p><p> <a href = \"/volumeDown\"> <button class = \"button\"> Volume Down </button> </a> ";
    String webPage3 = "</p><hr><!-- Play/Pause Button --><p> Play/Pause </p><p> <a href = \"/playPause\"> <button class = \"button\"> Play/Pause </button> </a> </p><hr><section class = \"change\"><form action=\"/changeVolume\" method = \"POST\"> <label for=\"volumeInput\">Change Volume</label><br><input id=\"volumeInput\" name=\"amount\" type=\"number\" min=\"-100\" max=\"100\" step = \"2\" required><br><span>-100</span><input id=\"volumeSlider\" type=\"range\" min=\"-100\" max=\"100\" step = \"2\" onchange = \"updateTextInput(this.value);\"><span>100</span><script type = \"text/javascript\">function updateTextInput(val){document.getElementById('volumeInput').value = val;}</script><br><input type=\"submit\" value = \"change\"></form></section></body></html>";
    server.setContentLength(webPage.length() + webPage2.length() + webPage3.length());
    server.send(200, "text/html", webPage);
    server.sendContent(webPage2);
    server.sendContent(webPage3);
} // sendPage()

/** server handles
 * handRoot() will handle the root directory
 * handleNotFound() will send an error page when the handle is not found
 */
void handleRoot()
{
    digitalWrite(led, 1);
    sendPage();
    delay(100);
    digitalWrite(led, 0);
} // handleRoot()

void handleNotFound()
{
    digitalWrite(led, 1);
    String message = "that was a really dumb url... why would it exist\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t index = 0; index < server.args(); index++)
    {
        message += " " + server.argName(index) + ": " + server.arg(index) + "\n";
    }
    server.send(404, "text/plain", message);
    digitalWrite(led, 0);
} // handleNotFound()

/** bluetooth based server handles
 * playPauseHandle() will send the play/pause key
 * volumeUpHandle() will send 5 volume up keys
 * volumeDownHandle() will send 5 volume down keys
 * changeVolumeHandle() will send the given amount of volume up/down keys
 */
void playPauseHandle()
{
    digitalWrite(led, 1); // hello
    bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
    sendPage();
    digitalWrite(led, 0);
} // playPauseHandle()

void volumeUpHandle()
{
    digitalWrite(led, 1);
    for (int8_t f = 0; f < 5; f++)
    {
        bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
    }
    
    sendPage();
    digitalWrite(led, 0);
} // volumeUpHandle()

void volumeDownHandle()
{
    digitalWrite(led, 1);
    for (int8_t f = 0; f < 5; f++)
    {
        bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
    }
    
    sendPage();
    digitalWrite(led, 0);
} // volumeDownHandle()

void changeVolumeHandle()
{
    digitalWrite(led, 1);
    int8_t change = getArgValue("amount").toInt();
    if ((change != NOT_FOUND && change != 0) && (change <= 100 && change >= -100)) // if change is found, not zero, and within bounds
    {
        Serial.print("change = ");
        Serial.println(change);
        Serial.print("change/2 = ");
        Serial.println(change/2);
        change = change / 2; // since windows increases by increments of 2
        if (change > 0) // if volume up
        {
            for (int8_t f = 0; f < change; f++)
            {
                bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
                delay(8);
            }
        }
        else // if volume down
        {
            for (int8_t f = 0; change < f; f--)
            { 
                bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
                delay(8);
            }
        }
    }

    sendPage();
    digitalWrite(led, 0);
} // changeVolumeHandle()

/** setup and loop
 * setup() will initialize the volume knob
 * loop() will continuously check the server and hardware knob for input
 */
void setup(void)
{
    /////////////////////////////
    /////////WiFi Stuff//////////
    /////////////////////////////
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("");
    
    // Wait for connection
    for (int attempt = 0; attempt < 10; attempt++)
    {
        if (WiFi.status() != WL_CONNECTED) // if connection is not made
        {
            Serial.print(".");
            if (attempt == 9) // after 10th attempt
            {
                Serial.print("WiFi can't connect... moving on");
            }
            delay(500);
        }
        else
        {
            attempt = 10; // break out if connected
        }
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    if (MDNS.begin("esp32")) 
    {
        Serial.println("MDNS responder started");
    }
    
    //tells the server what to do for each root
    server.on("/", handleRoot);
    
    server.on("/playPause", playPauseHandle);
    
    server.on("/volumeUp", volumeUpHandle);
    
    server.on("/volumeDown", volumeDownHandle);
    
    server.on("/changeVolume", changeVolumeHandle);
    
    server.onNotFound(handleNotFound); //this is what happens when handle is not found
    
    //starts the server
    server.begin();
    Serial.println("HTTP server started");
    
    /////////////////////////////
    ///////Bluetooth Stuff///////
    /////////////////////////////
    bleKeyboard.begin();
    Serial.println("Bluetooth enabled");
    
    /////////////////////////////
    ///////Hardware Stuff////////
    /////////////////////////////
    pinMode(aPin, INPUT);
    pinMode(bPin, INPUT);
    pinMode(buttonPin, INPUT);
    pinMode(vibrationMotorPin, OUTPUT);
    aLastState = digitalRead(aPin);
    buttonLastState = digitalRead(buttonPin);
    Serial.println("Rotary encoder initialized");
}

void loop(void)
{
    //handles the potential internet control
    server.handleClient();

    //handles the physical rotary encoder//
    //volume up and down//
    aState = digitalRead(aPin);
    bState = digitalRead(bPin);
    if (!aState && aState != aLastState)
    {
        digitalWrite(vibrationMotorPin, HIGH);
        if (!bState)
        {
            bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
            Serial.println("Volume Up Sent");
        }
        else
        {
            bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
            Serial.println("Volume Down Sent");
        }
        digitalWrite(vibrationMotorPin, LOW);
    }
    aLastState = aState;
    
    //push button//
    buttonState = digitalRead(buttonPin); // reads LOW when pressed and HIGH when not
    if (buttonState == LOW) // if button is pressed
    {
        if (buttonLastState == HIGH) // if button was previously not pressed
        {
            digitalWrite(vibrationMotorPin, HIGH); // turn on vibration
            bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE); // send play/pause
            Serial.println("Play/Pause Sent");
        }
    }
    else // if button is not pressed
    {
        if (buttonLastState == LOW) // if button was previously pressed
        {
            digitalWrite(vibrationMotorPin, LOW); // turn off vibration
            delay(20);
        }
    }
    buttonLastState = buttonState;
}
