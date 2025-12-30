# Pure-Sinus

The purpose of this project is to provide universal solutions for portable power stations, battery chargers, and inverters. 

## Table of Contents

- [Solr Mobile Power Management Platform](#solr-mobile-power-management-platform)
- [Solr Battery Controller](#solr-battery-controller)

# Solr Mobile Power Management Platform

<img src="solr-mobile-power-manager/assets/solr_board_view.jpg" width="200" alt="solr_board_view">

A compact and powerful battery-powered control electronics system with output power up to 500W. 
It can also be used as a standalone pure sine wave inverter converting 12V DC to 220V AC.
The system includes intelligent monitoring, protection, and management features for safe and reliable portable power delivery.

### Key Features

- **Pure Sine Wave AC Output**: 220V 50Hz AC up to 500W
- **500W Power Rating**: Sufficient for charging multiple devices and powering small appliances
- **DC Output**: 12V DC with up to 180W output for USB charging and DC loads
- **Lithium 3S 12V Battery Input**
- **Intelligent Power Management System**:
  - **Real-time battery voltage monitoring**: Low-battery protection with hysteresis
  - **Overload & Short-Circuit Protection**
  - **Dual Current Sensing**: Independent monitoring of DC output and AC inverter currents
  - **Dual Temperature Monitoring**: With automatic fan control and overheat protection
  - **Bidirectional DC**: DC output can also be used to charge the battery
  - **Low power consumption in standby mode**
- **Portable & Modular**: Designed as a platform for expanding functionality

### Use Cases

✓ **Device Charging**: Portable charging station for phones, tablets, and laptops  
✓ **Power Tools**: Supply stable AC power to cordless tools and equipment  
✓ **Emergency Power**: Backup power for critical devices during outages  
✓ **Outdoor Adventures**: Portable power for camping, RVs, and off-grid scenarios  
✓ **IoT & Smart Devices**: Reliable power for home automation and smart systems  

### Project Structure

```
pure-sinus/
└── solr-mobile-power-manager/         # Main folder
    ├── assets/                        # Images and visual assets
    ├── documents/                     # Datasheets and technical documentation
    ├── altium/                      # Altium PCB designs and circuit schematics
    │   ├── inverter_12V_220V_500W/    # Main inverter/power station board
    │   │   ├── Fabrication Files/     # Gerber, BOM, and manufacturing files
    │   │   └── libs/                  # Component libraries for Altium Designer
    │   └── tl494_pwm_controller/      # Dedicated PWM controller module
    │       ├── Fabrication Files/     # Gerber, BOM, and manufacturing files
    │       └── libs/                  # Component libraries for Altium Designer
    └── software/                      # PlatformIO firmware and control logic
        └── inverter-controller/       # Arduino firmware (PlatformIO-based)
            ├── src/                   # Source code files
            ├── include/               # Header files and configuration
            ├── lib/                   # Custom and external libraries
            ├── test/                  # Unit tests
            └── platformio.ini         # PlatformIO project configuration
```

### Hardware Specifications

### Main Components

- **Microcontroller**: Arduino Nano (ATmega328P)
- **PWM Controller**: TL494 PWM IC (with custom dedicated module)
- **Pure Sinus Driver**: EGS002
- **Current Sensors**: 
  - ACS712 30A Hall-effect current sensor DC
  - ACS758 100A Hall-effect current sensor AC
- **Temperature Sensors**: LM35
- **Output Transformer**: EC42-20 High-frequency SMPS transformer (12V→380V conversion)

### Hardware Assembly

### 1. TL494 PWM Module Assembly And Setup

- `pwm_controller_board.rar` contains all required files for PCB manufacturing the TL494 PWM module and located in `/altium/tl494_pwm_controller/Fabrication Files`
- <img src="solr-mobile-power-manager/assets/tl494_pcb_data.jpg" width="400" alt="pcb_request_data">
- Follow the provided BOM and Gerber files to fabricate and assemble the PWM module
- <img src="solr-mobile-power-manager/assets/tl494_pcb.jpg" width="200" alt="tl494_pcb">
- PWM module setup: 
    - Connect 12V power supply to `+12V` and `GND`
    - Set the frequency to `32kHz` for outputs `OutA` and `OutB` using potentiometer `Freq`
    - Adjust potentiometer `Duty` for `42%` duty cycle at no load
    - No need to configure current limiter, so `I-Sense` pin can be connected directly to ground
- <img src="solr-mobile-power-manager/assets/tl494_config.jpg" width="300" alt="tl494_config">

### 2. EGS002 Driver Module Setup

The EGS002 module handles pure sinus wave generation
- <img src="solr-mobile-power-manager/assets/egs002.jpg" width="200" alt="egs_pcb">
- Solder `JP8` jumper to `JP4`, to set dead time for `1.0us`
- <img src="solr-mobile-power-manager/assets/egs002_config.jpg" width="200" alt="egs_pcb_config">
- <img src="solr-mobile-power-manager/assets/egs002_config_doc.jpg" width="300" alt="egs_pcb_config_doc">
- No additional configuration is required

### 3. Inverter Board Assembly And Setup

- `inverter_board.rar` contains all required files for PCB manufacturing the board and located in `/altium/inverter_12V_220V_500W/Fabrication Files/`
- <img src="solr-mobile-power-manager/assets/inverter_pcb_data.jpg" width="300" alt="inverter_pcb_request_data">
- Follow the provided BOM and Gerber files to fabricate and assemble the PCB
- <img src="solr-mobile-power-manager/assets/inverter_board.jpg" width="300" alt="inverter_board">

### Assembly Steps

1. First, solder all SMD components on top and bottom layers
2. Solder all components, except `C6, C10, C11, T1, C21, M1 and M2`
3. Place heat sink `HS1` and `HS2`, then solder MOSFETs and LM78xx, secure with screws and isolating pads
4. Solder all other components except driver modules and Arduino
5. ***Note:*** It is recommended to use sockets for the microcontroller and driver modules for easy replacement and troubleshooting.
6. Then, connect/solder the `Arduino` and `TL494 PWM module`
7. Program the Arduino with the provided firmware
8. Connect laboratory power supply to `12V` input, set the current limit to `4-5A`, set `J8` switch and power on the board
9. Measure HV output voltage, it should be `400V DC` with no load and no `EGS002` module connected
- <img src="solr-mobile-power-manager/assets/hv_config.jpg" width="300" alt="hv_config">
10. Turn off power supply and wait(`R2` also used as bleed resistor) or discharge HV capacitors `C10` and `C11` before connecting the `EGS002` module
11. Connect the `EGS002`, run the next time. `EGS002` led should be on continuously indicating normal operation
12. Measure the output `AC` voltage, adjust the `VR1` potentiometer to set output voltage to `230V AC`
13. Check output `AC` waveform with oscilloscope, it should be a clean sine wave, also check with a load connected
- <img src="solr-mobile-power-manager/assets/ac_config.jpg" width="300" alt="ac_config">
14. Check output `DC` voltage, it should be `12V DC`
15. Connect cooling fan: it should run for a short time on power on, during system self-check
16. Then, set debug switch `P1` to on. Check ACS current sensors outputs, adjust if needed
17. Finally, glue thermal sensors to heat sinks using thermal adhesive glue (`heatsinkplaster`)
- <img src="solr-mobile-power-manager/assets/lm35_set.jpg" width="200" alt="lm35_set">

### PCB Components
- Heat Sinks **HS1 and HS2** `150x19.7x15.6mm`: [link Aliexpress](https://www.aliexpress.com/item/1005004124759503.html?spm=a2g0o.productlist.main.1.42223a20SAT5k2&algo_pvid=6427d303-d8d6-43af-a767-dd0f720f51ae&algo_exp_id=6427d303-d8d6-43af-a767-dd0f720f51ae-0&pdp_ext_f=%7B%22order%22%3A%22378%22%2C%22spu_best_type%22%3A%22order%22%2C%22eval%22%3A%221%22%2C%22fromPage%22%3A%22search%22%7D&pdp_npi=6%40dis%21EUR%212.55%212.55%21%21%2120.51%2120.51%21%40211b628117670993457081121e6103%2112000030402237637%21sea%21LV%21143434442%21X%211%210%21n_tag%3A-29919%3Bd%3A4c3c036e%3Bm03_new_user%3A-29895&curPageLogUid=1Bff7gj6ESD6&utparam-url=scene%3Asearch%7Cquery_from%3A%7Cx_object_id%3A1005004124759503%7C_p_origin_prod%3A)
- Transformer **T1** `12V To 0-220V-380V 18V 500W EC42`: [link Aliexpress](https://www.aliexpress.com/item/1005003166892846.html?spm=a2g0o.order_list.order_list_main.5.2afd1802XkXKQq)
- Inductor **L2** `1.5 V. 2mH`: [link Aliexpress](https://www.aliexpress.com/item/1005006733553679.html?spm=a2g0o.order_list.order_list_main.382.6bd81802CETRoA)
- Choke filter **L1** `5A`: [link Aliexpress](https://www.aliexpress.com/item/33005799242.html?spm=a2g0o.order_list.order_list_main.65.6bd81802CETRoA)
- Capacitors **C10 and C11** `100uF 450V`: [link Aliexpress](https://www.aliexpress.com/item/1005006810057095.html?spm=a2g0o.order_list.order_list_main.362.6bd81802CETRoA)
- Shunt Resistor **SH1 and SH2** `20mR 1x10`: [link Aliexpress](https://www.aliexpress.com/item/1005005512229757.html?spm=a2g0o.order_list.order_list_main.372.6bd81802CETRoA)
- Screw terminal connectors **J3, J4, J5, J6**: [link Aliexpress](https://www.aliexpress.com/item/1005007060857910.html?spm=a2g0o.order_list.order_list_main.377.6bd81802CETRoA)
- Fuses **F1, F2** `25A`: [link Aliexpress](https://www.aliexpress.com/item/1005003956737422.html?spm=a2g0o.order_list.order_list_main.347.6bd81802CETRoA)
- The complete Bill of Materials (BOM) can be found in the `/altium/inverter_12V_220V_500W/Fabrication Files/` directory as `BOM_Inverter_12V_220V_500W.xlsx`.

### Firmware Architecture

Built with **PlatformIO** and **Arduino Framework** targeting Arduino Nano `ATmega328P`.

**Debug**:
- **When debug switch set**: 9600 baud UART can be used for monitoring and troubleshooting

## Safety Considerations And Warnings

### ⚠️ High Voltage Warning

This project operates with dangerous voltages

## Next Version Improvements

- [ ] **OLED display and controls**: Add an OLED display and control buttons for monitoring and outputs on/off control
- [ ] **Multi-Output USB**: Integrate USB-C and USB-A charging ports with PD protocol support, with up to 100W
- [ ] **Battery Chemistry Flexibility**: Support for LiFePO4, Lead-acid, and other battery types
- [ ] **Solar Input**: Add MPPT/PWM solar charge controller for renewable energy integration

---

# Solr Battery Controller

A universal `DC 12V 30A` lithium battery controller.
The compact PCB design provides robust protection against battery over-discharge and over-current conditions,
ensuring maximum battery longevity and optimal performance with integrated power on delay feature.

<img src="solr-battery-controller/assets/contoller_pcb.jpg" width="300" alt="solr_controller_pcb">

### Key Features

- **Universal 12V Battery Compatibility**: Easy to set for various 12V battery types
- **Low Voltage Protection**: Detects and prevents dangerous voltage drops
- **Overcurrent Protection**: Output load disconnection on excessive current draw
- **Power On Delay**: Auto recovery after power interruptions with hysteresis, battery should be charged at least 10% for power on 
- **Cooling Fan Output**: Automatic fan control based on current consumption. Can be reprogrammed as battery charger
- **Reverse Polarity Protection**
- **Compact Design**: 94mm x 70mm

### Project Structure

```
solr-battery-controller/               # Battery controller folder
├── assets/                            # Images and visual assets
├── documents/                         # Datasheets and technical documentation
├── altium/                            # Altium PCB designs and circuit schematics
│   └── solr-controller-base/          # Main controller board
│       ├── Fabrication Files/         # Gerber, BOM, and manufacturing files
│       └── *.IntLib                   # Component libraries for Altium Designer
└── software/                          # Firmware and control logic
    └── solr-controller-base/          # Arduino firmware (PlatformIO-based)
        ├── src/                       # Source code files
        ├── include/                   # Header files and configuration
        ├── lib/                       # Custom and external libraries
        ├── test/                      # Unit tests
        └── platformio.ini             # PlatformIO project configuration
```

### PCB Assembly

- <img src="solr-battery-controller/assets/controller_board.jpg" width="300" alt="solr_controller_pcb">
- `solr-controller-board.rar` contains all required files for PCB manufacturing the board and located in `/altium/solr-controller-base/Fabrication Files/`
- <img src="solr-battery-controller/assets/pcb_request_data.jpg" width="400" alt="solr_controller_pcb">

### PCB Components

- Relay **K1** `12V 30A`: [link Aliexpress](https://www.aliexpress.com/item/1005004371446685.html?spm=a2g0o.productlist.main.4.1e95351ck3FxvC&aem_p4p_detail=202512301211284046174373068260000285333&algo_pvid=0a38c24e-9a0b-48a9-8ece-bbadc1ed5a35&algo_exp_id=0a38c24e-9a0b-48a9-8ece-bbadc1ed5a35-3&pdp_ext_f=%7B%22order%22%3A%2258%22%2C%22spu_best_type%22%3A%22order%22%2C%22eval%22%3A%221%22%2C%22fromPage%22%3A%22search%22%7D&pdp_npi=6%40dis%21EUR%212.30%212.29%21%21%212.64%212.64%21%40211b628117671254885737151e60ff%2112000028942938667%21sea%21LV%21143434442%21X%211%210%21n_tag%3A-29919%3Bd%3A4c3c036e%3Bm03_new_user%3A-29895&curPageLogUid=ecZqGVMWNfA1&utparam-url=scene%3Asearch%7Cquery_from%3A%7Cx_object_id%3A1005004371446685%7C_p_origin_prod%3A&search_p4p_id=202512301211284046174373068260000285333_1)
- Connectors **J3 J4** `HB9500 2pin` : [link Aliexpress](https://www.aliexpress.com/item/1005008336461654.html?spm=a2g0o.detail.pcDetailTopMoreOtherSeller.5.5da4zYsrzYsrUS&gps-id=pcDetailTopMoreOtherSeller&scm=1007.40050.354490.0&scm_id=1007.40050.354490.0&scm-url=1007.40050.354490.0&pvid=bdbd0872-ae3c-45be-8bcf-08dd0156c3fc&_t=gps-id:pcDetailTopMoreOtherSeller,scm-url:1007.40050.354490.0,pvid:bdbd0872-ae3c-45be-8bcf-08dd0156c3fc,tpp_buckets:668%232846%238111%231996&pdp_ext_f=%7B%22order%22%3A%22-1%22%2C%22eval%22%3A%221%22%2C%22sceneId%22%3A%2230050%22%2C%22fromPage%22%3A%22recommend%22%7D&pdp_npi=6%40dis%21EUR%210.50%210.50%21%21%210.58%210.58%21%40211b613917671256503675517e5ba9%2112000044644274242%21rec%21LV%21143434442%21XZ%211%210%21n_tag%3A-29919%3Bd%3A4c3c036e%3Bm03_new_user%3A-29895&utparam-url=scene%3ApcDetailTopMoreOtherSeller%7Cquery_from%3A%7Cx_object_id%3A1005008336461654%7C_p_origin_prod%3A)
- Current sensor **U3** `ACS712 30A`: [link Aliexpress](https://www.aliexpress.com/item/1005008877256845.html?spm=a2g0o.productlist.main.8.1c516013m5kkaQ&aem_p4p_detail=2025123012292518547528048932480000304795&algo_pvid=71db4e8c-a038-4a7a-af75-4887cdcbba6d&algo_exp_id=71db4e8c-a038-4a7a-af75-4887cdcbba6d-7&pdp_ext_f=%7B%22order%22%3A%2215%22%2C%22eval%22%3A%221%22%2C%22fromPage%22%3A%22search%22%7D&pdp_npi=6%40dis%21EUR%214.43%213.99%21%21%215.09%214.58%21%40211b876e17671265658067230e5f86%2112000047054668964%21sea%21LV%21143434442%21X%211%210%21n_tag%3A-29919%3Bd%3A4c3c036e%3Bm03_new_user%3A-29895&curPageLogUid=34I8RfFB5gaF&utparam-url=scene%3Asearch%7Cquery_from%3A%7Cx_object_id%3A1005008877256845%7C_p_origin_prod%3A&search_p4p_id=2025123012292518547528048932480000304795_2)
- MCU **U2**: Arduino Nano (ATmega328P)
- The complete Bill of Materials (BOM) can be found in the `/altium/solr-controller-base/Fabrication Files/` directory as `BOM_Solr_Battery_Controller.xlsx`.

### Firmware Architecture

Built with **PlatformIO** and **Arduino Framework** targeting Arduino Nano `ATmega328P`.

**Debug**:
- **When debug switch set** `P1`: 9600 baud UART can be used for monitoring and troubleshooting

## Next Version Improvements

- [ ] **Battery Chemistry Flexibility**: Support for LiFePO4, Lead-acid, and other battery types

---