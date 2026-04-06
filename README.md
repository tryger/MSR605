MSR605
======

A C/CPP Library for interacting with the MSR605 (and other MSR206 compatible) Magstripe Reader/Writers.

Added:
* Raw Write support (for the moment only for BPC 8 8 8 mode)
* Erase functionality
* Set High/Low Coercivity
* Reworked CLI with getopt-style flags

You are free to fork this repo, add the functionalities you need, and do a push request.

Usage
=====
```
msr605 -r|-w|-e|-i [--bulk] [--raw|--iso] [--hico|--loco] [--bpc <NNN>] [-1 <hex>] [-2 <hex>] [-3 <hex>] [--tracks <N...>] [-p <dev>]

  -r, --read            read card
  -w, --write           write card (raw)
  -e, --erase           erase card
  -i, --info            retrieve device firmware and model info
  -B, --bulk            write in bulk mode
  -p, --port <dev>      serial device (default: /dev/ttyUSB0)
  --raw / --iso         set data formatting (default: --raw)
  --bpc <NNN>           bits per char for tracks 1/2/3 (default: 888)
  -1, --track1 <hex>    track 1 data (write mode)
  -2, --track2 <hex>    track 2 data (write mode)
  -3, --track3 <hex>    track 3 data (write mode)
  --tracks <N...>       tracks to erase, any combo of 1/2/3 (default: 123)
  --hico / --loco       set coercivity (default: --hico)
  -d, --debug           enable debug output
```

Examples
========
`./msr605 -r` Read raw, default BPC (888), default port

`./msr605 -r --iso` Read ISO, default BPC

`./msr605 -w -1 AABBCC -2 DDEEFF` Write tracks 1 and 2 (raw, HiCo)

`./msr605 -w --loco -1 AABBCC` Write track 1 with low coercivity

`./msr605 -w -B -1 AABBCC` Write track 1 in bulk mode

`./msr605 -e --tracks 12` Erase tracks 1 and 2

`./msr605 -i` Print firmware and model info

TODO
====
Add functionalities remaining, like write in ISO mode, or change the BPI
