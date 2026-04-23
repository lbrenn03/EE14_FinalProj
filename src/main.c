#include <inttypes.h>
#include <string.h>
#include "ee14lib.h"
#include "crypto.h"

#define FW_VALID_ADDR   0x08007FF8
#define FW_VALID_MAGIC  0xDEADBEEFCAFEBABEULL

#define UPDATE_EN_PIN   D10
#define APP_ADDRESS     0x08008000
#define APP_FLASH_SIZE  (256*1024 - 32*1024)

// TODO: define the I2C device address for the auth peripheral
#define AUTH_I2C_ADDR   0x00
// TODO: define which I2C bus and pins the auth peripheral is on
#define AUTH_I2C        I2C1
#define AUTH_I2C_SCL    D1
#define AUTH_I2C_SDA    D0

volatile uint32_t WATCHDOG_TIMER = 5000;

void SysTick_Handler(void) {
    WATCHDOG_TIMER--;
}

void SysTick_initialize(void) {
    SysTick->CTRL = 0;
    SysTick->LOAD = 3999;
    NVIC_SetPriority(SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);
    SysTick->VAL  = 0;
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

void jump_to_application(void) {
    __disable_irq();
    HAL_DeInit();
    uint32_t app_sp = *(volatile uint32_t *)(APP_ADDRESS);
    uint32_t app_pc = *(volatile uint32_t *)(APP_ADDRESS + 4);
    SCB->VTOR = APP_ADDRESS;
    __set_MSP(app_sp);
    void (*reset_handler)(void) = (void (*)(void))app_pc;
    reset_handler();
}

typedef enum { AUTH, UPDATE, SUCCESS, FAIL } BOOT_STATE;
typedef enum { BOOT_ERR_OK, BOOT_ERR_TIMEOUT, BOOT_ERR_AUTH, BOOT_ERR_CRC, BOOT_ERR_FLASH } BOOT_ERR;

BOOT_ERR authorize_boot(void) {
    i2c_init(AUTH_I2C, AUTH_I2C_SCL, AUTH_I2C_SDA);

    uint8_t challenge[32];
    generate_challenge(challenge, sizeof(challenge));

    if (!i2c_write(AUTH_I2C, AUTH_I2C_ADDR, challenge, sizeof(challenge))) {
        return BOOT_ERR_AUTH;
    }

    uint8_t response[32];
    if (!i2c_read(AUTH_I2C, AUTH_I2C_ADDR, response, sizeof(response))) {
        return BOOT_ERR_AUTH;
    }

    if (verify_mac(challenge, sizeof(challenge), response) != 0) {
        return BOOT_ERR_AUTH;
    }

    return BOOT_ERR_OK;
}

BOOT_ERR recv_code(void) {
    // TODO: define handshake protocol to receive image_size from host
    uint32_t image_size = 0;

    if (image_size == 0 || image_size > APP_FLASH_SIZE) {
        return BOOT_ERR_FLASH;
    }

    const uint32_t pagesize = 2048;

    if ((FLASH->SR & FLASH_SR_BSY) == 0) {
        if ((FLASH->CR & FLASH_CR_LOCK) != 0) {
            FLASH->KEYR = 0x45670123;
            FLASH->KEYR = 0xCDEF89AB;
        }
    }

    uint8_t *app_base = (uint8_t *)APP_ADDRESS;
    for (uint32_t i = 0; i < image_size; i += pagesize) {
        uint32_t chunk = (image_size - i < pagesize) ? (image_size - i) : pagesize;

        // Receive chunk bytes over UART
        for (uint32_t j = 0; j < chunk; j++) {
            app_base[i + j] = (uint8_t)serial_read(USART2);
        }

        // TODO: erase flash page before writing
        // TODO: write received bytes to flash (word-by-word via FLASH->CR PG mode)

        // TODO: send ack to host
    }

    FLASH->CR |= FLASH_CR_LOCK;

    // TODO: implement verify_crc — receive trailing CRC from host and verify
    // if (!verify_crc((uint8_t *)APP_ADDRESS, image_size)) {
    //     return BOOT_ERR_CRC;
    // }

    return BOOT_ERR_OK;
}

int main(void) {
    host_serial_init(9600);
    SysTick_initialize();

    gpio_config_mode(UPDATE_EN_PIN, INPUT);
    gpio_config_pullup(UPDATE_EN_PIN, PULL_UP);

    if (*(uint64_t *)FW_VALID_ADDR == FW_VALID_MAGIC && gpio_read(UPDATE_EN_PIN) == 1) {
        jump_to_application();
    }

    BOOT_STATE curr_state = AUTH;

    while (1) {
        if (curr_state == AUTH) {
            curr_state = (authorize_boot() == BOOT_ERR_OK) ? UPDATE : FAIL;

        } else if (curr_state == UPDATE) {
            curr_state = (recv_code() == BOOT_ERR_OK) ? SUCCESS : FAIL;

        } else if (curr_state == SUCCESS) {
            *(uint64_t *)FW_VALID_ADDR = FW_VALID_MAGIC;
            jump_to_application();

        } else if (curr_state == FAIL) {
            // TODO: erase app flash region
            // TODO: signal failure to host over UART and wait for retry
        }
    }
}