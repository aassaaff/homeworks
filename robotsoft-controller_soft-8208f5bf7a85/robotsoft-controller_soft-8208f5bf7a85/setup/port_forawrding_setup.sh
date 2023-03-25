#!/bin/bash

if ! dpkg-query -l socat > /dev/null; then
       sudo apt install socat
fi  

SVC_NAME=plc_fwd
OUTFILE=/etc/systemd/system/${SVC_NAME}@.service
PORT=9000

sudo bash -c "cat >$OUTFILE" <<'EOF'
[Unit]
Description=forward traffic to sick PLC
After=network.target

[Service]
Type=simple
ExecStart=/usr/bin/socat -d -d TCP4-LISTEN:%i,reuseaddr,fork,su=nobody TCP4:11.0.0.30:9000

[Install]
WantedBy=multi-user.target
EOF

sudo systemctl daemon-reload
sudo systemctl start ${SVC_NAME}@${PORT}
sudo systemctl enable ${SVC_NAME}@${PORT}
