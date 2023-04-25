void setup() {

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    UARTTaskCode,  /* Task function. */
    "DisplayTask", /* name of task. */
    10000,         /* Stack size of task */
    NULL,          /* parameter of the task */
    1,             /* priority of the task */
    &UARTTask,     /* Task handle to keep track of created task */
    0);            /* pin task to core 0 */

  // GPIO
  pinMode(LOCK_PIN, OUTPUT);
  digitalWrite(LOCK_PIN, HIGH);
  controllerIsOn = 1;

  //Setup sleep wakeup on Touch Pad 3 ( GPIO15 )
  touchSleepWakeUpEnable(T3,40);


  Serial.begin(9600);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Starting BLE work!");

  // Setup up BLE
  initBle(SCOOTER_NAME);
   auto* server = BLEDevice::createServer();
   auto* pService = server->createService(SERVICE_UUID); 
   

  pMainCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_MAIN,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

  pMainCharacteristic->setCallbacks(new MainBLECallback());


   pDebugCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_DEBUG,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);


  pSettingsCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_SETTINGS,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

  pSettingsCharacteristic->setCallbacks(new SettingsBLECallback());

pService->start();
BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();
  
  ArduinoBleOTA.begin(InternalStorage, HW_NAME, HW_VER, SW_NAME, SW_VER);
  //ArduinoBleOTA.setSecurity(security);
  advertiseBle(SCOOTER_NAME,SERVICE_UUID);
  //security.begin();
  
  Serial.println("Ready!");

  // Play ready sound
  tone(BUZZZER_PIN, 300, 100);
  delay(100);
  tone(BUZZZER_PIN, 400, 100);
  delay(100);
  tone(BUZZZER_PIN, 500, 100);
  delay(100);
  noTone(BUZZZER_PIN);

  turnOffDisplayLed();
}
