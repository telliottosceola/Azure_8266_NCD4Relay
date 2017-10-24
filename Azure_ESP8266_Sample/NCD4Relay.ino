//Comment line below out to turn off Serial logging
#define LOGGING

#include <Wire.h>

//User Accessible Variables
//Whether or not the controller is ready to accept commands
bool initialized;
int bankOneStatus;

byte address = 0x20;
byte address2 = 0x21;
int retrys = 0;
byte outputRegister = 0x0A;

void parseStringCommand(char* command, int commandLength) {
  String commandString(command);
  
  if (commandString.equalsIgnoreCase("turnonallrelays")) {
    turnOnAllRelays();
  }
  if (commandString.equalsIgnoreCase("turnoffallrelays")) {
    turnOffAllRelays();
  }
  if (commandString.startsWith("setBankStatus:")) {
    int status = commandString.substring(14).toInt();
    if (status < 0 || status > 255) {
    }
    Serial.print("Setting bank status to: ");
    Serial.println(status);
    setBankStatus(status);
    Serial.println("done");
  }
  //Relay Specific Command
  int relayNumber = commandString.substring(0, 1).toInt();
  Serial.print("relayNumber: ");
  Serial.println(relayNumber);
  String relayCommand = commandString.substring(1);
  Serial.print("relayCommand:");
  Serial.print(relayCommand);
  Serial.println(".");
  if (relayCommand.equalsIgnoreCase("on")) {
    Serial.println("Turning on relay");
    turnOnRelay(relayNumber);
  }
  if (relayCommand.equalsIgnoreCase("off")) {
    turnOffRelay(relayNumber);
  }
  if (relayCommand.equalsIgnoreCase("toggle")) {
    toggleRelay(relayNumber);
  }
  if (relayCommand.equalsIgnoreCase("momentary")) {
    turnOnRelay(relayNumber);
    delay(300);
    turnOffRelay(relayNumber);
  }
}

//Retry added
void setAddress(int a0, int a1, int a2) {
  address = 0x20;
  if (a0 == 1) {
    address = address | 1;
  }
  if (a1 == 1) {
    address = address | 2;
  }
  if (a2 == 1) {
    address = address | 4;
  }
  //Start I2C port
  Wire.begin(12, 14);
  //Open connection to specified address
retryAddress1:
  Wire.beginTransmission(address);
  //Set all channels to outputs
  Wire.write(0x00);
  Wire.write(0xF0);
  //Determine if device is present at that address
  byte status = Wire.endTransmission();

  Wire.beginTransmission(address);
  Wire.write(0x06);
  Wire.write(0xF0);
  status = Wire.endTransmission();
  if (status != 0) {
    Serial.printf("I2C end transmission returned %i\n", status);
    if (retrys < 3) {
#ifdef LOGGING
      Serial.println("Retry set address command");
#endif
      retrys++;
      goto retryAddress1;
    } else {
#ifdef LOGGING
      Serial.println("Set Address failed");
#endif
      initialized = false;
      retrys = 0;
    }

  } else {
    // Serial.println("Command Successful");
    initialized = true;
    retrys = 0;
    readStatus();
  }
}

void turnOnRelay(int relay) {
  if (relay > 4 || relay < 0) {
    return;
  }
  byte bankValue = bankOneStatus;
  byte registerAddress = 0x0A;
  switch (relay) {
    case 1:
      bankValue = bankValue | 1;
      break;
    case 2:
      bankValue = bankValue | 2;
      break;
    case 3:
      bankValue = bankValue | 4;
      break;
    case 4:
      bankValue = bankValue | 8;
      break;
  }
turnOnRelayRetry:
  Wire.beginTransmission(address);
  Wire.write(registerAddress);
  Wire.write(bankValue);
  byte status = Wire.endTransmission();
  if (status != 0) {
    if (retrys < 3) {
#ifdef LOGGING
      Serial.println("Retry Turn On Relay command");
#endif
      retrys++;
      goto turnOnRelayRetry;
    } else {
#ifdef LOGGING
      Serial.println("Turn On Relay failed");
#endif
      initialized = false;
      retrys = 0;
    }
  } else {
    initialized = true;
    retrys = 0;
    readStatus();
  }
}

void turnOffRelay(int relay) {
  if (relay > 4 || relay < 0) {
    return;
  }
  byte bankValue = bankOneStatus;
  byte registerAddress = 0x0A;
  switch (relay) {
    case 1:
      bankValue = bankValue & ~1;
      break;
    case 2:
      bankValue = bankValue & ~2;
      break;
    case 3:
      bankValue = bankValue & ~4;
      break;
    case 4:
      bankValue = bankValue & ~8;
      break;
  }
turnOffRelaysRetry:
  Wire.beginTransmission(address);
  Wire.write(registerAddress);
  Wire.write(bankValue);
  byte status = Wire.endTransmission();
  if (status != 0) {
    if (retrys < 3) {
#ifdef LOGGING
      Serial.println("Retry Turn Off Relay command");
#endif
      retrys++;
      goto turnOffRelaysRetry;
    } else {
#ifdef LOGGING
      Serial.println("Turn Off Relay command failed");
#endif
      initialized = false;
      retrys = 0;
    }
  } else {
    initialized = true;
    retrys = 0;
    readStatus();
  }
}

void turnOnAllRelays() {
turnOnAllRelays1:
  Wire.beginTransmission(address);
  Wire.write(0x0A);
  Wire.write(0x0F);
  byte status = Wire.endTransmission();
  if (status != 0) {
    if (retrys < 3) {
#ifdef LOGGING
      Serial.println("Retry turn off all relays banks 1-2 command");
#endif
      retrys++;
      goto turnOnAllRelays1;
    } else {
#ifdef LOGGING
      Serial.println("Turn off all relays banks 1-2 command failed");
#endif
      initialized = false;
      retrys = 0;
    }
  } else {
    initialized = true;
    retrys = 0;
    readStatus();
  }
}

void turnOffAllRelays() {
turnOffAllRelaysRetry1:
  Wire.beginTransmission(address);
  Wire.write(0x0A);
  Wire.write(0);
  byte status = Wire.endTransmission();
  if (status != 0) {
    if (retrys < 3) {
#ifdef LOGGING
      Serial.println("Retry turn off all relays banks 1-2 command");
#endif
      retrys++;
      goto turnOffAllRelaysRetry1;
    } else {
#ifdef LOGGING
      Serial.println("Turn off all relays banks 1-2 command failed");
#endif
      initialized = false;
      retrys = 0;
    }
  } else {
    initialized = true;
    retrys = 0;
    readStatus();
  }
}

void toggleRelay(int relay) {
  if (relay > 4 || relay < 0) {
    return;
  }
  byte bankValue =  bankOneStatus;
  switch (relay) {
    case 1:
      bankValue = bankValue ^ 1;
      break;
    case 2:
      bankValue = bankValue ^ 2;
      break;
    case 3:
      bankValue = bankValue ^ 4;
      break;
    case 4:
      bankValue = bankValue ^ 8;
      break;
  }
toggleRelayRetry:
  Wire.beginTransmission(address);
  Wire.write(0x0A);
  Wire.write(bankValue);
  byte status = Wire.endTransmission();
  if (status != 0) {
    if (retrys < 3) {
#ifdef LOGGING
      Serial.println("Retry toggle relay command");
#endif
      retrys++;
      goto toggleRelayRetry;
    } else {
#ifdef LOGGING
      Serial.println("Toggle relay command failed");
#endif
      initialized = false;
      retrys = 0;
    }
  } else {
    initialized = true;
    retrys = 0;
    readStatus();
  }
}

void setBankStatus(int status) {
setBankStatusRetry:
  Wire.beginTransmission(address);
  Wire.write(0x0A);
  Wire.write(status);
  byte s = Wire.endTransmission();
  if (s != 0) {
    if (retrys < 3) {
#ifdef LOGGING
      Serial.println("Retry set bank status command");
#endif
      retrys++;
      goto setBankStatusRetry;
    } else {
#ifdef LOGGING
      Serial.println("Set bank status command failed");
#endif
      initialized = false;
      retrys = 0;
    }
  } else {
    initialized = true;
    retrys = 0;
    readStatus();
  }
}

int readRelayStatus(int relay) {
  if (relay > 4 || relay < 0) {
    return 256;
  }
  int value;
  switch (relay) {
    case 1:
      value = 1;
      break;
    case 2:
      value = 2;
      break;
    case 3:
      value = 4;
      break;
    case 4:
      value = 8;
      break;
  }
getRelayStatusRetry:
  Wire.beginTransmission(address);
  Wire.write(0x0A);
  byte status = Wire.endTransmission();
  if (status != 0) {
    if (retrys < 3) {
#ifdef LOGGING
      Serial.println("Retry read relay status command");
#endif
      retrys++;
      goto getRelayStatusRetry;
    } else {
#ifdef LOGGING
      Serial.println("Read relay status command failed");
#endif
      initialized = false;
      retrys = 0;
      return 256;
    }
  } else {
    retrys = 0;
    initialized = true;
  }
  Wire.requestFrom(address, 1);
  byte bankStatus = Wire.read();
  if (bankStatus & value) {
    return 1;
  } else {
    return 0;
  }
}

int readRelayBankStatus() {
readBankStatusRetry:
  Wire.beginTransmission(address);
  Wire.write(0x0A);
  byte status = Wire.endTransmission();
  if (status != 0) {
    if (retrys < 3) {
#ifdef LOGGING
      Serial.println("Retry read bank status command");
#endif
      retrys++;
      goto readBankStatusRetry;
    } else {
#ifdef LOGGING
      Serial.println("Read bank status command failed");
#endif
      initialized = false;
      retrys = 0;
      return 256;
    }
  } else {
    retrys = 0;
    initialized = true;
  }
  Wire.requestFrom(address, 1);
  byte bankStatus = Wire.read();
  return bankStatus;
}

void readStatus() {
readBankOneRetry:
  //Open Connection to controller
  Wire.beginTransmission(address);
  //Get current status of relays 1-8
  Wire.write(0x0A);
  byte status = Wire.endTransmission();
  if (status != 0) {
    if (retrys < 3) {
#ifdef LOGGING
      Serial.println("Retry read status banks 1-2 command");
#endif
      retrys++;
      goto readBankOneRetry;
    } else {
#ifdef LOGGING
      Serial.println("Read ststus banks 1-2 command failed");
#endif
      initialized = false;
      retrys = 0;
    }
  } else {
    retrys = 0;
    initialized = true;
    Wire.requestFrom(address, 1);
    bankOneStatus = Wire.read();
  }

}

int readInputStatus(int input) {
  if (input > 4 || input < 0) {
    return 256;
  }
  int value;
  switch (input) {
    case 1:
      value = 16;
      break;
    case 2:
      value = 32;
      break;
    case 3:
      value = 64;
      break;
    case 4:
      value = 128;
      break;
  }
getGPIOStatusRetry:
  Wire.beginTransmission(address);
  Wire.write(0x09);
  byte status = Wire.endTransmission();
  if (status != 0) {
    if (retrys < 3) {
#ifdef LOGGING
      Serial.println("Retry read input status command");
#endif
      retrys++;
      goto getGPIOStatusRetry;
    } else {
#ifdef LOGGING
      Serial.println("Read input status command failed");
#endif
      initialized = false;
      retrys = 0;
      return 256;
    }
  } else {
    retrys = 0;
    initialized = true;
  }
  Wire.requestFrom(address, 1);
  byte bankStatus = Wire.read();
  if (bankStatus & value) {
    return 0;
  } else {
    return 1;
  }
}
byte readAllInputs() {
readInputStatusRetry:
  Wire.beginTransmission(address);
  Wire.write(0x09);
  byte status = Wire.endTransmission();
  if (status != 0) {
    if (retrys < 3) {
#ifdef LOGGING
      Serial.println("Retry read bank status command");
#endif
      retrys++;
      goto readInputStatusRetry;
    } else {
#ifdef LOGGING
      Serial.println("Read bank status command failed");
#endif
      initialized = false;
      retrys = 0;
      return 256;
    }
  } else {
    retrys = 0;
    initialized = true;
  }
  Wire.requestFrom(address, 1);
  byte bankStatus = Wire.read();
  byte inverted = ~bankStatus;
  byte shifted = inverted >> 4;
  return shifted;
}
