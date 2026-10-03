# BTOW serial protocol

USART3: 115200 baud, 8 data bits, no parity, 1 stop bit. ASCII commands end with `\n`. Current pinout: TX PC10, RX PC11.

| Command | Effect |
| --- | --- |
| `T<value>` | Command both motors |
| `TY<value>` | Left/Y command |
| `TX<value>` | Right/X command |
| `E`, `EY`, `EX` | Zero and release manual override on selected axes |
| `S` | Text status |
| `H` | Help |
| `?TLM` | One full telemetry response |
| `CFG DUMP` or `?CFG` | Read configuration |
| `CFG <name>=<value>` | Change an allowed parameter |
| `CFG SAVE` | Persist configuration |
| `CFG SAVE_ALIGN` | Persist alignment when available |
| `CFG RESET` | Defaults in RAM; does not automatically save |
| `DSTART` | Clear and start local capture |
| `DSTOP` | Stop capture |
| `DDUMP` | Download capture; rejected while manual override is active |

`T/TY/TX` accept -32767 to 32767. Belt mapping uses unipolar torque from 0 to 32767. Torque commands do not return an OK for each sample. Renew active axes within 150 ms to avoid the current deadman. If one active axis expires, the firmware zeros both.

## Diagnostic capture

```text
DGHZ 200 COUNT <n>
DGHEADER t,Ycmd,Yapp,Yiq,Yrpm,Xcmd,Xapp,Xiq,Xrpm,flags
DG <t>,<Ycmd>,<Yapp>,<Yiq>,<Yrpm>,<Xcmd>,<Xapp>,<Xiq>,<Xrpm>,<flags>
...
DGE
```

| Field | Unit/meaning |
| --- | --- |
| `t` | Milliseconds since capture started |
| `Ycmd`, `Xcmd` | Requested manual command before physical inversion |
| `Yapp`, `Xapp` | Signed command after inversion/limiting |
| `Yiq`, `Xiq` | FOC current Q4; amperes = raw / 800 |
| `Yrpm`, `Xrpm` | MT6701 RPM Q4; RPM = raw / 16 |
| `flags` | Bitmask below |

| Bit | Value | Meaning |
| --- | --- | --- |
| 0 | 1 | Y limiter |
| 1 | 2 | X limiter |
| 2 | 4 | Valid Y RPM |
| 3 | 8 | Valid X RPM |
| 4 | 16 | Y feedback fault |
| 5 | 32 | X feedback fault |
| 6 | 64 | Deadman |

The buffer holds 512 records and stops when full. Nominal sampling is 200 Hz in the main loop; use timestamps to assess the effective rate. Capture does not change gains or guarantee applied command equals requested command.

## Estimated torque

```text
command torque ≈ command / 32767 × Imax × Kt
electromagnetic torque ≈ Iq [A] × Kt [N·m/A]
```

Account for polarity, calibration, limiting and saturation. Neither formula directly measures mechanical belt pull.
