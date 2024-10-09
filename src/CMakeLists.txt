add_executable(${NAME}
        LCD_1in28_LVGL_test.c
        LVGL_example.c
        Widgets.c
        #ImageData.c
        main.c
        Eye.c
        mask.c
        )

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(${NAME} 
	pico_stdlib
	Config 
	LCD 
#	Touch 
#	GUI
#	Fonts
	QMI8658 
	lvgl
	hardware_clocks 
	hardware_rtc 
	hardware_adc
	hardware_dma
)

# create map/bin/hex file etc.
pico_add_extra_outputs(${NAME})

# enable usb output, disable uart output
pico_enable_stdio_usb(${NAME} 0)
pico_enable_stdio_uart(${NAME} 1)
target_compile_definitions(${NAME} PRIVATE
    PICO_DEFAULT_UART_RX_PIN=17
    PICO_DEFAULT_UART_TX_PIN=16
#    SUB_EYE=1
)


