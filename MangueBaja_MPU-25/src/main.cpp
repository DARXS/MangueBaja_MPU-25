#include <Arduino.h>
#include <StateMachine.h>
#include <BLE.h>
#include <CAN.h>  

/* Task Management */
TaskHandle_t StateMachine_Task = NULL, BLEtask = NULL;

uint8_t status = FAIL_RESPONSE; // Flag to check the LoRa and update in debug data packet (bluetooth)

/* Tasks */
void StateMachineTask(void *pvParameters);
void BLE_Data_Task(void *pvParameters);

void setup() 
{
  Serial.begin(115200);
  GPS_uart.begin(GPS_Baud_Rate, SERIAL_8N1, GPS_TX, GPS_RX);
  
  pinMode(EMBEDDED_LED, OUTPUT);

  /* CAN-BUS initialize */
  if (!CAN_start_device())
    esp_restart();
  
  /* Init the LoRa module and storage the @return in this @var */
  status = LORA_init();
    
  /* Init the BLE host connection */
  Init_BLE_Server();

  /* Init the Circular Buffer and Tickers */
  CircularBuffer_Ticker_Init();

  /* Tasks */
  // Task to management the Circular Buffer and the State Machine
  xTaskCreatePinnedToCore(StateMachineTask, "StateMachineTask", 4096, NULL, 5, &StateMachine_Task, 0);
  // Task to management the BLE communication
  xTaskCreatePinnedToCore(BLE_Data_Task, "BLE_Data_Task", 4096, NULL, 4, &BLEtask, 1);
}

void loop() {/**/}

void StateMachineTask(void *pvParameters)
{
  while (1)
  {
    CircularBuffer_CurrentState();

    vTaskDelay(1);
  }
  
  vTaskDelay(1);
}

void BLE_Data_Task(void *arg)
{
  for (;;)
  { 
    if (BLE_connected() && App_MPU_data_request())
    {
      Save_LORA_init_status(status);
      Send_MPU_REQUEST(true);
      
      vTaskDelay(200); // Wait the response
      
      Send_BLE_msg();
      CLEAR_BLE_MSG();

      vTaskDelay(MAX_BLE_DELAY);
    } 

    vTaskDelay(1);
  }

  vTaskDelay(1);
}
