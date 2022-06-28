#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"


#define TX_ADR_WIDTH  5        
#define RX_ADR_WIDTH  5       
#define TX_PLOAD_WIDTH  32  
#define RX_PLOAD_WIDTH  32  

//#define TX_ADDRESS   0x0909090909 
//#define RX_ADDRESS   0x0909090909 

uint8_t TX_ADDRESS[5] = {0xE6, 0xE6, 0xE6, 0xE6, 0xE6};
uint8_t RX_ADDRESS[5] = {0xE6, 0xE6, 0xE6, 0xE6, 0xE6};
//#define TX_ADDRESS   0x0  //本地地址
//#define RX_ADDRESS   0x0  //接收地址
#define R_REGISTER_MASK   0x00                   
#define W_REGISTER_MASK   0x20
#define R_RX_PAYLOAD  0x61                      
#define W_TX_PAYLOAD  0xA0                       
#define FLUSH_TX     0xE1                       
#define FLUSH_RX     0xE2                      
#define REUSE_TX_PL  0xE3                       
#define NOP          0xFF                      

// *************************************SPI(nRF24L01)寄存器地址****************************************************
// Mnemonic   Address  Description
#define CONFIG        0x00  // 配置收发状态，CRC校验模式以及收发状态响应方式
#define EN_AA         0x01  // 自动应答功能设置
#define EN_RXADDR     0x02  // 可用信道设置
#define SETUP_AW      0x03  // 收发地址宽度设置
#define SETUP_RETR    0x04  // 自动重发功能设置
#define RF_CH         0x05  // 工作频率设置
#define RF_SETUP      0x06  // 发射速率、功耗功能设置
#define STATUS        0x07  // 状态寄存器
#define OBSERVE_TX    0x08  // 发送监测功能
#define CD            0x09  // 地址检测           
#define RX_ADDR_P0    0x0A  // 频道0接收数据地址
#define RX_ADDR_P1    0x0B  // 频道1接收数据地址
#define RX_ADDR_P2    0x0C  // 频道2接收数据地址
#define RX_ADDR_P3    0x0D  // 频道3接收数据地址
#define RX_ADDR_P4    0x0E  // 频道4接收数据地址
#define RX_ADDR_P5    0x0F  // 频道5接收数据地址
#define TX_ADDR       0x10  // 发送地址寄存器
#define RX_PW_P0      0x11  // 接收频道0接收数据长度
#define RX_PW_P1      0x12  // 接收频道0接收数据长度
#define RX_PW_P2      0x13  // 接收频道0接收数据长度
#define RX_PW_P3      0x14  // 接收频道0接收数据长度
#define RX_PW_P4      0x15  // 接收频道0接收数据长度
#define RX_PW_P5      0x16  // 接收频道0接收数据长度
#define FIFO_STATUS   0x17  // FIFO栈入栈出状态寄存器设置
#define TX_OK         0x20  //TX发送完成中断
#define MAX_TX        0x10  //达到最大发送次数中断


#define MOSI   11
#define CSN    8
#define MISO   12
#define SCK    13
#define CE     9

// Pin Wiggling Macros:
void SPI_CS_1() {
    gpio_put(CSN, false); 
}
void SPI_CS_0(){
    gpio_put(CSN, true); 
}
void SPI_SCK_1(){
    gpio_put(SCK, true);
}
void SPI_SCK_0(){
    gpio_put(SCK, false);
} 
void SPI_MOSI_1(){
    gpio_put(MOSI, true);
}
void SPI_MOSI_0(){
    gpio_put(MOSI, false); 
}
void SPI_CE_1(){
    gpio_put(CE, true); 
}
void SPI_CE_0(){
    gpio_put(CE, false); 
}
    
uint8_t SPI_READ_MISO(){
  return gpio_get(MISO);
}



void gpio_pins_initialize(){

  gpio_init(MISO);
  gpio_set_dir(MISO, GPIO_IN); 
  
  gpio_init(SCK);
  gpio_set_dir(SCK, GPIO_OUT); 

  gpio_init(MOSI);
  gpio_set_dir(MOSI, GPIO_OUT); 

  gpio_init(CE);
  gpio_set_dir(CE, GPIO_OUT); 

  gpio_init(CSN);
  gpio_set_dir(CSN, GPIO_OUT); 

  SPI_CS_0();
  SPI_SCK_0();
}


void spi_delay() {
  //delay(1);
}

void gpio_clockout_8_bits(uint8_t txData) {
  spi_delay();
  for (int i = 0; i < 8; ++i) {
      SPI_SCK_0();
      spi_delay();
      if(txData & 0x80) // MSB on each byte  
          SPI_MOSI_1();
      else
          SPI_MOSI_0();
      SPI_SCK_1(); // clock data
      txData = txData << 1;
      spi_delay();
  }
  SPI_SCK_0();
}

uint8_t gpio_clockin_8_bits(){
  uint8_t rxData = 0;
  spi_delay();
  for (int i=0; i < 8; ++i) {
      SPI_SCK_0();
      spi_delay();
      // SPI_MOSI_0() # dummy byte - dummy bit, clock edge is sufficient for MISO data to spit out.
      SPI_SCK_1();
      spi_delay();
      rxData = rxData << 1; // Why shift first then OR'? range (0, 8) will need to shift only 7 times.
      rxData |= SPI_READ_MISO();
      spi_delay();
  }
  SPI_SCK_0();
  return rxData;
}
  


void spi_write_register(uint8_t reg, uint8_t* val, uint8_t num_bytes){
  // Select chip
  SPI_CS_1();

  // Write chip register 
  gpio_clockout_8_bits(reg);
  // Write value
  for (int i = 0; i < num_bytes; ++i){
    uint8_t writing_byte = val[i];
    gpio_clockout_8_bits(writing_byte);
    // val = val >> 8;
    
  }

  // Deselect chip
  SPI_CS_0();
}

void spi_read_register(uint8_t reg, uint8_t num_bytes, uint8_t* pbuf){
  // Select chip
  SPI_CS_1();

  // Write register address to read.
  gpio_clockout_8_bits(reg);
  // Read value
  for (int i = 0; i < num_bytes; ++i) {
    pbuf[i] = gpio_clockin_8_bits();
  }
  // Deselect chip
  SPI_CS_0();
}

// uint8_t tx_addr[5];
// spi_read_register(R_REGISTER_MASK + TX_ADDR, 5, tx_addr);
// printf("tx_addr: "); for (int i = 0; i < 5; i++) printf(tx_addr[i], HEX); printf();
uint8_t nrf24_get_STATUS() {
  uint8_t stat;
  spi_read_register(R_REGISTER_MASK + STATUS, 1, &stat);
  // printf("--------");
  // printf("STATUS: "); printf(stat,HEX);
  return stat;
}

uint8_t nrf24_get_FIFO_STATUS() {
  uint8_t fifo_status;
  spi_read_register(R_REGISTER_MASK + FIFO_STATUS, 1, &fifo_status);
  printf("fifo status: "); printf("%x", fifo_status);
}


uint8_t nrf24_get_CONFIG() {
  uint8_t config_reg;
  spi_read_register(R_REGISTER_MASK + CONFIG, 1, &config_reg);
  printf("CONFIG: "); printf("%x", config_reg);
}


void nrf24_poweron_self_test() {
  uint8_t config_reg;
  spi_read_register(R_REGISTER_MASK + CONFIG, 1, &config_reg);
  if (config_reg != 0x08) { // the register reset value is expect 0x08
    printf("(!) Critical Error: NRF24 CONFIG register should have reset value of 0x08. Re-plug in nrf24 on the 3.3V power wire.");
  }
}

/*  Brief: 1. Disable Auto Acknowledgement, Auto Retransmit. TX_DS will be be set if these two are not turn-off.
 *         2. TX_DS (in STATUS register) is expected to be set when data in TX FIFO is set.
 *  Others: 
 *        How to know TX payload is loaded?
 *        After writing to W_TX_PAYLOAD, TX_EMPTY (in FIFO_STATUS register) becomes 0.
 *        
 *        What happens if sending is not successful?
 *        TX_FULL (in FIFO_STATUS register) becomes 1.
 *        TX_FULL (in STATUS register) becomes 1.
 *        TX_DS (in STATUS register) remains 0.  
 *  States: 
 *        The states can be referred in 6.1.1 State diagram.
 */
bool nrf24_tx_self_test() {
  uint8_t stat;
  // [Current State: Power-on reset 100 ms] 
  SPI_CE_0();
  // [Current State: Power Down]
  spi_write_register(W_REGISTER_MASK + EN_AA, 0x00, 1);        // disable auto acknowledgement  
  spi_write_register(W_REGISTER_MASK + EN_RXADDR, 0x00, 1);    // disable RX datapipes
  spi_write_register(W_REGISTER_MASK + SETUP_RETR, 0x00, 1);   // disable automatic retransmission, ARC = 0000
  //////// DISPOSABLE
  spi_write_register(W_REGISTER_MASK + RF_CH, (uint8_t*) 40, 1);   // disable automatic retransmission, ARC = 0000
  spi_write_register(W_REGISTER_MASK + RF_SETUP, (uint8_t*) 0b00001110, 1);
  spi_write_register(W_REGISTER_MASK + RX_PW_P0, (uint8_t*) 4, 1);
  ////////DISPOSABLE
  // [Current State: Standby-I]
  spi_write_register(W_REGISTER_MASK + CONFIG, (uint8_t*) 0x0E, 1);       // PWR_UP = 1 PRIMRX=0 (TX mode)
  stat = nrf24_get_STATUS();
  // [Current State: TX MODE]
  uint8_t payload[] = {0x11, 0x11, 0x22, 0x33}; // clock in a payload, TX FIFO not empty 
  spi_write_register(W_TX_PAYLOAD, payload, 4);
  stat = nrf24_get_STATUS();
  SPI_CE_1(); // fire out the transmit packet
  sleep_ms(10);
  // TX Setting
  stat = nrf24_get_STATUS();
  if (stat & 0x20) { // TX_DS bit is set.
    printf("nrf24 transmission self test has passed.");
    return true;
  } else {
    printf("nrf24 transmission self test has failed.");
    return false;
  }
  SPI_CE_0(); // stop transmission. return to Standby-I state.
  // Return to [State: Standby-I]
}

void nrf24_keep_sending() {

  uint8_t payload[] = {0x11, 0x11, 0x22, 0x33}; // clock in a payload, TX FIFO not empty 
  spi_write_register(W_TX_PAYLOAD, payload, 4);
  SPI_CE_1(); // fire out the transmit packet
  sleep_ms(1);
  // TX Setting
  uint8_t stat = nrf24_get_STATUS();
  printf("%x\n", stat);
  // if (stat & 0x20) { // TX_DS bit is set.
  if (stat == 0x2e) { // TX_DS bit is set.
    // printf("nrf24 send successful.");
  } else {
    // printf("nrf24 send failure.");
  }
  // write 1 to clear TX_DS
  spi_write_register(W_REGISTER_MASK + STATUS, (uint8_t*) 0x20, 1); 
  SPI_CE_0(); // stop transmission. return to Standby-I state.
  // Return to [State: Standby-I]
}



int main() {

    bi_decl(bi_program_description("nrf24l01 sender program"));

    // enable uart
    uart_init(uart0, 115200);
    // set SPI pin modes.
    // gpio_pins_initialize();
    gpio_set_function(0, GPIO_FUNC_UART)
    gpio_set_function(1, GPIO_FUNC_UART)

    // nrf24_poweron_self_test();
    
    // nrf24_tx_self_test();

    // init on-board led
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);


    while(1) {
      // nrf24_keep_sending();
      // printf("nrf24 init started.\r\n");
      uart_puts(uart0. "helloworld!\n");
      sleep_ms(1000);
      gpio_put(25, 0);
      sleep_ms(1000);
      gpio_put(25, 1);
      sleep_ms(1000);
    }

    return 0;

}

