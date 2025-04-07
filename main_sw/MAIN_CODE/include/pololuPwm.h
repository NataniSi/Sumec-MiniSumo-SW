/**
 * Sensor: https://www.pololu.com/product/4071
 * Docs (esp-idf 4.4)
 * Rmt docs: https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32s3/api-reference/peripherals/rmt.html
 * Ring buffer and rmt example: https://github.com/espressif/esp-idf/blob/v4.4/examples/peripherals/rmt/ir_protocols/main/ir_protocols_main.c
 * Ring buffer code: https://github.com/espressif/esp-idf/blob/a3864c088dafb0b8ce94dba272685b850b46c837/components/esp_ringbuf/ringbuf.c
 * Rmt code https://github.com/espressif/esp-idf/blob/v4.4/components/driver/rmt.c
 */



#include <driver/rmt.h>



class pwmSensor {
private:
  rmt_config_t rmtConfObj = {
  .rmt_mode      = RMT_MODE_RX,
  .clk_div       = 80,  // 1tick = 1us.
  .mem_block_num = 1,
  .flags         = 0,
  .rx_config =
  {
  .idle_threshold      = 5000,  // Longest pulse (in ticks).
  .filter_ticks_thresh = 250,   // Shortest pulse (in ticks).
  .filter_en           = true,
  },
  };
  RingbufHandle_t rmtRingBufferHandle = NULL;
  const uint16_t  BUFFER_LEN_ITEMS    = 0x1ff;

public:
  pwmSensor(uint8_t pin, uint8_t channel);
  bool     pwmSetup();
  void     pwmRead(uint16_t* retrievedItemRetVar);
  uint32_t pwmToMm(uint16_t pwmLength);
};



/**
 * @param pin Gpio to read from.
 * @param channel Rmt channel. Valid read channels are 4-7.
 * @brief Setup.
 */
pwmSensor::pwmSensor(uint8_t pin, uint8_t channel) {
  if(channel < 4 || channel > 7) {
    log_e("Wrong read channel");
    return;
  }

  rmtConfObj.channel  = (rmt_channel_t)channel;  // >= 4.
  rmtConfObj.gpio_num = (gpio_num_t)pin;
}


/**
 * @returns Error (0|1) - (cannot optain ring buffer handle|all is ok).
 * @brief Sets up the rmt stuff. 
 */
bool pwmSensor::pwmSetup() {
  ESP_ERROR_CHECK(rmt_config(&rmtConfObj));

  ESP_ERROR_CHECK(rmt_driver_install(rmtConfObj.channel, BUFFER_LEN_ITEMS * 2, 0));  // Why is it multiplied by two?
  rmt_get_ringbuf_handle(rmtConfObj.channel, &rmtRingBufferHandle);

  if(rmtRingBufferHandle == NULL) {
    log_e("rmtRingBufferHandle is NULL. Do something differently, ig.");
    return 0;
  }
  return 1;
}


/**
 * @param retrievedItemRetVar Variable to store the pwm pulse length in. Value of 0xffff means the sensor didn't find anything (or just didn't return a meaningful value).
 * @brief Starts the rmt read on the sensors channel and pin. Could be optimised by using just one channel and swaping the connected gpio pin but this works too. (This way should also be faster.)
 */
void pwmSensor::pwmRead(uint16_t* retrievedItemRetVar) {
  *retrievedItemRetVar            = 0;
  size_t        retrievedItemSize = 0;
  rmt_item32_t* retrievedItem     = NULL;
  rmt_rx_start(rmtConfObj.channel, 1);
  retrievedItem = (rmt_item32_t*)xRingbufferReceive(rmtRingBufferHandle, &retrievedItemSize, 2500);
  if(retrievedItem != NULL) {
    for(uint64_t idx = 0; idx < retrievedItemSize * 5; idx++) {
      if(retrievedItem[idx].level0 == 1) {
        if(retrievedItem[idx].duration0 > 1000) {
          *retrievedItemRetVar = retrievedItem[idx].duration0;
          rmt_rx_stop(rmtConfObj.channel);
          break;
        }
      }

      if(retrievedItem[idx].level1 == 1) {
        if(retrievedItem[idx].duration1 > 1000) {
          *retrievedItemRetVar = retrievedItem[idx].duration1;
          rmt_rx_stop(rmtConfObj.channel);
          break;
        }
      }
    }
    vRingbufferReturnItem(rmtRingBufferHandle, (void*)retrievedItem);
  }
  rmt_rx_stop(rmtConfObj.channel);

  if(*retrievedItemRetVar <= 1000) {
    *retrievedItemRetVar = 0xffff;
  }

  return;
}


/**
 * @param pwmLength Length of the pwm HIGH pulse in us.
 * @returns Distance in mm.
 * @brief Converts pwm signal to distance using a fromula from the polopu docs.
 */
uint32_t pwmSensor::pwmToMm(uint16_t pwmLength) {
  return (2 * (pwmLength - 1000));
}
