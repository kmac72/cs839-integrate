// Adapted from: https://github.com/jamesmontemagno/mycadence-arduino/blob/main/device.h

//This is standard speed & cadence bluetooth UUID
static BLEUUID    serviceUUID("00001816-0000-1000-8000-00805f9b34fb");
// This is the standard notify characteristic UUID
static BLEUUID     notifyUUID("00002a5b-0000-1000-8000-00805f9b34fb");  

static BLEAdvertisedDevice * devices[20];
static int device_count = 0;

// button handling
#define BUTTON_DEBOUNCE_TIME 50
#define BUTTON_SHORT_PRESS_TIME 400

// Add a device to our device list (deduplicate in the process)
void addDevice(BLEAdvertisedDevice * device) {
  for (uint8_t i = 0; i < device_count; i++) {
    if(device->getName() == devices[i]->getName()){
      return;
    }
  }
  if(!device->haveServiceUUID() || !device->isAdvertisingService(serviceUUID)) {
    return;
  }
  devices[device_count] = device;
  device_count++;
}

// Select a device and return it
BLEAdvertisedDevice * selectDevice(void) {
  if(device_count == 0) return nullptr;
  if(device_count == 1) return devices[0];

  int selected = 0;

  // Select a device
  // Instead of reading from pairing button, pair immediately
  while(true) {
    if(selected > device_count-1) selected = 0;
    int start = selected < 2 ? 0 : selected - 1;
    int lastState = LOW;  // the previous state from the input pin
    int currentState;     // the current reading from the input pin
    unsigned long pressedTime  = 0;
    for(int i = 0; i <device_count; i++){
      // Hard-code
      if (strcmp(devices[i]->getName().c_str(),"CS8L-C 06217")==0){
        return devices[i];
        break;
      }
    }
       // save the the last state
       lastState = currentState;    
    }
  
  return nullptr;
}  
