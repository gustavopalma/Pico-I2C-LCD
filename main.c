#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"

#define LCD_LIMPA_TELA     0x01
#define LCD_INICIA         0x02
#define LCD_ENTRYMODESET   0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_FUNCTIONSET    0x20

#define LCD_INICIO_ESQUERDA 0x02

#define LCD_LIGA_DISPLAY 0x04

#define LCD_16x2 0x08

#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE_BIT 0x04

// By default these LCD display drivers are on bus address 0x27
#define BUS_ADDR 0x27

// Modes for lcd_envia_byte
#define LCD_CARACTER  1
#define LCD_COMANDO    0

#define MAX_LINES      2
#define MAX_CHARS      16

#define DELAY_US 600

void lcd_envia_comando(uint8_t val) {
    i2c_write_blocking(i2c_default, BUS_ADDR, &val, 1, false);
}

void lcd_pulsa_enable(uint8_t val) {
    sleep_us(DELAY_US);
    lcd_envia_comando(val | LCD_ENABLE_BIT);
    sleep_us(DELAY_US);
    lcd_envia_comando(val & ~LCD_ENABLE_BIT);
    sleep_us(DELAY_US);
}

void lcd_envia_byte(uint8_t caractere, int modo) {
    uint8_t nible_high = modo | (caractere & 0xF0) | LCD_BACKLIGHT;
    uint8_t nible_low = modo | ((caractere << 4) & 0xF0) | LCD_BACKLIGHT;

    lcd_envia_comando(nible_high);
    lcd_pulsa_enable(nible_high);
    lcd_envia_comando(nible_low);
    lcd_pulsa_enable(nible_low);
}

void lcd_limpa_tela(void) {
    lcd_envia_byte(LCD_LIMPA_TELA, LCD_COMANDO);
}

void lcd_posiciona_cursor(int linha, int coluna) {
    int aux = (linha == 0) ? 0x80 + coluna : 0xC0 + coluna;
    lcd_envia_byte(aux, LCD_COMANDO);
}

void lcd_envia_caracter(char caractere) {
    lcd_envia_byte(caractere, LCD_CARACTER);
}

void lcd_envia_string(const char *s) {
    while (*s) {
        lcd_envia_caracter(*s++);
    }
}

void lcd_init() {
    lcd_envia_byte(LCD_INICIA, LCD_COMANDO);
    lcd_envia_byte(LCD_INICIA | LCD_LIMPA_TELA, LCD_COMANDO);
    lcd_envia_byte(LCD_ENTRYMODESET | LCD_INICIO_ESQUERDA, LCD_COMANDO);
    lcd_envia_byte(LCD_FUNCTIONSET | LCD_16x2, LCD_COMANDO);
    lcd_envia_byte(LCD_DISPLAYCONTROL | LCD_LIGA_DISPLAY, LCD_COMANDO);
    lcd_limpa_tela();
}

int main() {

    stdio_init_all();

    sleep_ms(5000);

    // This example will use I2C0 on the default SDA and SCL pins (4, 5 on a Pico)
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    lcd_init();

    lcd_posiciona_cursor(0,0);
    while(true) {
        int opcao_escolhida = 0;
        do {
            fflush(stdin);
            printf("Console do Display:\n");
            printf("1 - posicionar Cursor\n");
            printf("2 - Escrever Mensagem\n");
            printf("3 - Apagar LCD\n");
            printf("Opção Escolhida: \n");
        } while ((scanf("%d",&opcao_escolhida) < 0));
        printf("%d\n", opcao_escolhida);
        int linha = -1;
        int coluna = -1;
        char texto_linha[16];
        memset(texto_linha, '\0', 16);
        switch (opcao_escolhida)
        {
            case 1:
                do {
                    printf("Insira a linha para posicionar o cursor:\n");
                } while (scanf("%d",&linha) < 0 && linha >= 0);
                printf("%d\n", linha);
                do {
                    printf("Insira a coluna para posicionar o cursor:\n");
                } while (scanf("%d",&coluna) < 0 && coluna >= 0);
                printf("%d\n", coluna);
                lcd_posiciona_cursor(linha, coluna);
            break;
        
            case 2:
                do {    
                    printf("Digite os dados para serem escritos no LCD:\n");
                } while (scanf("%s",&texto_linha) < 0);
                printf("%s\n",texto_linha);
                lcd_envia_string(texto_linha);
            break;
            
            case 3:
               lcd_limpa_tela();
            break;

        
        }
    }

    return 0;
}