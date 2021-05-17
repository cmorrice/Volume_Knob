#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <BleKeyboard.h>

//networking constants
const char* ssid = "Fios-KpG3D";
const char* password = "buzz43hid63beep";

//software constants
const int notFound = -1;
const String messageStart = "<html><head> <title>Bluetooth + WiFi Server</Title> </head><body style='font-family: sans-serif; font-size: 12px'> <h3>";
const String messageEnd = "</body> </html>";

//software variables
bool aState;
bool aLastState;
bool bState;
bool buttonState;
bool buttonLastState = false;

//hardware constants
const int8_t led = 13;
const int8_t aPin = 12;
const int8_t bPin = 14;
const int8_t buttonPin = 27;
const int8_t vibrationMotorPin = 25;

//hardware objects
WebServer server(80);
BleKeyboard bleKeyboard;

/////////////////////////////
//////////functions//////////
/////////////////////////////
String getArgValue(String name)
{
    for (uint8_t i = 0; i < server.args(); i++)
    if(server.argName(i) == name)
    {
        String temp = "argument: " + name + " " + server.arg(i);
        Serial.println(temp);
        return server.arg(i);
    }
    return String(notFound);
}

void handleRoot()
{
    digitalWrite(led, 1);
    String message = messageStart + "Following functions are available:</h3>";
    message += "<li><a href='/playPause'>/playPause</a> hits the play/pause button</li>";
    message += "<li><a href='/volumeUp'>/volumeUp</a> increases the volume by 10</li>";
    message += "<li><a href='/volumeDown'>/volumeDown</a> decreases the volume by 10</li>";
    message += "<li><a href='/changeVolume?change='>/changeVolume</a> changes the volume by the argument <em>change=integer amount</em></li>";
    message += "<li><a href='/thisiswrong'>/thisiswrong</a> this will take you to a none-existent page<br></li></ul>";
    message += "<p>Syntax is as follows: http://192.168.1.177/<strong>command</strong>?<strong>argument1</strong>=<strong>value1</strong>&<strong>argument2</strong>=<strong>value2</strong>&...</p> </body> </html>";
    server.send(200, "text/html", message);
    digitalWrite(led, 0);
}

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
    for (uint8_t i = 0; i < server.args(); i++)
    {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
    digitalWrite(led, 0);
}

/////////////////////////////
///bluetooth based handles///
/////////////////////////////
void playPauseHandle()
{
    digitalWrite(led, 1);
    bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
    String message = messageStart + "Play/Pause Function</h3>";
    message += "<p>Play/Pause hit<p/>";
    
    yeet(message);
}

void volumeUpHandle()
{
    digitalWrite(led, 1);
    for (int8_t f = 0; f < 5; f++)
    {
        bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
    }
    
    String message = messageStart + "Volume Up Function</h3>";
    message += "<p>Volume increased by 10<p/>";
    
    yeet(message);
}

void volumeDownHandle()
{
    digitalWrite(led, 1);
    for (int8_t f = 0; f < 5; f++)
    {
        bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
    }
    
    String message = messageStart + "Volume Down Function</h3>";
    message += "<p>Volume decreased by 10<p/>";
    
    yeet(message);
}

void changeVolumeHandle()
{
    digitalWrite(led, 1);
    String message = messageStart + "changeVolume Function</h3>";
    int8_t change = getArgValue("change").toInt();
    if ((change != notFound) && (change <= 100 && change >= -100))
    {
        if (change < 0)
        {
            for (int8_t f = 0; f > change/2; f--)
            {
                Serial.print("f = ");
                Serial.println(f);
                Serial.print("change = ");
                Serial.println(change);
                Serial.print("change/2 = ");
                Serial.println(change/2);
                bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
                delay(8);
            }
        }
        else
        {
            for (int8_t f = 0; f < change/2; f++)
            {
                Serial.print("f = ");
                Serial.println(f);
                Serial.print("change = ");
                Serial.println(change);
                Serial.print("change/2 = ");
                Serial.println(change/2);
                bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
                delay(8);
            }
        }
        message += "<p>Volume changed by " + String(change) + "<p/>";
    }
    else
    {
        message += "<p>To change the volume use this after the changeVolume handle: changeVolume?<em>change=<strong>integer of change amount</strong></em>";
    }
    
    yeet(message);
}

/////////////////////////////
//////helper functions///////
/////////////////////////////
//yeet takes the message, caps it off with the end message, and yeets it to the server
void yeet(String message)
{
    message += messageEnd;
    server.send(200, "text/html",message);
    digitalWrite(led, 0);  
}


/////////////////////////////
///////set up and loop///////
/////////////////////////////
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
    for (int f = 0; f < 10; f++)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
            if (f == 9)
            {
                Serial.print("WiFi can't connect... moving on");
            }
        }
        else
        {
            f = 10;
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
    Serial.println("Rotary encoder initialized");
}

void loop(void)
{
    //handles the potential internet control
    server.handleClient();
    //Serial.println("server has been handled");
    
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
    
    /*
    if (aState != aLastState)
    {
        digitalWrite(vibrationMotorPin, HIGH);
        if (digitalRead(bPin) == aState)
        {
            counter++;
            bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
            Serial.println("Volume Up Sent");
        }
        else
        {
            counter--;
            bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
            Serial.println("Volume Down Sent");
        }
        digitalWrite(vibrationMotorPin, LOW);
    }
    */
    
    aLastState = aState;
    //push button//
    //always shows true while nothing is plugged in so comment out when not plugged in
    buttonState = digitalRead(buttonPin);
    if (buttonState)
    {
        if (!buttonLastState)
        {
            digitalWrite(vibrationMotorPin, HIGH);
            bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
        }
    }
    else
    {
        if (buttonLastState)
        {
            digitalWrite(vibrationMotorPin, LOW);
        }
    }
    buttonLastState = buttonState;
}
