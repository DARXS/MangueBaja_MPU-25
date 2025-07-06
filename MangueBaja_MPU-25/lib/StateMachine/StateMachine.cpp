#include "StateMachine.h"

// if you wanna to push the DEBUG_ST, uncomment this define
//#define DEBUG

CircularBuffer<state_t, BUFFER_SIZE> state_buffer;
TinyGPSPlus gps;
Ticker ticker1Hz, ticker250mHz;

radio_packet_t volatile_packet;
state_t current_state = IDLE_ST;
bool buffer_full = false, g = false;

void CircularBuffer_Ticker_Init()
{
    memset(&volatile_packet, 0, sizeof(radio_packet_t));
    ticker1Hz.attach(1.0, ticker1HzISR);
    ticker250mHz.attach(4.0, ticker250mHzISR);
}

void CircularBuffer_CurrentState()
{
    if (state_buffer.isFull())
    {
        // buffer_full = true;
        current_state = state_buffer.pop();
    } else {
        // buffer_full = false;
        if (!state_buffer.isEmpty())
            current_state = state_buffer.pop();
        else
            current_state = IDLE_ST;
    }

    switch (current_state)
    {
    case IDLE_ST:
        //Serial.println("i");
        break;

    case RADIO_ST:
        //Serial.println("Radio");

        Save_CAN_Data_in_Packet(&volatile_packet);        
        LORA_SendStruct(&volatile_packet, sizeof(volatile_packet));
        // if(LORA_SendStruct(&volatile_packet, sizeof(volatile_packet)))
        // Serial.println("ok");

        break;

    case GPS_ST:
        //Serial.println("GPS");

        while (GPS_uart.available() > 0)
        {
            if (gps.encode(GPS_uart.read()))
                g = get_GPS_data();
        }

        /* Send latitude message */
        if (Send_GPS_data(volatile_packet.latitude, LAT_ID))
        {
            /* Send longitude message if latitude is successful */
            Send_GPS_data(volatile_packet.longitude, LNG_ID);
        }

        break;

    case DEBUG_ST:
        Serial.println("Debug state");
        Serial.printf("Latitude (LAT) = %lf\r\n", volatile_packet.latitude);
        Serial.printf("Longitude (LNG) = %lf\r\n", volatile_packet.longitude);

        PRINT_GPS_TIME();
        PRINT_GPS_DATE();

        Serial.printf("Satellites = %d\r\n", volatile_packet.sat);
        Serial.println("\n\n");
        break;
    }
}

bool get_GPS_data()
{

    if (gps.location.isValid())
    {
        volatile_packet.latitude = gps.location.lat();
        volatile_packet.longitude = gps.location.lng();

        if (gps.satellites.isValid())
            volatile_packet.sat = gps.satellites.value();
        else
            volatile_packet.sat = 0;

        return true;
    }

    //else
    //{
    //    volatile_packet.latitude = 0;
    //    volatile_packet.longitude = 0;

        return false;
    //}
}

/* Tickers */
void ticker250mHzISR(void)
{
    state_buffer.push(GPS_ST);
}

void ticker1HzISR()
{
    state_buffer.push(RADIO_ST);

    #ifdef DEBUG
        state_buffer.push(DEBUG_ST);
    #endif
}

// DEBUG FUNCTIONS
void PRINT_GPS_TIME(void)
{
    // Time and Data in point 0 of the map
    (gps.time.isValid() ? Serial.printf("Time: %dh:%dm:%ds\r\n", gps.time.hour(), \
        gps.time.minute(), gps.time.second()) : Serial.println("Time: 0h:0m:0s\r\n"));
}

void PRINT_GPS_DATE(void)
{
    (gps.date.isValid() ? Serial.printf("Date: %d/%d/%d\r\n", gps.date.day(), gps.date.month(), \
        gps.date.year()) : Serial.println("Date: 0/0/0\r\n"));
}
