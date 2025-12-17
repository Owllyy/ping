#!/bin/bash
# Upload ft_ping to VM and open SSH connection

VM_USER="vboxuser"
VM_HOST="10.12.200.21"
VM_PORT="22"

make re

echo "Uploading ft_ping to VM..."
scp -P "$VM_PORT" ft_ping "$VM_USER@$VM_HOST:~/"

echo "Opening SSH connection to VM..."
ssh -p "$VM_PORT" "$VM_USER@$VM_HOST"
