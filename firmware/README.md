# BT-Wheel controller firmware

This folder contains the dual-motor controller firmware used by BT-Wheel. The supported PlatformIO environment is `TWO_AXIS_VARIANT`.

Open this folder in VS Code with PlatformIO IDE, or run here:

```powershell
pio run -e TWO_AXIS_VARIANT
```

The output is `.pio/build/TWO_AXIS_VARIANT/firmware.bin`. Upload with ST-Link only after checking the board, MCU support and wiring, with nobody restrained by the belts:

```powershell
pio run -e TWO_AXIS_VARIANT -t upload
```

Both MT6701 encoders must already be configured for PWM. The firmware uses PB6 for the left/Y encoder and PB10 for the right/X encoder, with USART3 remapped to PC10/PC11 for application serial communication.

Follow the [project tutorial](../docs/TUTORIAL.md) for hardware modifications and safety limitations. This firmware is not a certified safety system.

The original fork README is preserved as [UPSTREAM_README.md](UPSTREAM_README.md) for provenance; it is not the current BT-Wheel wiring/setup guide. Preserve the [GPLv3 license](LICENSE), source copyright notices and driver licenses when redistributing this component.
