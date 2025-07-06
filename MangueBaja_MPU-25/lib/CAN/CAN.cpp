#include "CAN.h"

CANmsg txMsg(CAN_RX_id, CAN_TX_id, CAN_BPS_1000K);
radio_packet_t can_receive_packet;
bluetooth bluetooth_packet;
bool mode = false;

bool CAN_start_device(bool debug_mode)
{
  txMsg.Set_Debug_Mode(debug_mode);
  if (!txMsg.init(canISR))
  {
    Serial.println("CAN ERROR!! SYSTEM WILL RESTART IN 2 SECONDS...");
    // log_e("CAN ERROR!! SYSTEM WILL RESTART IN 2 SECONDS...");
    vTaskDelay(2000);
    return false;
  }
  /* if you needed apply a Mask and id to filtering write this:
   * txMsg.init(canISR, ID); or txMsg.init(canISR, ID, Mask);
   * if you ID is bigger then 0x7FF the extended mode is activated
   * you can set the filter in this function too:
   * txMsg.Set_Filter(uint32_t ID, uint32_t Mask, bool Extended);  */
  memset(&can_receive_packet, 0, sizeof(radio_packet_t));
  CLEAR_BLE_MSG();
  return true;
}

void Save_LORA_init_status(uint8_t st)
{
  bluetooth_packet.lora_init = st;
}

void Save_CAN_Data_in_Packet(radio_packet_t *packet)
{
  packet->imu_acc.acc_x = can_receive_packet.imu_acc.acc_x;
  packet->imu_acc.acc_y = can_receive_packet.imu_acc.acc_y;
  packet->imu_acc.acc_z = can_receive_packet.imu_acc.acc_z;

  packet->imu_dps.dps_x = can_receive_packet.imu_dps.dps_x;
  packet->imu_dps.dps_y = can_receive_packet.imu_dps.dps_y;
  packet->imu_dps.dps_z = can_receive_packet.imu_dps.dps_z;

  packet->rpm = can_receive_packet.rpm;
  packet->speed = bluetooth_packet.speed_pulse_counter;
  packet->temperature = bluetooth_packet.termistor;
  packet->flags = can_receive_packet.flags;
  packet->SOC = bluetooth_packet.measure_volt;
  packet->cvt = bluetooth_packet.cvt_temperature;
  packet->volt = can_receive_packet.volt;
  packet->timestamp = millis();
}

bool Send_GPS_data(double _msg, uint32_t _ID)
{
  txMsg.clear(_ID);
  txMsg << _msg;
  return txMsg.write();
}

bool Send_MPU_REQUEST(bool _msg)
{
  txMsg.clear(MPU_ID);
  txMsg << _msg;
  return txMsg.write();
}

bluetooth update_packet()
{
  return bluetooth_packet;
}

void CLEAR_BLE_MSG()
{
  memset(&bluetooth_packet, 0, sizeof(bluetooth));
}

/* CAN functions */
void canISR(CAN_FRAME *rxMsg)
{
  mode = !mode;
  digitalWrite(EMBEDDED_LED, mode);

  can_receive_packet.timestamp = millis();

  switch (rxMsg->id)
  {
  case IMU_ACC_ID:
    memcpy(&can_receive_packet.imu_acc, (imu_acc_t *)&rxMsg->data.uint8, sizeof(imu_acc_t));
    // Serial.printf("ACC X = %f\r\n", (float)((can_receive_packet.imu_acc.acc_x*0.061)/1000));
    // Serial.printf("ACC Y = %f\r\n", (float)((can_receive_packet.imu_acc.acc_y*0.061)/1000));
    // Serial.printf("ACC Z = %f\r\n", (float)((can_receive_packet.imu_acc.acc_z*0.061)/1000));
    break;

  case IMU_DPS_ID:
    memcpy(&can_receive_packet.imu_dps, (imu_dps_t *)&rxMsg->data.uint8, sizeof(imu_dps_t));
    // Serial.printf("DPS X = %d\r\n", can_receive_packet.imu_dps.dps_x);
    // Serial.printf("DPS Y = %d\r\n", can_receive_packet.imu_dps.dps_y);
    // Serial.printf("DPS Z = %d\r\n", can_receive_packet.imu_dps.dps_z);
    break;

  case RPM_ID:
    memcpy(&can_receive_packet.rpm, (uint16_t *)&rxMsg->data.uint8, sizeof(uint16_t));
    // Serial.printf("RPM = %d\r\n", can_receive_packet.rpm);
    break;

  case SPEED_ID:
    memcpy(&bluetooth_packet.speed_pulse_counter, (uint8_t *)&rxMsg->data.uint8, sizeof(uint8_t));
    // Serial.printf("Speed = %d\r\n", bluetooth_packet.speed_pulse_counter);
    break;

  case TEMPERATURE_ID:
    memcpy(&bluetooth_packet.termistor, (uint8_t *)&rxMsg->data.uint8, sizeof(uint8_t));
    // Serial.printf("Motor = %d\r\n", bluetooth_packet.termistor);
    break;

  case FLAGS_ID:
    memcpy(&can_receive_packet.flags, (uint8_t *)&rxMsg->data.uint8, sizeof(uint8_t));
    // Serial.printf("Flags = %d\r\n", can_receive_packet.flags);
    break;

  case SOC_ID:
    memcpy(&bluetooth_packet.measure_volt, (uint8_t *)&rxMsg->data.uint8, sizeof(uint8_t));
    // Serial.printf("SOC = %d\r\n", bluetooth_packet.measure_volt);
    break;

  case CVT_ID:
    memcpy(&bluetooth_packet.cvt_temperature, (uint8_t *)&rxMsg->data.uint8, sizeof(uint8_t));
    // Serial.printf("CVT = %d\r\n", bluetooth_packet.cvt_temperature);
    break;

  case VOLTAGE_ID:
    memcpy(&can_receive_packet.volt, (float *)&rxMsg->data.uint8, sizeof(float));
    // Serial.printf("Volt = %f\r\n", can_receive_packet.volt);
    break;

  case MMI_ID:
    memcpy(&bluetooth_packet.accel_begin, (uint8_t *)&rxMsg->data.uint8, sizeof(uint8_t));
    // Serial.printf("accel_begin = %d\r\n", bluetooth_packet.accel_begin);
    break;

  case TCU_ID:
    memcpy(&bluetooth_packet.servo_state, (uint8_t *)&rxMsg->data.uint8, sizeof(uint8_t));
    //Serial.printf("servo_state = %s\r\n", bluetooth_packet.servo_state == 4 ? "CHOKE" :  \
    bluetooth_packet.servo_state == 3 ? "MID" : bluetooth_packet.servo_state == 2 ? "RUN" : "ERRO");
    break;

  case SCU_ID:
  {
    uint8_t recv;

    memcpy(&recv, (uint8_t *)&rxMsg->data.uint8, sizeof(uint8_t));
    // Serial.printf("All data: %d\r\n", recv);

    bluetooth_packet.internet_modem = recv & 0x03;
    bluetooth_packet.mqtt_client_connection = (recv >> 2) & 0x03;
    bluetooth_packet.sd_start = (recv >> 4) & 0x03;
    bluetooth_packet.check_sd = (recv >> 6) & 0x03;

    // Serial.printf("internet_modem: %d\r\n", bluetooth_packet.internet_modem);
    // Serial.printf("mqtt_client_connection: %d\r\n", bluetooth_packet.mqtt_client_connection);
    // Serial.printf("sd_start: %d\r\n", bluetooth_packet.sd_start);
    // Serial.printf("check_sd: %d\r\n", bluetooth_packet.check_sd);
    break;
  }
  }
}
