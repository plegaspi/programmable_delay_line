// Pin Definitions for 10-bit delay
#define DB0 23
#define DB1 25
#define DB2 27
#define DB3 29
#define DB4 31
#define DB5 33
#define DB6 35
#define DB7 37
#define DB8 39
#define DB9 41

// Pin Definitions for Device Select Lines
#define D0 22
#define D1 52
#define D2 53
#define D3 50

// Pin Definitions for Channel Addresses
#define A0 51
#define A1 49
#define A2 47
#define A3 45

// Pin Definitions for Write Enable
#define WE 43

int delay_pins[] = {DB0, DB1, DB2, DB3, DB4, DB5, DB6, DB7, DB8, DB9};
int delay_bits[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int addresses[] = {A0, A1, A2, A3};

int active_channel;
int device = 0;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  for (int bit = 0; bit < sizeof(delay_pins) / sizeof(delay_pins[0]); bit++)
  {
    pinMode(delay_pins[bit], OUTPUT);
    digitalWrite(delay_pins[bit], HIGH);
  }

  // Initialize addresses. Address A0 by default. Active HIGH
  pinMode(addresses[0], OUTPUT);
  digitalWrite(addresses[active_channel], HIGH);
  for (int address_index = 1; address_index < sizeof(addresses) / sizeof(addresses[0]); address_index++)
  {
    pinMode(addresses[address_index], OUTPUT);
    digitalWrite(addresses[address_index], LOW);
  }

  // Initialize devices. Device D0 by default. Active LOW
  pinMode(D0, OUTPUT);
  digitalWrite(D0, LOW);

  pinMode(D1, OUTPUT);
  digitalWrite(D1, HIGH);

  pinMode(D2, OUTPUT);  // New device
  digitalWrite(D2, HIGH);

  pinMode(D3, OUTPUT);  // New device
  digitalWrite(D3, HIGH);

  // Write enable. False by default. Active LOW
  pinMode(WE, OUTPUT);
  digitalWrite(WE, HIGH);
  Serial.println("Programmable Delay Line Serial Interface");
  Serial.print("PDL> ");
}

int parse_command(String command, String tokens[], int maxTokens)
{
  int tokenCount = 0;
  int startIndex = 0;
  int endIndex = 0;
  while (tokenCount < maxTokens && (endIndex = command.indexOf(" ", startIndex)) != -1)
  {
    tokens[tokenCount++] = command.substring(startIndex, endIndex);
    startIndex = endIndex + 1;
  }

  if (startIndex < command.length() && tokenCount < maxTokens)
  {
    tokens[tokenCount++] = command.substring(startIndex);
  }

  return tokenCount;
}

String commands[] = {"SET_DELAY", "SET_DEVICE", "SET_CHANNEL", "HELP", "TEST", "SET"};

void setDelay(int steps)
{
   if (steps < pow(2, 10))
   {
        // Set all address lines LOW first
        for (int address_index = 0; address_index < sizeof(addresses)/sizeof(addresses[0]); address_index++) {
            digitalWrite(addresses[address_index], LOW);
        }

        // Set the active channel HIGH
        Serial.print("     ");
        Serial.print("Setting channel to ");
        Serial.println(active_channel);
        Serial.print("Address");
        Serial.println(addresses[active_channel]);
        Serial.print("Active Channel");
        Serial.println(addresses[active_channel]);
        digitalWrite(addresses[active_channel], HIGH);


        // Set the delay bits
        for (int bit = 0; bit < sizeof(delay_bits) / sizeof(delay_bits[0]); bit++)
        {
            delay_bits[bit] = bitRead(steps, bit);
            if (delay_bits[bit] == 1)
            {
                digitalWrite(delay_pins[bit], HIGH);
            }
            else
            {
                digitalWrite(delay_pins[bit], LOW);
            }
        }

        digitalWrite(WE, LOW);
        delayMicroseconds(1);
        digitalWrite(WE, HIGH);

        // After writing, reset all address lines to LOW
        for (int address_index = 0; address_index < sizeof(addresses)/sizeof(addresses[0]); address_index++) {
            digitalWrite(addresses[address_index], LOW);
        }

        float delay_time = steps * 10.0;
        Serial.print("     ");
        Serial.print("Set the delay to ");
        Serial.print(delay_time);
        Serial.println(" ps");
   }
   else
   {
        Serial.println("You have exceeded the allowed step size");
   }
}


void setDevice(int device_line) {
  if (device_line == 0) {
    device = D0;
  } else if (device_line == 1) {
    device = D1;
  } else if (device_line == 2) {
    device = D2; 
  } else if (device_line == 3) {
    device = D3;  
  }

  // Set the selected device to LOW and others to HIGH
  digitalWrite(D0, device == D0 ? LOW : HIGH);
  digitalWrite(D1, device == D1 ? LOW : HIGH);
  digitalWrite(D2, device == D2 ? LOW : HIGH);
  digitalWrite(D3, device == D3 ? LOW : HIGH);

  Serial.print("     ");
  Serial.print("Set device to D");
  Serial.println(device_line);
}

void setChannel(int address) {
   active_channel = address;
}

void set(int device, int channel, int delay) 
{
  setDevice(device);
  setChannel(channel);
  setDelay(delay);
}

void loop()
{
  if (Serial.available() > 0)
  {
	String command_input = Serial.readString();
	command_input.trim();  
	Serial.println(command_input);

	const int max_tokens = 10;
	String tokens[max_tokens];
	int tokenCount = parse_command(command_input, tokens, max_tokens);

	if (tokenCount > 0)
	{
  	String command = tokens[0];
  	bool isValidCommand = false;

  	// Check if command exists in the list of valid commands
  	for (int command_index = 0; command_index < sizeof(commands) / sizeof(commands[0]); command_index++)
  	{
    	if (command.equals(commands[command_index]))
    	{
      	isValidCommand = true;
      	break;
    	}
  	}

  	if (isValidCommand)
  	{
    	if (command.equals("SET_DELAY"))
    	{
      	setDelay(tokens[1].toInt());
    	}
    	else if (command.equals("SET_DEVICE"))
    	{
      	setDevice(tokens[1].toInt());
    	}
    	else if (command.equals("SET_CHANNEL"))
    	{
      	setChannel(tokens[1].toInt());
    	}
    	else if (command.equals("SET"))
    	{
      	set(tokens[1].toInt(), tokens[2].toInt(), tokens[3].toInt());
    	}
    	else if (command.equals("HELP"))
    	{
      	// Print the help information
      	Serial.println("Available commands:");
      	Serial.println("SET_DELAY <steps> - Set the delay of the active delay line.");
      	Serial.println("SET_DEVICE <device> - Set the active device (0 to 3).");
      	Serial.println("SET_CHANNEL <channel> - Set the channel address (0 to 3).");
      	Serial.println("SET <device> <channel> <steps> - Set the device, channel, and delay in a single command");
      	Serial.println("HELP - Show this help message.");
    	}
  	}
  	else
  	{
    	Serial.print(" 	");
    	Serial.print(command);
    	Serial.println(" is an invalid command. Type 'HELP' into the Serial Monitor for a list of valid commands.");
  	}
	}
	Serial.println();
	Serial.print("PDL> ");
  }
}