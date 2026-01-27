#!/bin/sh
#
# PCF8574 configuration:
# - A0, A1, A2 high
# - jumper between P2 and P3

alias cli="wifi2c-cli rematu.x 2430"

echo "Configuring clock and resetting bus"
cli c10 e

echo "Negative presence detect (reading), expect NAK"
cli s w40 p

echo "Negative presence detect (writing), expect NAK"
cli s w41 p

echo "Positive presence detect (reading), expect ACK"
cli s w4e p

echo "Positive presence detect (writing), expect ACK"
cli s w4f p

echo "Writing in read mode, expect NAK for write"
cli s w4f wff p

echo "Writing 0xfe to port, expect ACKs"
cli s w4e wfe p

echo "Reading port, expect 0xfe"
cli s w4f rn p

echo "Writing 0x11 to port, expect ACKs"
cli s w4e w11 p

echo "Reading port, expect 0x11"
cli s w4f rn p

echo "Writing 0x09, expect ACK"
cli s w4e w09 p

echo "Reading port, expect 0x01 due to jumper"
cli s w4f rn p

echo "Writing 0xff, reading, writing 0x00, reading, expect ACKs and correct data"
cli s w4e wff s w4f rn s w4e w00 s w4f rn p
