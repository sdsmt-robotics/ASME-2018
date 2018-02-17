//Recieving byte
byte incomingByte = 0;

//Storage for bytes for decoding
byte incoming_command[2];
int queue_len = 0;

//storage for values being sent [button / joy] [value]
//97-100 are joystick values
//101-108 are button values
int current_vals[12][2];

int axis = 0;
//Distinguishes from button / joystick commands
void check_command()
{
  //incoming_command[0] is the encoded value being converted to decimal
  if(incoming_command[0] <= 100)
  {
    axis = incoming_command[0];
    current_vals[(axis - 97)][0] = axis;
    current_vals[(axis - 97)][0] = map(incoming_command[1], 0, 200, -100, 100);
    Serial.print("Joystick ");
    Serial.print(incoming_command[0], HEX);
    Serial.print(" ");
    Serial.print(map(incoming_command[1], 0, 200, -100, 100));
    Serial.println();
  }
}

//houses the code for driving the robot
void drive()
{
  
}
void setup() {
  Serial1.begin(9600);
  Serial.begin(9600);

  Serial.println("Starting Recieve Code");
}

void loop() {
  if (Serial1.available() > 0)
  {
    incomingByte = Serial1.read();
    if (incomingByte == 0x58)
    {
      Serial.println();
      check_command();
      drive()
      queue_len = 0;
    } 
    else 
    {
      incoming_command[queue_len] = incomingByte;
      queue_len++;
      Serial.print(incomingByte, HEX);
      Serial.print(" ");
    }
   
    //Serial.println(incomingByte, HEX);
  }

}